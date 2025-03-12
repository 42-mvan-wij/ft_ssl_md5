#include <assert.h>
#include <stdint.h>

#include "endianness.h"
#include "hash.h"
#include "utils.h"
#include "whirlpool.h"

struct whirlpool_state whirlpool_state(void) {
	struct whirlpool_state state = {
		.matrix = {
			.data = {0}
		},
	};
	return state;
}

static uint8_t const s_box[256] = {
	0x18, 0x23, 0xc6, 0xe8, 0x87, 0xb8, 0x01, 0x4f, 0x36, 0xa6, 0xd2, 0xf5, 0x79, 0x6f, 0x91, 0x52,
	0x60, 0xbc, 0x9b, 0x8e, 0xa3, 0x0c, 0x7b, 0x35, 0x1d, 0xe0, 0xd7, 0xc2, 0x2e, 0x4b, 0xfe, 0x57,
	0x15, 0x77, 0x37, 0xe5, 0x9f, 0xf0, 0x4a, 0xda, 0x58, 0xc9, 0x29, 0x0a, 0xb1, 0xa0, 0x6b, 0x85,
	0xbd, 0x5d, 0x10, 0xf4, 0xcb, 0x3e, 0x05, 0x67, 0xe4, 0x27, 0x41, 0x8b, 0xa7, 0x7d, 0x95, 0xd8,
	0xfb, 0xee, 0x7c, 0x66, 0xdd, 0x17, 0x47, 0x9e, 0xca, 0x2d, 0xbf, 0x07, 0xad, 0x5a, 0x83, 0x33,
	0x63, 0x02, 0xaa, 0x71, 0xc8, 0x19, 0x49, 0xd9, 0xf2, 0xe3, 0x5b, 0x88, 0x9a, 0x26, 0x32, 0xb0,
	0xe9, 0x0f, 0xd5, 0x80, 0xbe, 0xcd, 0x34, 0x48, 0xff, 0x7a, 0x90, 0x5f, 0x20, 0x68, 0x1a, 0xae,
	0xb4, 0x54, 0x93, 0x22, 0x64, 0xf1, 0x73, 0x12, 0x40, 0x08, 0xc3, 0xec, 0xdb, 0xa1, 0x8d, 0x3d,
	0x97, 0x00, 0xcf, 0x2b, 0x76, 0x82, 0xd6, 0x1b, 0xb5, 0xaf, 0x6a, 0x50, 0x45, 0xf3, 0x30, 0xef,
	0x3f, 0x55, 0xa2, 0xea, 0x65, 0xba, 0x2f, 0xc0, 0xde, 0x1c, 0xfd, 0x4d, 0x92, 0x75, 0x06, 0x8a,
	0xb2, 0xe6, 0x0e, 0x1f, 0x62, 0xd4, 0xa8, 0x96, 0xf9, 0xc5, 0x25, 0x59, 0x84, 0x72, 0x39, 0x4c,
	0x5e, 0x78, 0x38, 0x8c, 0xd1, 0xa5, 0xe2, 0x61, 0xb3, 0x21, 0x9c, 0x1e, 0x43, 0xc7, 0xfc, 0x04,
	0x51, 0x99, 0x6d, 0x0d, 0xfa, 0xdf, 0x7e, 0x24, 0x3b, 0xab, 0xce, 0x11, 0x8f, 0x4e, 0xb7, 0xeb,
	0x3c, 0x81, 0x94, 0xf7, 0xb9, 0x13, 0x2c, 0xd3, 0xe7, 0x6e, 0xc4, 0x03, 0x56, 0x44, 0x7f, 0xa9,
	0x2a, 0xbb, 0xc1, 0x53, 0xdc, 0x0b, 0x9d, 0x6c, 0x31, 0x74, 0xf6, 0x46, 0xac, 0x89, 0x14, 0xe1,
	0x16, 0x3a, 0x69, 0x09, 0x70, 0xb6, 0xd0, 0xed, 0xcc, 0x42, 0x98, 0xa4, 0x28, 0x5c, 0xf8, 0x86,
};

static void substitute_bytes(uint8_t matrix[64]) {
	for (uint8_t i = 0; i < 64; i++) {
		matrix[i] = s_box[matrix[i]];
	}
}

