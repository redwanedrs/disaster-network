#include "disaster_protocol.h"
#include <mbedtls/gcm.h>
#include <esp_system.h>
#include <string.h>

namespace Disaster {

static uint32_t crc32(const uint8_t* data, size_t len) {
  uint32_t crc = 0xFFFFFFFFu;
  for (size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; ++bit) {
      crc = (crc >> 1) ^ (0xEDB88320u & (-(int32_t)(crc & 1u)));
    }
  }
  return ~crc;
}

static void put16(uint8_t* p, uint16_t v) {
  p[0] = (uint8_t)(v & 0xFF);
  p[1] = (uint8_t)((v >> 8) & 0xFF);
}

static uint16_t get16(const uint8_t* p) {
  return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static void put32(uint8_t* p, uint32_t v) {
  p[0] = (uint8_t)(v & 0xFF);
  p[1] = (uint8_t)((v >> 8) & 0xFF);
  p[2] = (uint8_t)((v >> 16) & 0xFF);
  p[3] = (uint8_t)((v >> 24) & 0xFF);
}

static uint32_t get32(const uint8_t* p) {
  return (uint32_t)p[0] |
         ((uint32_t)p[1] << 8) |
         ((uint32_t)p[2] << 16) |
         ((uint32_t)p[3] << 24);
}

size_t encodePacket(const Packet& packet, uint8_t* out, size_t outCapacity,
                    const uint8_t key[16]) {
  if (!out || !key || packet.payloadLen > MAX_PAYLOAD_LEN) return 0;

  const size_t encryptedLen = packet.payloadLen + CRC_LEN;
  const size_t totalLen = HEADER_LEN + NONCE_LEN + encryptedLen + TAG_LEN;
  if (totalLen > outCapacity || totalLen > MAX_PACKET_LEN) return 0;

  uint8_t* header = out;
  header[0] = PROTOCOL_MAGIC;
  header[1] = PROTOCOL_VERSION;
  header[2] = packet.source;
  header[3] = packet.destination;
  header[4] = static_cast<uint8_t>(packet.type);
  put32(&header[5], packet.sequence);
  put16(&header[9], packet.payloadLen);

  uint8_t* nonce = &out[HEADER_LEN];
  uint32_t r[3];
  r[0] = esp_random(); r[1] = esp_random(); r[2] = esp_random();
  memcpy(nonce, r, NONCE_LEN);

  uint8_t plain[MAX_PAYLOAD_LEN + CRC_LEN];
  memcpy(plain, packet.payload, packet.payloadLen);
  uint32_t crc = crc32(packet.payload, packet.payloadLen);
  put32(&plain[packet.payloadLen], crc);

  uint8_t* cipher = &out[HEADER_LEN + NONCE_LEN];
  uint8_t* tag = &cipher[encryptedLen];

  mbedtls_gcm_context gcm;
  mbedtls_gcm_init(&gcm);
  int rc = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key, 128);
  if (rc == 0) {
    rc = mbedtls_gcm_crypt_and_tag(&gcm, MBEDTLS_GCM_ENCRYPT, encryptedLen,
                                   nonce, NONCE_LEN, header, HEADER_LEN,
                                   plain, cipher, TAG_LEN, tag);
  }
  mbedtls_gcm_free(&gcm);
  return rc == 0 ? totalLen : 0;
}

bool decodePacket(const uint8_t* data, size_t dataLen, Packet& packet,
                  const uint8_t key[16]) {
  if (!data || !key || dataLen < HEADER_LEN + NONCE_LEN + CRC_LEN + TAG_LEN) return false;
  if (data[0] != PROTOCOL_MAGIC || data[1] != PROTOCOL_VERSION) return false;

  const uint16_t payloadLen = get16(&data[9]);
  if (payloadLen > MAX_PAYLOAD_LEN) return false;
  const size_t encryptedLen = payloadLen + CRC_LEN;
  const size_t expected = HEADER_LEN + NONCE_LEN + encryptedLen + TAG_LEN;
  if (dataLen != expected) return false;

  packet.source = data[2];
  packet.destination = data[3];
  packet.type = static_cast<MessageType>(data[4]);
  packet.sequence = get32(&data[5]);

  const uint8_t* nonce = &data[HEADER_LEN];
  const uint8_t* cipher = &data[HEADER_LEN + NONCE_LEN];
  const uint8_t* tag = &cipher[encryptedLen];
  uint8_t plain[MAX_PAYLOAD_LEN + CRC_LEN];

  mbedtls_gcm_context gcm;
  mbedtls_gcm_init(&gcm);
  int rc = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key, 128);
  if (rc == 0) {
    rc = mbedtls_gcm_auth_decrypt(&gcm, encryptedLen, nonce, NONCE_LEN,
                                  data, HEADER_LEN, tag, TAG_LEN,
                                  cipher, plain);
  }
  mbedtls_gcm_free(&gcm);
  if (rc != 0) return false;

  const uint32_t suppliedCrc = get32(&plain[payloadLen]);
  const uint32_t calculatedCrc = crc32(plain, payloadLen);
  if (suppliedCrc != calculatedCrc) return false;

  packet.payloadLen = static_cast<uint8_t>(payloadLen);
  memcpy(packet.payload, plain, payloadLen);
  return true;
}

const char* messageTypeName(MessageType type) {
  switch (type) {
    case MessageType::STATUS: return "STATUS";
    case MessageType::SENSOR: return "SENSOR";
    case MessageType::SOS: return "SOS";
    case MessageType::TEXT: return "TEXT";
    case MessageType::ACK: return "ACK";
    case MessageType::PING: return "PING";
    case MessageType::ERROR: return "ERROR";
    default: return "UNKNOWN";
  }
}

} // namespace Disaster
