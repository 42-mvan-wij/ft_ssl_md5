#include <stddef.h>
#include <stdint.h>

static uint32_t reverse_bytes(uint32_t n) {
	return
		((n >> (0 * 8)) & 0xff) << ((sizeof(n) - 1 - 0) * 8) |
		((n >> (1 * 8)) & 0xff) << ((sizeof(n) - 1 - 1) * 8) |
		((n >> (2 * 8)) & 0xff) << ((sizeof(n) - 1 - 2) * 8) |
		((n >> (3 * 8)) & 0xff) << ((sizeof(n) - 1 - 3) * 8);
}

static void reverse_bytes_n(void *n, size_t bytes) {
	uint8_t *p = n;
	size_t index = 0;
	while (index < bytes / 2) {
		uint8_t byte = p[index];
		p[index] = p[bytes - index - 1];
		p[bytes - index - 1] = byte;
		index++;
	}
}

void little_to_host_endian_n(void *n, size_t bytes) {
	#if BYTE_ORDER == LITTLE_ENDIAN
		(void)n;
		(void)bytes;
	#elif BYTE_ORDER == BIG_ENDIAN
		return reverse_bytes_n(n, bytes);
	#else
		#error "Unknown byte order"
	#endif
}

uint32_t little_to_host_endian(uint32_t n) {
	little_to_host_endian_n(&n, sizeof(n));
	return n;
}

void host_to_little_endian_n(void *n, size_t bytes) {
	return little_to_host_endian_n(n, bytes);
}

uint32_t host_to_little_endian(uint32_t n) {
	return little_to_host_endian(n);
}

void big_to_host_endian_n(void *n, size_t bytes) {
	#if BYTE_ORDER == LITTLE_ENDIAN
		return reverse_bytes_n(n, bytes);
	#elif BYTE_ORDER == BIG_ENDIAN
		(void)n;
		(void)bytes;
	#else
		#error "Unknown byte order"
	#endif
}

uint32_t big_to_host_endian(uint32_t n) {
	big_to_host_endian_n(&n, sizeof(n));
	return n;
}

void host_to_big_endian_n(void *n, size_t bytes) {
	return big_to_host_endian_n(n, bytes);
}

uint32_t host_to_big_endian(uint32_t n) {
	return big_to_host_endian(n);
}

void little_to_big_endian_n(void *n, size_t bytes) {
	return reverse_bytes_n(n, bytes);
}

void big_to_little_endian_n(void *n, size_t bytes) {
	return reverse_bytes_n(n, bytes);
}

uint32_t big_to_little_endian(uint32_t n) {
	return reverse_bytes(n);
}

uint32_t little_to_big_endian(uint32_t n) {
	return reverse_bytes(n);
}