static void shift_columns(uint8_t matrix[64]) {
	for (uint8_t x = 0; x < 8; x++) {
		uint64_t c = 0;
		for (uint8_t y = 0; y < 8; y++) {
			c |= (uint64_t)matrix[x + y*8] << (y*8);
		}
		c = (c << (x * 8)) | (c >> (64 - x * 8));
		for (uint8_t y = 0; y < 8; y++) {
			matrix[x + y*8] = (c >> (y*8)) & 0xFF;
		}
	}
}

/// The multiplication is not regular multiplication, it's actually multiplication in GF(2^8)
/// Because it's non-standard it's easier to just precalculate
// static void precalculate_multiplication_table(void) {
// 	uint8_t v2[256];
// 	uint8_t v4[256];
// 	uint8_t v5[256];
// 	uint8_t v8[256];
// 	uint8_t v9[256];
//
// 	uint8_t v = 0;
// 	do {
// 		v2[v] = v << 1;
// 		if ((v & 0x80) != 0) {
// 			v2[v] ^= 0x1d;
// 		}
// 		v4[v] = v2[v] << 1;
// 		if ((v2[v] & 0x80) != 0) {
// 			v4[v] ^= 0x1d;
// 		}
// 		v5[v] = v4[v] ^ v;
// 		v8[v] = v4[v] << 1;
// 		if ((v4[v] & 0x80) != 0) {
// 			v8[v] ^= 0x1d;
// 		}
// 		v9[v] = v8[v] ^ v;
// 		v++;
// 	} while (v != 0);
//
// 	printf("static const uint8_t v1[256] = {\n");
// 	for (uint8_t i = 0; i < 16; i++) {
// 		printf("\t0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x,\n",
// 			i * 16 +  0, i * 16 +  1, i * 16 +  2, i * 16 +  3,
// 			i * 16 +  4, i * 16 +  5, i * 16 +  6, i * 16 +  7,
// 			i * 16 +  8, i * 16 +  9, i * 16 + 10, i * 16 + 11,
// 			i * 16 + 12, i * 16 + 13, i * 16 + 14, i * 16 + 15
// 		);
// 	}
// 	printf("};\n");
//
// 	printf("static const uint8_t v2[256] = {\n");
// 	for (uint8_t i = 0; i < 16; i++) {
// 		printf("\t0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x,\n",
// 			v2[i * 16 +  0], v2[i * 16 +  1], v2[i * 16 +  2], v2[i * 16 +  3],
// 			v2[i * 16 +  4], v2[i * 16 +  5], v2[i * 16 +  6], v2[i * 16 +  7],
// 			v2[i * 16 +  8], v2[i * 16 +  9], v2[i * 16 + 10], v2[i * 16 + 11],
// 			v2[i * 16 + 12], v2[i * 16 + 13], v2[i * 16 + 14], v2[i * 16 + 15]
// 		);
// 	}
// 	printf("};\n");
//
// 	printf("static const uint8_t v4[256] = {\n");
// 	for (uint8_t i = 0; i < 16; i++) {
// 		printf("\t0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x,\n",
// 			v4[i * 16 +  0], v4[i * 16 +  1], v4[i * 16 +  2], v4[i * 16 +  3],
// 			v4[i * 16 +  4], v4[i * 16 +  5], v4[i * 16 +  6], v4[i * 16 +  7],
// 			v4[i * 16 +  8], v4[i * 16 +  9], v4[i * 16 + 10], v4[i * 16 + 11],
// 			v4[i * 16 + 12], v4[i * 16 + 13], v4[i * 16 + 14], v4[i * 16 + 15]
// 		);
// 	}
// 	printf("};\n");
//
// 	printf("static const uint8_t v5[256] = {\n");
// 	for (uint8_t i = 0; i < 16; i++) {
// 		printf("\t0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x,\n",
// 			v5[i * 16 +  0], v5[i * 16 +  1], v5[i * 16 +  2], v5[i * 16 +  3],
// 			v5[i * 16 +  4], v5[i * 16 +  5], v5[i * 16 +  6], v5[i * 16 +  7],
// 			v5[i * 16 +  8], v5[i * 16 +  9], v5[i * 16 + 10], v5[i * 16 + 11],
// 			v5[i * 16 + 12], v5[i * 16 + 13], v5[i * 16 + 14], v5[i * 16 + 15]
// 		);
// 	}
// 	printf("};\n");
//
// 	printf("static const uint8_t v8[256] = {\n");
// 	for (uint8_t i = 0; i < 16; i++) {
// 		printf("\t0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x,\n",
// 			v8[i * 16 +  0], v8[i * 16 +  1], v8[i * 16 +  2], v8[i * 16 +  3],
// 			v8[i * 16 +  4], v8[i * 16 +  5], v8[i * 16 +  6], v8[i * 16 +  7],
// 			v8[i * 16 +  8], v8[i * 16 +  9], v8[i * 16 + 10], v8[i * 16 + 11],
// 			v8[i * 16 + 12], v8[i * 16 + 13], v8[i * 16 + 14], v8[i * 16 + 15]
// 		);
// 	}
// 	printf("};\n");
//
// 	printf("static const uint8_t v9[256] = {\n");
// 	for (uint8_t i = 0; i < 16; i++) {
// 		printf("\t0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x,\n",
// 			v9[i * 16 +  0], v9[i * 16 +  1], v9[i * 16 +  2], v9[i * 16 +  3],
// 			v9[i * 16 +  4], v9[i * 16 +  5], v9[i * 16 +  6], v9[i * 16 +  7],
// 			v9[i * 16 +  8], v9[i * 16 +  9], v9[i * 16 + 10], v9[i * 16 + 11],
// 			v9[i * 16 + 12], v9[i * 16 + 13], v9[i * 16 + 14], v9[i * 16 + 15]
// 		);
// 	}
// 	printf("};\n");
// 	fflush(NULL);
// }

