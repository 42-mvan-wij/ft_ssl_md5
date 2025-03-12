#pragma once

#include <stdint.h>

#include "hash.h"

struct matrix {
	uint8_t data[64];
};

struct whirlpool_state {
	struct matrix matrix;
	uint64_t msg_len[4];
};

struct whirlpool_state whirlpool_state(void);

/// `m` should have a consistent order of bytes (endianness) on different hosts
/// `m` should be a block of 64 bytes (512 bits)
struct whirlpool_state whirlpool_round(struct whirlpool_state state, uint8_t const m[64]);

/// `m` should have a consistent order of bytes (endianness) on different hosts
/// `m` should be a block of at most 512 bits (64 bytes)
struct hash512 whirlpool_final_round(struct whirlpool_state state, uint8_t const m[64], uint16_t bits);
