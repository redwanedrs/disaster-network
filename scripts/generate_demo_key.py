#!/usr/bin/env python3
"""Print a cryptographically random 128-bit AES key in C++ initializer form."""
from secrets import token_bytes

key = token_bytes(16)
print("static const uint8_t DISASTER_AES_KEY[16] = {")
print("  " + ", ".join(f"0x{b:02X}" for b in key))
print("};")
