#pragma once

#include <stdint.h>

#include "hash.h"

struct md5_state {
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;

	uint64_t msg_len;
};

struct md5_state md5_state(void);

/// `m` should have a consistent order of bytes (endianness) on different hosts
/// `m` should be a block of 64 bytes (512 bits)
struct md5_state md5_round(struct md5_state state, uint8_t const m[64]);

/// `m` should have a consistent order of bytes (endianness) on different hosts
/// `m` should be a block of 64 bytes (512 bits)
struct hash128 md5_final_round(struct md5_state state, uint8_t const m[64], uint16_t bits);
