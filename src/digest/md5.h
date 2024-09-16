#pragma once

#include <stddef.h>

#include "./hash.h"

struct hash128 md5_buf(void *buf, size_t buf_size);
struct hash128 md5_fd(int fd);
