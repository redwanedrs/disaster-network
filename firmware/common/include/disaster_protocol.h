#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

namespace Disaster {

constexpr uint8_t PROTOCOL_MAGIC = 0xD7;
constexpr uint8_t PROTOCOL_VERSION = 1;
constexpr size_t NONCE_LEN = 12;
constexpr size_t TAG_LEN = 16;
constexpr size_t CRC_LEN = 4;
constexpr size_t HEADER_LEN = 11; // magic, version, src, dst, type, seq(4), payload_len(2)
constexpr size_t MAX_PACKET_LEN = 255;

enum class MessageType : uint8_t {
  STATUS = 1,
  SENSOR = 2,
  SOS = 3,
  TEXT = 4,
  ACK = 5,
  PING = 6,
  ERROR = 7,
};

struct Packet {
  uint8_t source = 0;
  uint8_t destination = 0;
  MessageType type = MessageType::TEXT;
  uint32_t sequence = 0;
  uint8_t payload[MAX_PAYLOAD_LEN] = {};
  uint8_t payloadLen = 0;
};

// Wire format (all integer fields are little-endian):
// [magic][version][src][dst][type][seq:4][payload_len]
// [nonce:12][AES-GCM ciphertext(payload || CRC32)][tag:16]
//
// The CRC32 is encrypted, while the GCM tag provides cryptographic integrity.
// The header is authenticated as AES-GCM AAD but not encrypted.
size_t encodePacket(const Packet& packet, uint8_t* out, size_t outCapacity,
                    const uint8_t key[16]);

bool decodePacket(const uint8_t* data, size_t dataLen, Packet& packet,
                  const uint8_t key[16]);

const char* messageTypeName(MessageType type);

} // namespace Disaster
