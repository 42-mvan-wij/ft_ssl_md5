#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

#include "endian.h"
#include "hash.h"
#include "utils.h"

struct sha256 {
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;
	uint32_t e;
	uint32_t f;
	uint32_t g;
	uint32_t h;
};

__attribute__((no_sanitize("unsigned-integer-overflow")))
static struct sha256 sha256_round(struct sha256 state, void const *buf) {
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
	
	struct sha256 original_state = state;
	uint32_t w[64];
	ft_memcpy(w, buf, sizeof(uint32_t) * 16);
	uint32_t const *int_buf = buf;
	for (size_t i = 0; i < 16; i++) {
		w[i] = BIG_TO_HOST_ENDIAN(uint32_t, int_buf[i]);
	}

	for (size_t i = 16; i < 64; i++) {
		uint32_t s0 = circular_right_shift(w[i - 15], 7) ^ circular_right_shift(w[i - 15], 18) ^ (w[i - 15] >> 3);
		uint32_t s1 = circular_right_shift(w[i - 2], 17) ^ circular_right_shift(w[i - 2], 19) ^ (w[i - 2] >> 10);
		w[i] = w[i - 16] + s0 + w[i - 7] + s1;
	}

	for (size_t i = 0; i < 64; i++) {
		uint32_t ch = (state.e & state.f) ^ ((~state.e) & state.g);
		uint32_t s1 = circular_right_shift(state.e, 6) ^ circular_right_shift(state.e, 11) ^ circular_right_shift(state.e, 25);
		uint32_t temp1 = state.h + s1 + ch + k[i] + w[i];
		uint32_t maj = (state.a & state.b) ^ (state.a & state.c) ^ (state.b & state.c);
		uint32_t s0 = circular_right_shift(state.a, 2) ^ circular_right_shift(state.a, 13) ^ circular_right_shift(state.a, 22);
		uint32_t temp2 = s0 + maj;

		state.h = state.g;
		state.g = state.f;
		state.f = state.e;
		state.e = state.d + temp1;
		state.d = state.c;
		state.c = state.b;
		state.b = state.a;
		state.a = temp1 + temp2;
	}
	state.a += original_state.a;
	state.b += original_state.b;
	state.c += original_state.c;
	state.d += original_state.d;
	state.e += original_state.e;
	state.f += original_state.f;
	state.g += original_state.g;
	state.h += original_state.h;
	return state;
}

static struct sha256 sha256_final_round(struct sha256 state, uint8_t *buf, size_t buf_bit_size, uint64_t msg_bit_length) {
	uint8_t bytes[64];
	static_assert(sizeof(bytes) * 8 == 512, "Incorrect buf size");

	size_t index = 0;
	while (buf_bit_size >= 8) {
		bytes[index] = buf[index];
		index++;
		buf_bit_size -= 8;
	}
	if (buf_bit_size > 0) {
		bytes[index] = buf[index];
		bytes[index] |= 1 << (8 - buf_bit_size - 1);
		buf_bit_size = 0;
		index++;
	}
	else {
		bytes[index] = 0x80;
		index++;
	}

	if (sizeof(bytes) - index < sizeof(msg_bit_length)) {
		while (index < sizeof(bytes)) {
			bytes[index] = 0x00;
			index++;
		}
		state = sha256_round(state, bytes);
		index = 0;
	}

	while (index < sizeof(bytes) - sizeof(msg_bit_length)) {
		bytes[index] = 0x00;
		index++;
	}
	uint8_t *msg_len_ptr = (void *)&msg_bit_length;
	host_to_big_endian_n(msg_len_ptr, sizeof(msg_bit_length));
	size_t len_index = 0;
	while (len_index < sizeof(msg_bit_length)) {
		bytes[index + len_index] = msg_len_ptr[len_index];
		len_index++;
	}
	return sha256_round(state, bytes);
}

static struct sha256 sha256_initial_state(void) {
	return (struct sha256){
		.a = 0x6a09e667,
		.b = 0xbb67ae85,
		.c = 0x3c6ef372,
		.d = 0xa54ff53a,
		.e = 0x510e527f,
		.f = 0x9b05688c,
		.g = 0x1f83d9ab,
		.h = 0x5be0cd19,
	};
}

static struct hash256 sha256_to_hash(struct sha256 sha256) {
	struct hash256 hash256;
	uint32_t *bytes = (void*)hash256.bytes;
#if BYTE_ORDER == LITTLE_ENDIAN
	bytes[0] = sha256.h;
	bytes[1] = sha256.g;
	bytes[2] = sha256.f;
	bytes[3] = sha256.e;
	bytes[4] = sha256.d;
	bytes[5] = sha256.c;
	bytes[6] = sha256.b;
	bytes[7] = sha256.a;
	big_to_little_endian_n(hash256.bytes, sizeof(hash256.bytes));
#elif BYTE_ORDER == BIG_ENDIAN
	bytes[0] = sha256.a;
	bytes[1] = sha256.b;
	bytes[2] = sha256.c;
	bytes[3] = sha256.d;
	bytes[4] = sha256.e;
	bytes[5] = sha256.f;
	bytes[6] = sha256.g;
	bytes[7] = sha256.h;
#else
#error "Unknown byte order"
#endif
	return hash256;
}

struct hash256 sha256_buf(void *buf, size_t buf_size) {
	struct sha256 state = sha256_initial_state();
	size_t msg_length = buf_size;

	while (buf_size * 8 >= 512) {
		state = sha256_round(state, buf);
		buf_size -= 512 / 8;
	}
	
	return sha256_to_hash(sha256_final_round(state, buf, buf_size * 8, msg_length * 8));
}

struct hash256 sha256_fd(int fd) {
	struct sha256 state = sha256_initial_state();
	uint8_t buf[64];
	size_t buf_length = 0;
	uint64_t msg_length = 0;

	while (true) {
		buf_length = 0;
		do {
			ssize_t nread = read(fd, buf, sizeof(buf));
			if (nread < 0) {
				// FIXME: error
			}
			buf_length += nread;
			msg_length += nread;
			if (nread == 0) {
				return sha256_to_hash(sha256_final_round(state, buf, buf_length * 8, msg_length * 8));
			}
		} while (buf_length != sizeof(buf));
		state = sha256_round(state, buf);
	}
}