static void mix_rows(uint8_t matrix[64]) {
	static const uint8_t v1[256] = {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
			0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
			0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
			0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
			0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
			0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
			0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
			0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
			0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
			0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
			0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
			0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
			0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
			0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
			0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
			0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
	};
	static const uint8_t v2[256] = {
			0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e,
			0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3a, 0x3c, 0x3e,
			0x40, 0x42, 0x44, 0x46, 0x48, 0x4a, 0x4c, 0x4e, 0x50, 0x52, 0x54, 0x56, 0x58, 0x5a, 0x5c, 0x5e,
			0x60, 0x62, 0x64, 0x66, 0x68, 0x6a, 0x6c, 0x6e, 0x70, 0x72, 0x74, 0x76, 0x78, 0x7a, 0x7c, 0x7e,
			0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c, 0x8e, 0x90, 0x92, 0x94, 0x96, 0x98, 0x9a, 0x9c, 0x9e,
			0xa0, 0xa2, 0xa4, 0xa6, 0xa8, 0xaa, 0xac, 0xae, 0xb0, 0xb2, 0xb4, 0xb6, 0xb8, 0xba, 0xbc, 0xbe,
			0xc0, 0xc2, 0xc4, 0xc6, 0xc8, 0xca, 0xcc, 0xce, 0xd0, 0xd2, 0xd4, 0xd6, 0xd8, 0xda, 0xdc, 0xde,
			0xe0, 0xe2, 0xe4, 0xe6, 0xe8, 0xea, 0xec, 0xee, 0xf0, 0xf2, 0xf4, 0xf6, 0xf8, 0xfa, 0xfc, 0xfe,
			0x1d, 0x1f, 0x19, 0x1b, 0x15, 0x17, 0x11, 0x13, 0x0d, 0x0f, 0x09, 0x0b, 0x05, 0x07, 0x01, 0x03,
			0x3d, 0x3f, 0x39, 0x3b, 0x35, 0x37, 0x31, 0x33, 0x2d, 0x2f, 0x29, 0x2b, 0x25, 0x27, 0x21, 0x23,
			0x5d, 0x5f, 0x59, 0x5b, 0x55, 0x57, 0x51, 0x53, 0x4d, 0x4f, 0x49, 0x4b, 0x45, 0x47, 0x41, 0x43,
			0x7d, 0x7f, 0x79, 0x7b, 0x75, 0x77, 0x71, 0x73, 0x6d, 0x6f, 0x69, 0x6b, 0x65, 0x67, 0x61, 0x63,
			0x9d, 0x9f, 0x99, 0x9b, 0x95, 0x97, 0x91, 0x93, 0x8d, 0x8f, 0x89, 0x8b, 0x85, 0x87, 0x81, 0x83,
			0xbd, 0xbf, 0xb9, 0xbb, 0xb5, 0xb7, 0xb1, 0xb3, 0xad, 0xaf, 0xa9, 0xab, 0xa5, 0xa7, 0xa1, 0xa3,
			0xdd, 0xdf, 0xd9, 0xdb, 0xd5, 0xd7, 0xd1, 0xd3, 0xcd, 0xcf, 0xc9, 0xcb, 0xc5, 0xc7, 0xc1, 0xc3,
			0xfd, 0xff, 0xf9, 0xfb, 0xf5, 0xf7, 0xf1, 0xf3, 0xed, 0xef, 0xe9, 0xeb, 0xe5, 0xe7, 0xe1, 0xe3,
	};
	static const uint8_t v4[256] = {
			0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c,
			0x40, 0x44, 0x48, 0x4c, 0x50, 0x54, 0x58, 0x5c, 0x60, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x7c,
			0x80, 0x84, 0x88, 0x8c, 0x90, 0x94, 0x98, 0x9c, 0xa0, 0xa4, 0xa8, 0xac, 0xb0, 0xb4, 0xb8, 0xbc,
			0xc0, 0xc4, 0xc8, 0xcc, 0xd0, 0xd4, 0xd8, 0xdc, 0xe0, 0xe4, 0xe8, 0xec, 0xf0, 0xf4, 0xf8, 0xfc,
			0x1d, 0x19, 0x15, 0x11, 0x0d, 0x09, 0x05, 0x01, 0x3d, 0x39, 0x35, 0x31, 0x2d, 0x29, 0x25, 0x21,
			0x5d, 0x59, 0x55, 0x51, 0x4d, 0x49, 0x45, 0x41, 0x7d, 0x79, 0x75, 0x71, 0x6d, 0x69, 0x65, 0x61,
			0x9d, 0x99, 0x95, 0x91, 0x8d, 0x89, 0x85, 0x81, 0xbd, 0xb9, 0xb5, 0xb1, 0xad, 0xa9, 0xa5, 0xa1,
			0xdd, 0xd9, 0xd5, 0xd1, 0xcd, 0xc9, 0xc5, 0xc1, 0xfd, 0xf9, 0xf5, 0xf1, 0xed, 0xe9, 0xe5, 0xe1,
			0x3a, 0x3e, 0x32, 0x36, 0x2a, 0x2e, 0x22, 0x26, 0x1a, 0x1e, 0x12, 0x16, 0x0a, 0x0e, 0x02, 0x06,
			0x7a, 0x7e, 0x72, 0x76, 0x6a, 0x6e, 0x62, 0x66, 0x5a, 0x5e, 0x52, 0x56, 0x4a, 0x4e, 0x42, 0x46,
			0xba, 0xbe, 0xb2, 0xb6, 0xaa, 0xae, 0xa2, 0xa6, 0x9a, 0x9e, 0x92, 0x96, 0x8a, 0x8e, 0x82, 0x86,
			0xfa, 0xfe, 0xf2, 0xf6, 0xea, 0xee, 0xe2, 0xe6, 0xda, 0xde, 0xd2, 0xd6, 0xca, 0xce, 0xc2, 0xc6,
			0x27, 0x23, 0x2f, 0x2b, 0x37, 0x33, 0x3f, 0x3b, 0x07, 0x03, 0x0f, 0x0b, 0x17, 0x13, 0x1f, 0x1b,
			0x67, 0x63, 0x6f, 0x6b, 0x77, 0x73, 0x7f, 0x7b, 0x47, 0x43, 0x4f, 0x4b, 0x57, 0x53, 0x5f, 0x5b,
			0xa7, 0xa3, 0xaf, 0xab, 0xb7, 0xb3, 0xbf, 0xbb, 0x87, 0x83, 0x8f, 0x8b, 0x97, 0x93, 0x9f, 0x9b,
			0xe7, 0xe3, 0xef, 0xeb, 0xf7, 0xf3, 0xff, 0xfb, 0xc7, 0xc3, 0xcf, 0xcb, 0xd7, 0xd3, 0xdf, 0xdb,
	};
	static const uint8_t v5[256] = {
			0x00, 0x05, 0x0a, 0x0f, 0x14, 0x11, 0x1e, 0x1b, 0x28, 0x2d, 0x22, 0x27, 0x3c, 0x39, 0x36, 0x33,
			0x50, 0x55, 0x5a, 0x5f, 0x44, 0x41, 0x4e, 0x4b, 0x78, 0x7d, 0x72, 0x77, 0x6c, 0x69, 0x66, 0x63,
			0xa0, 0xa5, 0xaa, 0xaf, 0xb4, 0xb1, 0xbe, 0xbb, 0x88, 0x8d, 0x82, 0x87, 0x9c, 0x99, 0x96, 0x93,
			0xf0, 0xf5, 0xfa, 0xff, 0xe4, 0xe1, 0xee, 0xeb, 0xd8, 0xdd, 0xd2, 0xd7, 0xcc, 0xc9, 0xc6, 0xc3,
			0x5d, 0x58, 0x57, 0x52, 0x49, 0x4c, 0x43, 0x46, 0x75, 0x70, 0x7f, 0x7a, 0x61, 0x64, 0x6b, 0x6e,
			0x0d, 0x08, 0x07, 0x02, 0x19, 0x1c, 0x13, 0x16, 0x25, 0x20, 0x2f, 0x2a, 0x31, 0x34, 0x3b, 0x3e,
			0xfd, 0xf8, 0xf7, 0xf2, 0xe9, 0xec, 0xe3, 0xe6, 0xd5, 0xd0, 0xdf, 0xda, 0xc1, 0xc4, 0xcb, 0xce,
			0xad, 0xa8, 0xa7, 0xa2, 0xb9, 0xbc, 0xb3, 0xb6, 0x85, 0x80, 0x8f, 0x8a, 0x91, 0x94, 0x9b, 0x9e,
			0xba, 0xbf, 0xb0, 0xb5, 0xae, 0xab, 0xa4, 0xa1, 0x92, 0x97, 0x98, 0x9d, 0x86, 0x83, 0x8c, 0x89,
			0xea, 0xef, 0xe0, 0xe5, 0xfe, 0xfb, 0xf4, 0xf1, 0xc2, 0xc7, 0xc8, 0xcd, 0xd6, 0xd3, 0xdc, 0xd9,
			0x1a, 0x1f, 0x10, 0x15, 0x0e, 0x0b, 0x04, 0x01, 0x32, 0x37, 0x38, 0x3d, 0x26, 0x23, 0x2c, 0x29,
			0x4a, 0x4f, 0x40, 0x45, 0x5e, 0x5b, 0x54, 0x51, 0x62, 0x67, 0x68, 0x6d, 0x76, 0x73, 0x7c, 0x79,
			0xe7, 0xe2, 0xed, 0xe8, 0xf3, 0xf6, 0xf9, 0xfc, 0xcf, 0xca, 0xc5, 0xc0, 0xdb, 0xde, 0xd1, 0xd4,
			0xb7, 0xb2, 0xbd, 0xb8, 0xa3, 0xa6, 0xa9, 0xac, 0x9f, 0x9a, 0x95, 0x90, 0x8b, 0x8e, 0x81, 0x84,
			0x47, 0x42, 0x4d, 0x48, 0x53, 0x56, 0x59, 0x5c, 0x6f, 0x6a, 0x65, 0x60, 0x7b, 0x7e, 0x71, 0x74,
			0x17, 0x12, 0x1d, 0x18, 0x03, 0x06, 0x09, 0x0c, 0x3f, 0x3a, 0x35, 0x30, 0x2b, 0x2e, 0x21, 0x24,
	};
	static const uint8_t v8[256] = {
			0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78,
			0x80, 0x88, 0x90, 0x98, 0xa0, 0xa8, 0xb0, 0xb8, 0xc0, 0xc8, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,
			0x1d, 0x15, 0x0d, 0x05, 0x3d, 0x35, 0x2d, 0x25, 0x5d, 0x55, 0x4d, 0x45, 0x7d, 0x75, 0x6d, 0x65,
			0x9d, 0x95, 0x8d, 0x85, 0xbd, 0xb5, 0xad, 0xa5, 0xdd, 0xd5, 0xcd, 0xc5, 0xfd, 0xf5, 0xed, 0xe5,
			0x3a, 0x32, 0x2a, 0x22, 0x1a, 0x12, 0x0a, 0x02, 0x7a, 0x72, 0x6a, 0x62, 0x5a, 0x52, 0x4a, 0x42,
			0xba, 0xb2, 0xaa, 0xa2, 0x9a, 0x92, 0x8a, 0x82, 0xfa, 0xf2, 0xea, 0xe2, 0xda, 0xd2, 0xca, 0xc2,
			0x27, 0x2f, 0x37, 0x3f, 0x07, 0x0f, 0x17, 0x1f, 0x67, 0x6f, 0x77, 0x7f, 0x47, 0x4f, 0x57, 0x5f,
			0xa7, 0xaf, 0xb7, 0xbf, 0x87, 0x8f, 0x97, 0x9f, 0xe7, 0xef, 0xf7, 0xff, 0xc7, 0xcf, 0xd7, 0xdf,
			0x74, 0x7c, 0x64, 0x6c, 0x54, 0x5c, 0x44, 0x4c, 0x34, 0x3c, 0x24, 0x2c, 0x14, 0x1c, 0x04, 0x0c,
			0xf4, 0xfc, 0xe4, 0xec, 0xd4, 0xdc, 0xc4, 0xcc, 0xb4, 0xbc, 0xa4, 0xac, 0x94, 0x9c, 0x84, 0x8c,
			0x69, 0x61, 0x79, 0x71, 0x49, 0x41, 0x59, 0x51, 0x29, 0x21, 0x39, 0x31, 0x09, 0x01, 0x19, 0x11,
			0xe9, 0xe1, 0xf9, 0xf1, 0xc9, 0xc1, 0xd9, 0xd1, 0xa9, 0xa1, 0xb9, 0xb1, 0x89, 0x81, 0x99, 0x91,
			0x4e, 0x46, 0x5e, 0x56, 0x6e, 0x66, 0x7e, 0x76, 0x0e, 0x06, 0x1e, 0x16, 0x2e, 0x26, 0x3e, 0x36,
			0xce, 0xc6, 0xde, 0xd6, 0xee, 0xe6, 0xfe, 0xf6, 0x8e, 0x86, 0x9e, 0x96, 0xae, 0xa6, 0xbe, 0xb6,
			0x53, 0x5b, 0x43, 0x4b, 0x73, 0x7b, 0x63, 0x6b, 0x13, 0x1b, 0x03, 0x0b, 0x33, 0x3b, 0x23, 0x2b,
			0xd3, 0xdb, 0xc3, 0xcb, 0xf3, 0xfb, 0xe3, 0xeb, 0x93, 0x9b, 0x83, 0x8b, 0xb3, 0xbb, 0xa3, 0xab,
	};
	static const uint8_t v9[256] = {
			0x00, 0x09, 0x12, 0x1b, 0x24, 0x2d, 0x36, 0x3f, 0x48, 0x41, 0x5a, 0x53, 0x6c, 0x65, 0x7e, 0x77,
			0x90, 0x99, 0x82, 0x8b, 0xb4, 0xbd, 0xa6, 0xaf, 0xd8, 0xd1, 0xca, 0xc3, 0xfc, 0xf5, 0xee, 0xe7,
			0x3d, 0x34, 0x2f, 0x26, 0x19, 0x10, 0x0b, 0x02, 0x75, 0x7c, 0x67, 0x6e, 0x51, 0x58, 0x43, 0x4a,
			0xad, 0xa4, 0xbf, 0xb6, 0x89, 0x80, 0x9b, 0x92, 0xe5, 0xec, 0xf7, 0xfe, 0xc1, 0xc8, 0xd3, 0xda,
			0x7a, 0x73, 0x68, 0x61, 0x5e, 0x57, 0x4c, 0x45, 0x32, 0x3b, 0x20, 0x29, 0x16, 0x1f, 0x04, 0x0d,
			0xea, 0xe3, 0xf8, 0xf1, 0xce, 0xc7, 0xdc, 0xd5, 0xa2, 0xab, 0xb0, 0xb9, 0x86, 0x8f, 0x94, 0x9d,
			0x47, 0x4e, 0x55, 0x5c, 0x63, 0x6a, 0x71, 0x78, 0x0f, 0x06, 0x1d, 0x14, 0x2b, 0x22, 0x39, 0x30,
			0xd7, 0xde, 0xc5, 0xcc, 0xf3, 0xfa, 0xe1, 0xe8, 0x9f, 0x96, 0x8d, 0x84, 0xbb, 0xb2, 0xa9, 0xa0,
			0xf4, 0xfd, 0xe6, 0xef, 0xd0, 0xd9, 0xc2, 0xcb, 0xbc, 0xb5, 0xae, 0xa7, 0x98, 0x91, 0x8a, 0x83,
			0x64, 0x6d, 0x76, 0x7f, 0x40, 0x49, 0x52, 0x5b, 0x2c, 0x25, 0x3e, 0x37, 0x08, 0x01, 0x1a, 0x13,
			0xc9, 0xc0, 0xdb, 0xd2, 0xed, 0xe4, 0xff, 0xf6, 0x81, 0x88, 0x93, 0x9a, 0xa5, 0xac, 0xb7, 0xbe,
			0x59, 0x50, 0x4b, 0x42, 0x7d, 0x74, 0x6f, 0x66, 0x11, 0x18, 0x03, 0x0a, 0x35, 0x3c, 0x27, 0x2e,
			0x8e, 0x87, 0x9c, 0x95, 0xaa, 0xa3, 0xb8, 0xb1, 0xc6, 0xcf, 0xd4, 0xdd, 0xe2, 0xeb, 0xf0, 0xf9,
			0x1e, 0x17, 0x0c, 0x05, 0x3a, 0x33, 0x28, 0x21, 0x56, 0x5f, 0x44, 0x4d, 0x72, 0x7b, 0x60, 0x69,
			0xb3, 0xba, 0xa1, 0xa8, 0x97, 0x9e, 0x85, 0x8c, 0xfb, 0xf2, 0xe9, 0xe0, 0xdf, 0xd6, 0xcd, 0xc4,
			0x23, 0x2a, 0x31, 0x38, 0x07, 0x0e, 0x15, 0x1c, 0x6b, 0x62, 0x79, 0x70, 0x4f, 0x46, 0x5d, 0x54,
	};

	static uint8_t const *const c[64] = {
		v1, v1, v4, v1, v8, v5, v2, v9,
		v9, v1, v1, v4, v1, v8, v5, v2,
		v2, v9, v1, v1, v4, v1, v8, v5,
		v5, v2, v9, v1, v1, v4, v1, v8,
		v8, v5, v2, v9, v1, v1, v4, v1,
		v1, v8, v5, v2, v9, v1, v1, v4,
		v4, v1, v8, v5, v2, v9, v1, v1,
		v1, v4, v1, v8, v5, v2, v9, v1,
	};
	uint8_t res[64] = {0};

	for (uint8_t y = 0; y < 8; y++) {
		for (uint8_t x = 0; x < 8; x++) {
			for (uint8_t index = 0; index < 8; index++) {
				uint8_t const *v = c[x + index * 8];
				res[x + y * 8] ^= v[matrix[index + y * 8]];
			}
		}
	}

	ft_memcpy(matrix, res, sizeof(res));
}

