#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

#include "endian.h"
#include "hash.h"
#include "utils.h"

struct md5 {
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;
};

__attribute__((no_sanitize("unsigned-integer-overflow")))
static struct md5 md5_round(struct md5 state, void const *buf) {
	static uint32_t const shift_amounts[64] = {
		7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
		5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
		4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
		6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,
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
	
	struct md5 original_state = state;
	uint32_t *int_buf = (void *)buf;

	for (size_t i = 0; i < 64; i++) {
		uint32_t f;
		size_t g;
		if (i < 16) {
			f = (state.b & state.c) | ((~state.b) & state.d);
			g = i;
		}
		else if (i < 32) {
			f = (state.d & state.b) | ((~state.d) & state.c);
			g = (i * 5 + 1) % 16;
		}
		else if (i < 48) {
			f = state.b ^ state.c ^ state.d;
			g = (i * 3 + 5) % 16;
		}
		else {
			f = state.c ^ (state.b | (~state.d));
			g = (i * 7) % 16;
		}

		f += state.a + k[i] + LITTLE_TO_HOST_ENDIAN(uint32_t, int_buf[g]);
		state.a = state.d;
		state.d = state.c;
		state.c = state.b;
		state.b += circular_left_shift(f, shift_amounts[i]);
	}
	state.a += original_state.a;
	state.b += original_state.b;
	state.c += original_state.c;
	state.d += original_state.d;
	return state;
}

static struct md5 md5_final_round(struct md5 state, uint8_t *buf, size_t buf_bit_size, uint64_t msg_bit_length) {
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
		state = md5_round(state, bytes);
		index = 0;
	}

	while (index < sizeof(bytes) - sizeof(msg_bit_length)) {
		bytes[index] = 0x00;
		index++;
	}
	uint8_t *msg_len_ptr = (void *)&msg_bit_length;
	host_to_little_endian_n(msg_len_ptr, sizeof(msg_bit_length));
	size_t len_index = 0;
	while (len_index < sizeof(msg_bit_length)) {
		bytes[index + len_index] = msg_len_ptr[len_index];
		len_index++;
	}
	return md5_round(state, bytes);
}

static struct md5 md5_initial_state(void) {
	return (struct md5){
		.a = 0x67452301,
		.b = 0xefcdab89,
		.c = 0x98badcfe,
		.d = 0x10325476,
	};
}

static struct hash128 md5_to_hash(struct md5 md5) {
	struct hash128 hash128;
	uint32_t *bytes = (void*)hash128.bytes;
#if BYTE_ORDER == LITTLE_ENDIAN
	bytes[0] = md5.a;
	bytes[1] = md5.b;
	bytes[2] = md5.c;
	bytes[3] = md5.d;
#elif BYTE_ORDER == BIG_ENDIAN
	bytes[0] = md5.d;
	bytes[1] = md5.c;
	bytes[2] = md5.b;
	bytes[3] = md5.a;
	little_to_big_endian_n(hash256.bytes, sizeof(hash256.bytes));
#else
#error "Unknown byte order"
#endif
	return hash128;
}

struct hash128 md5_buf(void *buf, size_t buf_size) {
	struct md5 state = md5_initial_state();
	size_t msg_length = buf_size;

	while (buf_size * 8 >= 512) {
		state = md5_round(state, buf);
		buf_size -= 512 / 8;
	}
	
	return md5_to_hash(md5_final_round(state, buf, buf_size * 8, msg_length * 8));
}

struct hash128 md5_fd(int fd) {
	struct md5 state = md5_initial_state();
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
				return md5_to_hash(md5_final_round(state, buf, buf_length * 8, msg_length * 8));
			}
		} while (buf_length != sizeof(buf));
		state = md5_round(state, buf);
	}
}

