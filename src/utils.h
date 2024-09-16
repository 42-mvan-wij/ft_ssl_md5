#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#define PROG_NAME "ft_ssl"
#define ERR_PREFIX PROG_NAME ": Error"

size_t ft_strlen(char const *str);
ssize_t ft_putstr(int fd, char const *str);
int ft_strcmp(char const *s1, char const *s2);
bool ft_streq(char const *s1, char const *s2);
// ssize_t ft_putendl(int fd, char *str);
bool ft_isprint(char c);
bool should_be_escaped(char c);
void print_escaped(int fd, char const *s, size_t len);
void ft_memcpy(void *dst, void const *src, size_t size);
void *ft_realloc(void *p, size_t old_size, size_t new_size);
char *read_to_string(int fd, size_t *len);
uint32_t circular_left_shift(uint32_t n, uint32_t shift_bits);
uint32_t circular_right_shift(uint32_t n, uint32_t shift_bits);
