#include <assert.h>
#include <stdalign.h> // TODO: remove?

#include "endianness.h"
#include "md5.h"
#include "utils.h"

struct md5_state md5_state(void) {
	struct md5_state state = {
		.a = 0x67452301,
		.b = 0xefcdab89,
		.c = 0x98badcfe,
		.d = 0x10325476,

		.msg_len = 0,
	};

	return state;
}

/// Each block in `m` is host-endian, the blocks are in big-endian
static struct md5_state process_chunk(struct md5_state state, uint32_t const m[16]) {
	static uint8_t const s[64] = {
		7, 12, 17, 22,
		7, 12, 17, 22,
		7, 12, 17, 22,
		7, 12, 17, 22,

		5,  9, 14, 20,
		5,  9, 14, 20,
		5,  9, 14, 20,
		5,  9, 14, 20,

		4, 11, 16, 23,
		4, 11, 16, 23,
		4, 11, 16, 23,
		4, 11, 16, 23,

		6, 10, 15, 21,
		6, 10, 15, 21,
		6, 10, 15, 21,
		6, 10, 15, 21,
	};

	static uint32_t const k[64] = {
		0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
		0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
		0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
		0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
		0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
		0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
		0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
		0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
		0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
		0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
		0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
		0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
		0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
		0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
		0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
	};

	uint32_t a = state.a;
	uint32_t b = state.b;
	uint32_t c = state.c;
	uint32_t d = state.d;

	for (uint8_t i = 0; i < 64; i++) {
		uint32_t f;
		uint16_t g;

		if (i < 16) {
			f = (b & c) | ((~b) & d);
			g = i;
		}
		else if (i < 32) {
			f = (d & b) | ((~d) & c);
			g = (5 * i + 1) % 16;
		}
		else if (i < 48) {
			f = b ^ c ^ d;
			g = (3 * i + 5) % 16;
		}
		else {
			f = c ^ (b | (~d));
			g = (7 * i) % 16;
		}
		f += a + k[i] + little_to_host32(m[g]);
		a = d;
		d = c;
		c = b;
		b += left_rotate(f, s[i]);
	}

	state.a += a;
	state.b += b;
	state.c += c;
	state.d += d;

	state.msg_len += 512;

	return state;
}

static struct md5_state final_chunk(struct md5_state state, uint32_t const m[16], uint16_t bits) {
	assert(bits <= 512);
	uint32_t mm[16];
	ft_memcpy(mm, m, (bits + 7) / 8);

	// Precalculate because msg_len should not include the padding
	uint64_t total_msg_len = state.msg_len + bits;

	if (bits < 512) {
		uint8_t *bytes = (void*)mm;
		uint8_t partial_byte_index = bits / 8;
		uint8_t partial_bits = bits % 8;

		bytes[partial_byte_index] &= 0xFF << (8 - partial_bits);
		bytes[partial_byte_index] |= (uint32_t)1 << (8 - partial_bits - 1);
		
		for (uint8_t i = partial_byte_index + 1; i < 64; i++) {
			bytes[i] = 0;
		}
	}
	if (bits >= 512 - 64) {
		state = process_chunk(state, mm);
		for (uint8_t i = 0; i < 14; i++) {
			mm[i] = 0;
		}
	}
	*(uint64_t*)&mm[14] = host_to_little64(total_msg_len);

	state = process_chunk(state, mm);
	state.msg_len = total_msg_len;
	return state;
}

/// `m` should have a consistent order of bytes (endianness) on different hosts
/// `m` should be a block of 64 bytes (512 bits)
struct md5_state md5_round(struct md5_state state, uint8_t const m[64]) {
	if ((uintptr_t)m % alignof(uint32_t) == 0) {
		return process_chunk(state, (uint32_t*)m);
	}
	uint32_t mm[16];
	ft_memcpy(mm, m, 64);
	return process_chunk(state, mm);
}

/// `m` should have a consistent order of bytes (endianness) on different hosts
struct hash128 md5_final_round(struct md5_state state, uint8_t const m[64], uint16_t bits) {
	assert(bits <= 512);
	if ((uintptr_t)m % alignof(uint32_t) == 0) {
		state = final_chunk(state, (uint32_t*)m, bits);
	}
	else {
		uint32_t mm[16] = {0};
		ft_memcpy(mm, m, (bits + 7) / 8);
		state = final_chunk(state, mm, bits);
	}

#if BYTE_ORDER != LITTLE_ENDIAN
	state.a = host_to_little32(state.a);
	state.b = host_to_little32(state.b);
	state.c = host_to_little32(state.c);
	state.d = host_to_little32(state.d);
#endif

	struct hash128 hash;
	ft_memcpy(hash.hash + 0, &state.a, sizeof(state.a));
	ft_memcpy(hash.hash + 4, &state.b, sizeof(state.b));
	ft_memcpy(hash.hash + 8, &state.c, sizeof(state.c));
	ft_memcpy(hash.hash + 12, &state.d, sizeof(state.d));
	return hash;
}