static void add_key(uint8_t matrix[64], uint8_t const add[64]) {
	for (uint8_t y = 0; y < 8; y++) {
		for (uint8_t x = 0; x < 8; x++) {
			matrix[x + y*8] ^= add[x + y*8];
		}
	}
}

// static void print_matrices(uint8_t const k[64], uint8_t const m[64]) {
// 	for (int y = 0; y < 8; y++) {
// 		for (int x = 0; x < 8; x++) {
// 			printf("%.2X ", k[x + y*8]);
// 		}
// 		printf("      ");
// 		for (int x = 0; x < 8; x++) {
// 			printf("%.2X ", m[x + y*8]);
// 		}
// 		printf("\n");
// 	}
// 	printf("\n");
// }

// static void print_matrix(uint8_t const m[64]) {
// 	for (int y = 0; y < 8; y++) {
// 		for (int x = 0; x < 8; x++) {
// 			printf("%.2X ", m[x + y*8]);
// 		}
// 		printf("\n");
// 	}
// 	printf("\n");
// }

static struct matrix W(uint8_t const m[64], uint8_t const k[64]) {
	struct matrix rc = {0};

	struct matrix mm;
	struct matrix kk;
	ft_memcpy(mm.data, m, sizeof(mm.data));
	ft_memcpy(kk.data, k, sizeof(kk.data));

