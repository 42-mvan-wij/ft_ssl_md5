#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef PROGRAM
# define PROGRAM "ft_ssl"
#endif

void ft_memcpy(void *dst, void const *src, size_t bytes);
size_t ft_strlen(char const *s);
size_t ft_strlen_max(char const *str, size_t max);
void ft_putstr(int fd, char const *s);
void ft_putstrs(int fd, char const * const *strs);
bool ft_streq(char const *a, char const *b);
uint32_t left_rotate(uint32_t num, uint8_t rotate_amount);
uint32_t right_rotate(uint32_t num, uint8_t rotate_amount);
void print_escaped(int fd, uint8_t const *buffer, size_t len);
