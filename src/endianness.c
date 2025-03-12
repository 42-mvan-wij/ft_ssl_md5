#include "endianness.h"

uint16_t byte_swap16(uint16_t n) {
	return
		((n >> 8) & 0x00FFull) |
		((n << 8) & 0xFF00ull);
}

uint32_t byte_swap32(uint32_t n) {
	return
		((n >> 24) & 0x000000FFull) |
		((n >>  8) & 0x0000FF00ull) |
		((n <<  8) & 0x00FF0000ull) |
		((n << 24) & 0xFF000000ull);
}

uint64_t byte_swap64(uint64_t n) {
	return
		((n >> 56) & 0x00000000000000FFull) |
		((n >> 40) & 0x000000000000FF00ull) |
		((n >> 24) & 0x0000000000FF0000ull) |
		((n >>  8) & 0x00000000FF000000ull) |
		((n <<  8) & 0x000000FF00000000ull) |
		((n << 24) & 0x0000FF0000000000ull) |
		((n << 40) & 0x00FF000000000000ull) |
		((n << 56) & 0xFF00000000000000ull);
}

#include <stdio.h>
uint16_t host_to_little16(uint16_t n) {
	printf("%x => %x & %x\n", BYTE_ORDER, LITTLE_ENDIAN, BIG_ENDIAN);
#if BYTE_ORDER == BIG_ENDIAN
	return byte_swap16(n);
#elif BYTE_ORDER == LITTLE_ENDIAN
	return n;
#else
#error Unsupported byte order
#endif
}

uint16_t host_to_big16(uint16_t n) {
#if BYTE_ORDER == BIG_ENDIAN
	return n;
#elif BYTE_ORDER == LITTLE_ENDIAN
	return byte_swap16(n);
#else
#error Unsupported byte order
#endif
}

uint32_t host_to_little32(uint32_t n) {
#if BYTE_ORDER == BIG_ENDIAN
	return byte_swap32(n);
#elif BYTE_ORDER == LITTLE_ENDIAN
	return n;
#else
#error Unsupported byte order
#endif
}

uint32_t host_to_big32(uint32_t n) {
#if BYTE_ORDER == BIG_ENDIAN
	return n;
#elif BYTE_ORDER == LITTLE_ENDIAN
	return byte_swap32(n);
#else
#error Unsupported byte order
#endif
}

uint64_t host_to_little64(uint64_t n) {
#if BYTE_ORDER == BIG_ENDIAN
	return byte_swap64(n);
#elif BYTE_ORDER == LITTLE_ENDIAN
	return n;
#else
#error Unsupported byte order
#endif
}

uint64_t host_to_big64(uint64_t n) {
#if BYTE_ORDER == BIG_ENDIAN
	return n;
#elif BYTE_ORDER == LITTLE_ENDIAN
	return byte_swap64(n);
#else
#error Unsupported byte order
#endif
}

uint16_t little_to_host16(uint16_t n) {
#if BYTE_ORDER == BIG_ENDIAN
	return byte_swap16(n);
#elif BYTE_ORDER == LITTLE_ENDIAN
	return n;
#else
#error Unsupported byte order
#endif
}

uint16_t big_to_host16(uint16_t n) {
#if BYTE_ORDER == BIG_ENDIAN
	return n;
#elif BYTE_ORDER == LITTLE_ENDIAN
	return byte_swap16(n);
#else
#error Unsupported byte order
#endif
}

uint32_t little_to_host32(uint32_t n) {
#if BYTE_ORDER == BIG_ENDIAN
	return byte_swap32(n);
#elif BYTE_ORDER == LITTLE_ENDIAN
	return n;
#else
#error Unsupported byte order
#endif
}

uint32_t big_to_host32(uint32_t n) {
#if BYTE_ORDER == BIG_ENDIAN
	return n;
#elif BYTE_ORDER == LITTLE_ENDIAN
	return byte_swap32(n);
#else
#error Unsupported byte order
#endif
}

uint64_t little_to_host64(uint64_t n) {
#if BYTE_ORDER == BIG_ENDIAN
	return byte_swap64(n);
#elif BYTE_ORDER == LITTLE_ENDIAN
	return n;
#else
#error Unsupported byte order
#endif
}

uint64_t big_to_host64(uint64_t n) {
#if BYTE_ORDER == BIG_ENDIAN
	return n;
#elif BYTE_ORDER == LITTLE_ENDIAN
	return byte_swap64(n);
#else
#error Unsupported byte order
#endif
}
