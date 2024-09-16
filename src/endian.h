#pragma once

#include <stddef.h>
#include <stdint.h>

#define LITTLE_TO_HOST_ENDIAN(type, value) ({type ___v = value; little_to_host_endian_n(&___v, sizeof(type)); ___v;})
#define HOST_TO_LITTLE_ENDIAN(type, value) ({type ___v = value; host_to_little_endian_n(&___v, sizeof(type)); ___v;})

#define BIG_TO_HOST_ENDIAN(type, value) ({type ___v = value; big_to_host_endian_n(&___v, sizeof(type)); ___v;})
#define HOST_TO_BIG_ENDIAN(type, value) ({type ___v = value; host_to_big_endian_n(&___v, sizeof(type)); ___v;})

#define LITTLE_TO_BIG_ENDIAN(type, value) ({type ___v = value; little_to_big_endian_n(&___v, sizeof(type)); ___v;})
#define BIG_TO_LITTLE_ENDIAN(type, value) ({type ___v = value; big_to_little_endian_n(&___v, sizeof(type)); ___v;})

void little_to_host_endian_n(void *n, size_t bytes);
// uint32_t little_to_host_endian(uint32_t n);
void host_to_little_endian_n(void *n, size_t bytes);
// uint32_t host_to_little_endian(uint32_t n);
void big_to_host_endian_n(void *n, size_t bytes);
// uint32_t big_to_host_endian(uint32_t n);
void host_to_big_endian_n(void *n, size_t bytes);
// uint32_t host_to_big_endian(uint32_t n);
void little_to_big_endian_n(void *n, size_t bytes);
void big_to_little_endian_n(void *n, size_t bytes);
// uint32_t big_to_little_endian(uint32_t n);
// uint32_t little_to_big_endian(uint32_t n);

