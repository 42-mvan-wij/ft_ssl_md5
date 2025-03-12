#pragma once

#include <stdint.h>

struct hash128 {
	uint8_t hash[16]; // stored in big-endian
};

struct hash128_hex {
	char hex[33];
};

struct hash256 {
	uint8_t hash[32]; // stored in big-endian
};

struct hash256_hex {
	char hex[65];
};

struct hash512 {
	uint8_t hash[64]; // stored in big-endian
};

struct hash512_hex {
	char hex[129];
};

struct hash128_hex hash128_hex(struct hash128 const *hash);
struct hash256_hex hash256_hex(struct hash256 const *hash);
struct hash512_hex hash512_hex(struct hash512 const *hash);
