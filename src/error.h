#pragma once

#define MAX_ERR_MSG_SIZE 512

typedef enum result {
	OK = 0,
	FAIL,
} __attribute__((warn_unused_result)) t_result;

enum ft_error {
	E_NONE,
	E_ERRNO,
	E_INVALID_CMD,
	E_OPT_MISSING_VALUE,
	E_UNEXPECTED_OPT,
};

t_result set_error(enum ft_error errnum, char const *msg);
void clear_error(void);
void append_error_msg(char const *msg_append);
enum ft_error get_error(void);
char const *get_error_msg(void);
void print_error(int fd, char const *prefix);

