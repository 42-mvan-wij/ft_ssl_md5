#include <assert.h>
#include <stdalign.h>

#include "endianness.h"
#include "sha256.h"
#include "utils.h"

struct sha256_state sha256_state(void) {
	struct sha256_state state = {
		.a = 0x6a09e667,
		.b = 0xbb67ae85,
		.c = 0x3c6ef372,
		.d = 0xa54ff53a,
		.e = 0x510e527f,
		.f = 0x9b05688c,
		.g = 0x1f83d9ab,
		.h = 0x5be0cd19,

		.msg_len = 0,
	};

	return state;
}

/// Each block in `m` is host-endian, the blocks are in big-endian
static struct sha256_state process_chunk(struct sha256_state state, uint32_t const m[16]) {
	static uint32_t const k[64] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};

	uint32_t w[64];

	for (uint8_t i = 0; i < 16; i++) {
		w[i] = big_to_host32(m[i]);
	}
	for (uint8_t i = 16; i < 64; i++) {
		uint32_t s0 = right_rotate(w[i - 15], 7) ^ right_rotate(w[i - 15], 18) ^ (w[i - 15] >> 3);
		uint32_t s1 = right_rotate(w[i - 2], 17) ^ right_rotate(w[i - 2], 19) ^ (w[i - 2] >> 10);
		w[i] = w[i - 16] + s0 + w[i - 7] + s1;
	}

	uint32_t a = state.a;
	uint32_t b = state.b;
	uint32_t c = state.c;
	uint32_t d = state.d;
	uint32_t e = state.e;
	uint32_t f = state.f;
	uint32_t g = state.g;
	uint32_t h = state.h;

	for (uint8_t i = 0; i < 64; i++) {
		uint32_t s1 = right_rotate(e, 6) ^ right_rotate(e, 11) ^ right_rotate(e, 25);
		uint32_t choice = (e & f) ^ ((~e) & g);
		uint32_t temp1 = h + s1 + choice + k[i] + w[i];
		uint32_t s0 = right_rotate(a, 2) ^ right_rotate(a, 13) ^ right_rotate(a, 22);
		uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
		uint32_t temp2 = s0 + majority;

		h = g;
		g = f;
		f = e;
		e = d + temp1;
		d = c;
		c = b;
		b = a;
		a = temp1 + temp2;
	}

	state.a += a;
	state.b += b;
	state.c += c;
	state.d += d;
	state.e += e;
	state.f += f;
	state.g += g;
	state.h += h;

	state.msg_len += 512;

	return state;
}

static struct sha256_state final_chunk(struct sha256_state state, uint32_t const m[16], uint16_t bits) {
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
		bytes[partial_byte_index] |= 1 << (8 - partial_bits - 1);
		
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
	*(uint64_t*)&mm[14] = host_to_big64(total_msg_len);

	state = process_chunk(state, mm);
	state.msg_len = total_msg_len;
	return state;
}

/// `m` should have a consistent order of bytes (endianness) on different hosts
/// `m` should be a block of 64 bytes (512 bits)
struct sha256_state sha256_round(struct sha256_state state, uint8_t const m[64]) {
	if ((uintptr_t)m % alignof(uint32_t) == 0) {
		return process_chunk(state, (uint32_t*)m);
	}
	uint32_t mm[16];
	ft_memcpy(mm, m, 64);
	return process_chunk(state, mm);
}

/// `m` should have a consistent order of bytes (endianness) on different hosts
/// `m` should be a block of at most 512 bits (64 bytes)
struct hash256 sha256_final_round(struct sha256_state state, uint8_t const m[64], uint16_t bits) {
	assert(bits <= 512);
	if ((uintptr_t)m % alignof(uint32_t) == 0) {
		state = final_chunk(state, (uint32_t*)m, bits);
	}
	else {
		uint32_t mm[16] = {0};
		ft_memcpy(mm, m, (bits + 7) / 8);
		state = final_chunk(state, mm, bits);
	}

#if BYTE_ORDER != BIG_ENDIAN
	state.a = host_to_big32(state.a);
	state.b = host_to_big32(state.b);
	state.c = host_to_big32(state.c);
	state.d = host_to_big32(state.d);
	state.e = host_to_big32(state.e);
	state.f = host_to_big32(state.f);
	state.g = host_to_big32(state.g);
	state.h = host_to_big32(state.h);
#endif

	struct hash256 hash;
	ft_memcpy(hash.hash + 0, &state.a, sizeof(state.a));
	ft_memcpy(hash.hash + 4, &state.b, sizeof(state.b));
	ft_memcpy(hash.hash + 8, &state.c, sizeof(state.c));
	ft_memcpy(hash.hash + 12, &state.d, sizeof(state.d));
	ft_memcpy(hash.hash + 16, &state.e, sizeof(state.e));
	ft_memcpy(hash.hash + 20, &state.f, sizeof(state.f));
	ft_memcpy(hash.hash + 24, &state.g, sizeof(state.g));
	ft_memcpy(hash.hash + 28, &state.h, sizeof(state.h));
	return hash;
}
