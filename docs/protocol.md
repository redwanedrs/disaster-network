# Disaster Network Packet Protocol v1

## Design goals

- Compact enough for SX1278 LoRa frames.
- Explicit node addressing at the application layer.
- AES-128-GCM confidentiality + authentication.
- Random 96-bit nonce per frame.
- Encrypted CRC32 for the requested encrypted checksum field.
- Versioning for future protocol changes.

## Wire layout

All integer fields are little-endian.

```text
Offset  Size  Field
------  ----  --------------------------------
0       1     Magic = 0xD7
1       1     Version = 1
2       1     Source node ID
3       1     Destination node ID (0 = broadcast)
4       1     Message type
5       4     Sequence number
9       2     Plain payload length
11      12    AES-GCM nonce / IV
23      N+4   Ciphertext(payload || CRC32)
23+N+4  16    AES-GCM authentication tag
```

Maximum application payload is 120 bytes in the current firmware.

## Message types

| Value | Type | Meaning |
|---:|---|---|
| 1 | STATUS | Node state/heartbeat |
| 2 | SENSOR | Sensor reading/report |
| 3 | SOS | Emergency panic alert |
| 4 | TEXT | Human-readable short message |
| 5 | ACK | Acknowledgement placeholder |
| 6 | PING | Link test placeholder |
| 7 | ERROR | Error report |

## Cryptography

AES-128-GCM authenticates the header as AAD and encrypts the payload plus CRC32. The GCM authentication tag protects the frame against tampering. The CRC32 is not a security primitive; it is included because the project requested an encrypted checksum field and provides an additional corruption check.

The current demo implementation uses one shared 128-bit network key. This is acceptable for a prototype but is not a strong operational key-management design. Future work should add secure provisioning, per-node keys or a robust group-key scheme, replay protection, and key rotation.

## Replay protection

Sequence numbers currently identify frames but are not persisted or checked against a receive window. This is deliberately a TODO. Before deployment in an adversarial environment, nodes must reject stale/replayed frames.
