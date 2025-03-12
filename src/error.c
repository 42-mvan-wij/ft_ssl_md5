#include <assert.h>
#include <errno.h>
#include <string.h>

#include "error.h"
#include "utils.h" // NOTE: Needed for PROGRAM define

struct error_data {
	t_error err;
	char prefix[MAX_ERR_PREFIX_LEN + 1];
	char msg[MAX_ERR_MSG_LEN + 1];
	char object[MAX_ERR_OBJECT_LEN + 1];
};

static struct error_data *get_error_data_ptr(void) {
	static struct error_data error_data = {0};
	return &error_data;
}

static void set_string(char *dst, char const *src, size_t dst_max) {
	size_t len = ft_strlen_max(src, dst_max);
	ft_memcpy(dst, src, len);
	dst[len] = '\0';
}

t_result reset_error(void) {
	struct error_data *error_data = get_error_data_ptr();
	error_data->err = E_NONE;
	error_data->msg[0] = '\0';
	return OK;
}

t_result propagate_error(void) {
	assert(get_error_data_ptr()->err != E_NONE);
	return FAIL;
}

void set_err_prefix(char const *prefix) {
	struct error_data *error_data = get_error_data_ptr();
	set_string(error_data->prefix, prefix, MAX_ERR_PREFIX_LEN);
}

void set_err_object(char const *object) {
	struct error_data *error_data = get_error_data_ptr();
	set_string(error_data->object, object, MAX_ERR_OBJECT_LEN);
}

void set_err_msg(char const *msg) {
	struct error_data *error_data = get_error_data_ptr();
	set_string(error_data->msg, msg, MAX_ERR_MSG_LEN);
}

void reset_err_prefix(void) {
	struct error_data *error_data = get_error_data_ptr();
	error_data->prefix[0] = '\0';
}

void reset_err_object(void) {
	struct error_data *error_data = get_error_data_ptr();
	error_data->object[0] = '\0';
}

void reset_err_msg(void) {
	struct error_data *error_data = get_error_data_ptr();
	error_data->msg[0] = '\0';
}

t_result set_error(t_error err, char const *err_msg) {
	struct error_data *error_data = get_error_data_ptr();
	error_data->err = err;
	if (err_msg != NULL) {
		set_err_msg(err_msg);
	}
	if (err == E_NONE) {
		return OK;
	}
	return FAIL;
}

void print_error_advanced(int fd, char const *prefix, t_error err, char const *object, char const *msg) {
	if (err == E_NONE) {
		return;
	}

#ifdef PROGRAM
	if (PROGRAM != NULL && (PROGRAM)[0] != '\0') {
		ft_putstrs(fd, (char const *[]){PROGRAM, ": ", NULL});
	}
#endif

	ft_putstr(fd, "Error: ");

	if (prefix != NULL && prefix[0] != '\0') {
		ft_putstrs(fd, (char const *[]){prefix, ": ", NULL});
	}

	if (object != NULL && object[0] != '\0') {
		ft_putstrs(fd, (char const *[]){object, ": ", NULL});
	}

	if (msg != NULL && msg[0] != '\0') {
		ft_putstr(fd, msg);
	}
	else if (err == E_ERRNO) {
		ft_putstr(fd, strerror(errno));
	}
	else {
		ft_putstr(fd, "(no message)");
	}
	ft_putstr(fd, "\n");
}

void print_error_local(int fd, char const *prefix, t_error err, char const *object, char const *msg) {
	struct error_data *error_data = get_error_data_ptr();

	char const *use_prefix = prefix != NULL   ? prefix : error_data->prefix;
	t_error     use_err    = err    != E_NONE ? err    : error_data->err;
	char const *use_object = object != NULL   ? object : error_data->object;
	char const *use_msg    = msg    != NULL   ? msg    : error_data->msg;

	return print_error_advanced(fd, use_prefix, use_err, use_object, use_msg);
}

void print_error(int fd) {
	struct error_data *error_data = get_error_data_ptr();
	return print_error_local(fd, error_data->prefix, error_data->err, error_data->object, error_data->msg);
}
