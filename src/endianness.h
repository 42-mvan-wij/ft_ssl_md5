#pragma once

#include <endian.h>
#include <stdint.h>

// Undef all these "functions" (macros) to ensure I don't accidentally use any of them
#undef htobe16
#undef htole16
#undef htobe32
#undef htole32
#undef htobe64
#undef htole64
#undef be16toh
#undef le16toh
#undef be32toh
#undef le32toh
#undef be64toh
#undef le64toh

#undef bswap_16
#undef bswap_32
#undef bswap_64


uint16_t byte_swap16(uint16_t n);
uint32_t byte_swap32(uint32_t n);
uint64_t byte_swap64(uint64_t n);

uint16_t host_to_little16(uint16_t n);
uint16_t host_to_big16(uint16_t n);
uint32_t host_to_little32(uint32_t n);
uint32_t host_to_big32(uint32_t n);
uint64_t host_to_little64(uint64_t n);
uint64_t host_to_big64(uint64_t n);
uint16_t little_to_host16(uint16_t n);
uint16_t big_to_host16(uint16_t n);
uint32_t little_to_host32(uint32_t n);
uint32_t big_to_host32(uint32_t n);
uint64_t little_to_host64(uint64_t n);
uint64_t big_to_host64(uint64_t n);
