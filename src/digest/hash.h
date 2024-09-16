#pragma once

#include <stdint.h>

struct hash128 {
	uint8_t bytes[16];
};

struct hash256 {
	uint8_t bytes[32];
};

void write_hash128(int fd, struct hash128 hash128);
void write_hash256(int fd, struct hash256 hash256);
