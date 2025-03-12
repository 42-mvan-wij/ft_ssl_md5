#pragma once

#include <errno.h>

#ifndef MAX_ERR_MSG_LEN
# define MAX_ERR_MSG_LEN 1023
#endif
#ifndef MAX_ERR_PREFIX_LEN
# define MAX_ERR_PREFIX_LEN 15
#endif
#ifndef MAX_ERR_OBJECT_LEN
# define MAX_ERR_OBJECT_LEN 63
#endif

typedef enum e_result {
	OK = 0,
	FAIL,
} __attribute__((warn_unused_result)) t_result;

typedef enum e_error {
	E_NONE = 0,
	E_ERRNO,
	E_DUPLICATE_OPT,
	E_OPT_MISSING_VALUE,
	E_UNEXPECTED_OPT,
} t_error;

t_result reset_error(void);
t_result propagate_error(void);
void set_err_prefix(char const *prefix);
void set_err_object(char const *object);
void set_err_msg(char const *msg);
void reset_err_prefix(void);
void reset_err_object(void);
void reset_err_msg(void);
t_result set_error(t_error err, char const *err_msg);
void print_error_advanced(int fd, char const *prefix, t_error err, char const *object, char const *msg);
void print_error_local(int fd, char const *prefix, t_error err, char const *object, char const *msg);
void print_error(int fd);
