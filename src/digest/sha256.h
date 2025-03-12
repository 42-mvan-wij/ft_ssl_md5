#pragma once

#include <stdint.h>

#include "hash.h"

struct sha256_state {
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;
	uint32_t e;
	uint32_t f;
	uint32_t g;
	uint32_t h;

	uint64_t msg_len;
};

struct sha256_state sha256_state(void);

/// `m` should have a consistent order of bytes (endianness) on different hosts
/// `m` should be a block of 64 bytes (512 bits)
struct sha256_state sha256_round(struct sha256_state state, uint8_t const m[64]);

/// `m` should have a consistent order of bytes (endianness) on different hosts
/// `m` should be a block of at most 512 bits (64 bytes)
struct hash256 sha256_final_round(struct sha256_state state, uint8_t const m[64], uint16_t bits);
