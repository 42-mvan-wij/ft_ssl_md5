#include "hash.h"

struct hash128_hex hash128_hex(struct hash128 const *hash) {
	char const hex_digits[] = "0123456789abcdef";
	struct hash128_hex hex;
	for (uint8_t i = 0; i < sizeof(hash->hash); i++) {
		hex.hex[i * 2] = hex_digits[hash->hash[i] / 16];
		hex.hex[i * 2 + 1] = hex_digits[hash->hash[i] % 16];
	}
	hex.hex[sizeof(hex.hex) - 1] = '\0';
	return hex;
}

struct hash256_hex hash256_hex(struct hash256 const *hash) {
	char const hex_digits[] = "0123456789abcdef";
	struct hash256_hex hex;
	for (uint8_t i = 0; i < sizeof(hash->hash); i++) {
		hex.hex[i * 2] = hex_digits[hash->hash[i] / 16];
		hex.hex[i * 2 + 1] = hex_digits[hash->hash[i] % 16];
	}
	hex.hex[sizeof(hex.hex) - 1] = '\0';
	return hex;
}

struct hash512_hex hash512_hex(struct hash512 const *hash) {
	char const hex_digits[] = "0123456789abcdef";
	struct hash512_hex hex;
	for (uint8_t i = 0; i < sizeof(hash->hash); i++) {
		hex.hex[i * 2] = hex_digits[hash->hash[i] / 16];
		hex.hex[i * 2 + 1] = hex_digits[hash->hash[i] % 16];
	}
	hex.hex[sizeof(hex.hex) - 1] = '\0';
	return hex;
}
