#include <assert.h>
#include <unistd.h>

#include "utils.h"

void ft_memcpy(void *dst, void const *src, size_t bytes) {
	uint8_t *dst_p = dst;
	uint8_t const *src_p = src;
	for (size_t i = 0; i < bytes; i++) {
		dst_p[i] = src_p[i];
	}
}

size_t ft_strlen(char const *s) {
	size_t i = 0;
	while (s[i] != '\0') {
		i++;
	}
	return i;
}

size_t ft_strlen_max(char const *str, size_t max) {
	size_t len = 0;
	while (len < max && str[len] != '\0') {
		len++;
	}
	return len;
}

void ft_putstr(int fd, char const *s) {
	write(fd, s, ft_strlen(s));
}

void ft_putstrs(int fd, char const * const *strs) {
	char buffer[1024];
	size_t buffer_index = 0;
	while (*strs != NULL) {
		size_t len = ft_strlen(*strs);
		if (buffer_index + len <= sizeof(buffer)) {
			ft_memcpy(buffer + buffer_index, *strs, len);
			buffer_index += len;
		}
		else {
			write(fd, buffer, buffer_index);
			buffer_index = 0;
			if (len <= sizeof(buffer)) {
				ft_memcpy(buffer + buffer_index, *strs, len);
				buffer_index += len;
			}
			else {
				write(fd, *strs, ft_strlen(*strs));
			}
		}
		strs++;
	}
	if (buffer_index > 0) {
		write(fd, buffer, buffer_index);
	}
}

bool ft_streq(char const *a, char const *b) {
	size_t i = 0;
	while (a[i] == b[i] && a[i] != '\0' && b[i] != '\0') {
		i++;
	}
	return a[i] == b[i];
}

uint32_t right_rotate(uint32_t num, uint8_t rotate_amount) {
	return (num >> rotate_amount) | (num << (32 - rotate_amount));
}

uint32_t left_rotate(uint32_t num, uint8_t rotate_amount) {
	return (num << rotate_amount) | (num >> (32 - rotate_amount));
}

#define ESCAPE_BLOCK_INPUT 256
#define MAX_ESCAPED_LEN 4

static bool needs_escaping(uint8_t c) {
	return c <= 31 || c == '"' || c == '\\' || c >= 127;
}

static uint8_t escape(uint8_t c, char *escaped) {
	static char const hex_digits[] = "0123456789abcdef";
	switch (c) {
		case '"': {
			escaped[0] = '\\';
			escaped[1] = '"';
			return 2;
		}; break;
		case '\\': {
			escaped[0] = '\\';
			escaped[1] = '\\';
			return 2;
		}; break;
		case '\0': {
			escaped[0] = '\\';
			escaped[1] = '0';
			return 2;
		}; break;
		case '\t': {
			escaped[0] = '\\';
			escaped[1] = 't';
			return 2;
		}; break;
		case '\n': {
			escaped[0] = '\\';
			escaped[1] = 'n';
			return 2;
		}; break;
		case '\r': {
			escaped[0] = '\\';
			escaped[1] = 'r';
			return 2;
		}; break;
		default: {
			escaped[0] = '\\';
			escaped[1] = 'x';
			escaped[2] = hex_digits[c / 16];
			escaped[3] = hex_digits[c % 16];
			return 4;
		} break;
	}
}

static void print_escaped_block(int fd, uint8_t const *buffer, size_t len) {
	assert(len <= ESCAPE_BLOCK_INPUT);
	char escaped[ESCAPE_BLOCK_INPUT * MAX_ESCAPED_LEN];
	size_t escaped_index = 0;
	size_t buffer_index = 0;
	while (buffer_index < len) {
		size_t buffer_index_start = buffer_index;
		while (buffer_index < len && !needs_escaping(buffer[buffer_index])) {
			buffer_index++;
		}
		ft_memcpy(escaped + escaped_index, buffer + buffer_index_start, buffer_index - buffer_index_start);
		escaped_index += buffer_index - buffer_index_start;
		while (buffer_index < len && needs_escaping(buffer[buffer_index])) {
			escaped_index += escape(buffer[buffer_index], escaped + escaped_index);
			buffer_index++;
		}
	}
	write(fd, escaped, escaped_index);
}

void print_escaped(int fd, uint8_t const *buffer, size_t len) {
	while (len > ESCAPE_BLOCK_INPUT) {
		print_escaped_block(fd, buffer, ESCAPE_BLOCK_INPUT);
		buffer += ESCAPE_BLOCK_INPUT;
		len -= ESCAPE_BLOCK_INPUT;
	}
	if (len > 0) {
		print_escaped_block(fd, buffer, len);
	}
}
