#include <assert.h>
#include <unistd.h>

#include "hash.h"

static char as_hexdigit(uint8_t n) {
	static char const hexdigits[] = "0123456789abcdef";

	assert(n < sizeof(hexdigits));
	return hexdigits[n];
}

void write_hash128(int fd, struct hash128 hash128) {
	char hash_text[sizeof(hash128.bytes) * 2];

	for (size_t i = 0; i < sizeof(hash128.bytes); i++) {
		hash_text[i * 2] = as_hexdigit(hash128.bytes[i] / 16); 
		hash_text[i * 2 + 1] = as_hexdigit(hash128.bytes[i] % 16); 
	}
	write(fd, hash_text, sizeof(hash_text));
}

void write_hash256(int fd, struct hash256 hash256) {
	char hash_text[sizeof(hash256.bytes) * 2];

	for (size_t i = 0; i < sizeof(hash256.bytes); i++) {
		hash_text[i * 2] = as_hexdigit(hash256.bytes[i] / 16); 
		hash_text[i * 2 + 1] = as_hexdigit(hash256.bytes[i] % 16); 
	}
	write(fd, hash_text, sizeof(hash_text));
}


