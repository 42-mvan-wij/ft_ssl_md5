#pragma once

#include <stddef.h>

#include "hash.h"

struct hash256 sha256_buf(void *buf, size_t buf_size);
struct hash256 sha256_fd(int fd);
