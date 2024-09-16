#include <errno.h>
#include <stddef.h>
#include <string.h>

#include "error.h"
#include "utils.h"

struct error {
	enum ft_error errnum;
	char msg[MAX_ERR_MSG_SIZE];
};

static struct error *get_error_ptr(void) {
	static struct error error = {
		.errnum = E_NONE,
		.msg = {'\0'},
	};
	return &error;
}

t_result set_error(enum ft_error errnum, char const *msg) {
	struct error *err = get_error_ptr();
	err->errnum = errnum;
	if (msg != NULL) {
		size_t len = ft_strlen(msg);
		if (len >= MAX_ERR_MSG_SIZE) {
			len = MAX_ERR_MSG_SIZE;
		}
		ft_memcpy(err->msg, msg, len);
		err->msg[len] = '\0';
	}
	else {
		err->msg[0] = '\0';
	}
	if (errnum == E_NONE) {
		return OK;
	}
	return FAIL;
}

void clear_error(void) {
	struct error *err = get_error_ptr();
	err->errnum = E_NONE;
	err->msg[0] = '\0';
}

void append_error_msg(char const *msg_append) {
	struct error *err = get_error_ptr();

	size_t len = ft_strlen(msg_append);
	size_t cur_len = ft_strlen(err->msg);
	if (len >= MAX_ERR_MSG_SIZE - cur_len) {
		len = MAX_ERR_MSG_SIZE - cur_len;
	}
	ft_memcpy(err->msg, msg_append, len);
	err->msg[len] = '\0';
}

enum ft_error get_error(void) {
	return get_error_ptr()->errnum;
}

char const *get_error_msg(void) {
	return get_error_ptr()->msg;
}

void print_error(int fd, char const *prefix) {
	enum ft_error err = get_error();
	if (err == E_NONE) {
		return;
	}

	char const *msg = get_error_msg();
	if (prefix != NULL) {
		ft_putstr(fd, prefix);
		ft_putstr(fd, ": ");
	}

	if (err == E_ERRNO) {
		if (msg != NULL && msg[0] != '\0') {
			ft_putstr(fd, msg);
			ft_putstr(fd, ": ");
		}
		char *s = strerror(errno);
		ft_putstr(fd, s);
	}
	else if (msg != NULL && msg[0] != '\0') {
		ft_putstr(fd, msg);
	}
	else {
		ft_putstr(fd, "(");
		ft_putstr(fd, "no message");
		ft_putstr(fd, ")");
	}
	ft_putstr(fd, "\n");
}