	add_key(mm.data, kk.data);

	// printf("key 0:                        m 0:\n");
	// print_matrices(kk, mm.data);

	for (uint8_t i = 0; i < 10; i++) {
		substitute_bytes(mm.data); substitute_bytes(kk.data);
		// printf("\nafter substitution:\n");
		// print_matrix(mm.data);
		shift_columns(mm.data); shift_columns(kk.data);
		// printf("\nafter column shift:\n");
		// print_matrix(mm.data);
		mix_rows(mm.data); mix_rows(kk.data);
		// printf("\nafter mixing rows:\n");
		// print_matrix(mm.data);
		for (uint8_t j = 0; j < 8; j++) {
			rc.data[j] = s_box[j + i*8];
		}
		add_key(kk.data, rc.data);
		add_key(mm.data, kk.data);

		// printf("key %i:                        m %i:\n", i + 1, i + 1);
		// print_matrices(kk.data, mm.data);
	}
	return mm;
}

static void add_msg_len(uint64_t msg_len[4], uint16_t bits) {
	if (msg_len[0] > UINT64_MAX - bits) {
		if (msg_len[1] == UINT64_MAX) {
			if (msg_len[2] == UINT64_MAX) {
				msg_len[3]++;
			}
			msg_len[2]++;
		}
		msg_len[1]++;
	}
	msg_len[0] += bits;
}

static struct whirlpool_state process_chunk(struct whirlpool_state state, uint8_t const m[64]) {
	// printf("Derived matrix:\n");
	// print_matrix(m);

	add_key(state.matrix.data, W(m, state.matrix.data).data);
	add_key(state.matrix.data, m);

	add_msg_len(state.msg_len, 512);
	return state;
}

static struct whirlpool_state final_chunk(struct whirlpool_state state, uint8_t const m[64], uint16_t bits) {
	assert(bits <= 512);

	uint8_t mm[64];
	ft_memcpy(mm, m, (bits + 7) / 8);

	// Precalculate because msg_len should not include the padding
	uint64_t total_msg_len[4] = {state.msg_len[0], state.msg_len[1], state.msg_len[2], state.msg_len[3]};
	add_msg_len(total_msg_len, bits);

	if (bits < 512) {
		uint8_t partial_byte_index = bits / 8;
		uint8_t partial_bits = bits % 8;

		mm[partial_byte_index] &= 0xFF << (8 - partial_bits);
		mm[partial_byte_index] |= 1 << (8 - partial_bits - 1);
		
		for (uint8_t i = partial_byte_index + 1; i < 64; i++) {
			mm[i] = 0;
		}
	}
	if (bits >= 512 - 256) {
		state = process_chunk(state, mm);
		for (uint8_t i = 0; i < 32; i++) {
			mm[i] = 0;
		}
	}
	*(uint64_t*)&mm[32] = host_to_big64(total_msg_len[3]);
	*(uint64_t*)&mm[40] = host_to_big64(total_msg_len[2]);
	*(uint64_t*)&mm[48] = host_to_big64(total_msg_len[1]);
	*(uint64_t*)&mm[56] = host_to_big64(total_msg_len[0]);

	state = process_chunk(state, mm);

	state.msg_len[0] = total_msg_len[0];
	state.msg_len[1] = total_msg_len[1];
	state.msg_len[2] = total_msg_len[2];
	state.msg_len[3] = total_msg_len[3];

	// printf("final state:\n");
	// print_matrix(state.matrix.data);
	return state;
}

/// `m` should have a consistent order of bytes (endianness) on different hosts
/// `m` should be a block of 64 bytes (512 bits)
struct whirlpool_state whirlpool_round(struct whirlpool_state state, uint8_t const m[64]) {
	return process_chunk(state, m);
}

/// `m` should have a consistent order of bytes (endianness) on different hosts
/// `m` should be a block of at most 512 bits (64 bytes)
struct hash512 whirlpool_final_round(struct whirlpool_state state, uint8_t const m[64], uint16_t bits) {
	assert(bits <= 512);
	state = final_chunk(state, m, bits);

	struct hash512 hash;
	ft_memcpy(hash.hash, state.matrix.data, sizeof(hash.hash));

	// fflush(NULL);

	// precalculate_multiplication_table();
	return hash;
}
