#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "error.h"
#include "md5.h"
#include "sha256.h"
#include "utils.h"

struct digest_args {
	char **files;
	size_t file_num;
	char *string;
	bool print;
	bool quiet;
	bool reverse;
};

static t_result parse_digest_args(char **args, struct digest_args *opts) {
	*opts = (struct digest_args){
		.files = NULL,
		.file_num = 0,
		.print = false,
		.quiet = false,
		.reverse = false,
		.string = false,
	};

	size_t index = 0;
	while (args[index] != NULL && args[index][0] == '-') {
		char *arg = args[index];
		if (ft_streq(&arg[1], "p")) {
			opts->print = true;
		}
		else if (ft_streq(&arg[1], "q")) {
			opts->quiet = true;
		}
		else if (ft_streq(&arg[1], "r")) {
			opts->reverse = true;
		}
		else if (ft_streq(&arg[1], "s")) {
			index++;
			if (args[index] == NULL) {
				return set_error(E_OPT_MISSING_VALUE, "Option expected value, but it is missing");
			}
			opts->string = args[index];
		}
		else {
			return set_error(E_UNEXPECTED_OPT, "Unexpected option");
		}
		index++;
	}
	while (args[index] != NULL) {
		while (args[index + opts->file_num] != NULL) {
			opts->file_num++;
		}
		opts->files = malloc(sizeof(char *) * opts->file_num);
		if (opts->files == NULL) {
			return set_error(E_ERRNO, NULL);
		}
		size_t i = 0;
		while (args[index] != NULL) {
			opts->files[i] = args[index];
			index++;
			i++;
		}
	}
	return OK;
}

static void print_md5_buf(char *buf, size_t size, struct digest_args *const opts) {
	struct hash128 hash128 = md5_buf(buf, size);
	if (!opts->reverse && !opts->quiet) {
		ft_putstr(STDOUT_FILENO, "MD5(\"");
		print_escaped(STDOUT_FILENO, buf, size);
		ft_putstr(STDOUT_FILENO, "\")= ");
	}
	write_hash128(STDOUT_FILENO, hash128);
	if (opts->reverse && !opts->quiet) {
		ft_putstr(STDOUT_FILENO, " \"");
		print_escaped(STDOUT_FILENO, buf, size);
		ft_putstr(STDOUT_FILENO, "\"");
	}
	ft_putstr(STDOUT_FILENO, "\n");
}

static void print_md5_fd(int fd, char *filename, struct digest_args *const opts) {
	struct hash128 hash128 = md5_fd(fd);
	if (!opts->reverse && !opts->quiet) {
		ft_putstr(STDOUT_FILENO, "MD5(");
		ft_putstr(STDOUT_FILENO, filename);
		ft_putstr(STDOUT_FILENO, ")= ");
	}
	write_hash128(STDOUT_FILENO, hash128);
	if (opts->reverse && !opts->quiet) {
		ft_putstr(STDOUT_FILENO, " *");
		ft_putstr(STDOUT_FILENO, filename);
	}
	ft_putstr(STDOUT_FILENO, "\n");
}

static t_result exec_md5(struct digest_args *const opts) {
	if ((opts->file_num == 0 && !opts->string) || opts->print) {
		if (opts->print) {
			size_t len;
			char *input = read_to_string(STDIN_FILENO, &len);
			if (input == NULL) {
				assert(get_error() != E_NONE);
				return FAIL;
			}
			if (opts->quiet) {
				ft_putstr(STDOUT_FILENO, "\"");
				print_escaped(STDOUT_FILENO, input, len);
				ft_putstr(STDOUT_FILENO, "\"\n");
			}
			print_md5_buf(input, len, opts);
			free(input);
		}
		else {
			print_md5_fd(STDIN_FILENO, "stdin", opts);
		}
	}

	if (opts->string != NULL) {
		print_md5_buf(opts->string, ft_strlen(opts->string), opts);
	}

	for (size_t i = 0; i < opts->file_num; i++) {
		int fd = open(opts->files[i], O_RDONLY);
		if (fd < 0) {
			(void)set_error(E_ERRNO, opts->files[i]);
			print_error(STDERR_FILENO, PROG_NAME ": md5");
			clear_error();
			continue;
		}
		print_md5_fd(fd, opts->files[i], opts);
		close(fd);
	}
	return OK;
}

static void print_sha256_buf(char *buf, size_t size, struct digest_args *const opts) {
	struct hash256 hash256 = sha256_buf(buf, size);
	if (!opts->reverse && !opts->quiet) {
		ft_putstr(STDOUT_FILENO, "SHA256(\"");
		print_escaped(STDOUT_FILENO, buf, size);
		ft_putstr(STDOUT_FILENO, "\")= ");
	}
	write_hash256(STDOUT_FILENO, hash256);
	if (opts->reverse && !opts->quiet) {
		ft_putstr(STDOUT_FILENO, " \"");
		print_escaped(STDOUT_FILENO, buf, size);
		ft_putstr(STDOUT_FILENO, "\"");
	}
	ft_putstr(STDOUT_FILENO, "\n");
}

static void print_sha256_fd(int fd, char *filename, struct digest_args *const opts) {
	struct hash256 hash256 = sha256_fd(fd);
	if (!opts->reverse && !opts->quiet) {
		ft_putstr(STDOUT_FILENO, "SHA256(");
		ft_putstr(STDOUT_FILENO, filename);
		ft_putstr(STDOUT_FILENO, ")= ");
	}
	write_hash256(STDOUT_FILENO, hash256);
	if (opts->reverse && !opts->quiet) {
		ft_putstr(STDOUT_FILENO, " *");
		ft_putstr(STDOUT_FILENO, filename);
	}
	ft_putstr(STDOUT_FILENO, "\n");
}

static t_result exec_sha256(struct digest_args *const opts) {
	if ((opts->file_num == 0 && !opts->string) || opts->print) {
		if (opts->print) {
			size_t len;
			char *input = read_to_string(STDIN_FILENO, &len);
			if (input == NULL) {
				assert(get_error() != E_NONE);
				return FAIL;
			}
			if (opts->quiet) {
				ft_putstr(STDOUT_FILENO, "\"");
				print_escaped(STDOUT_FILENO, input, len);
				ft_putstr(STDOUT_FILENO, "\"\n");
			}
			print_sha256_buf(input, len, opts);
			free(input);
		}
		else {
			print_sha256_fd(STDIN_FILENO, "stdin", opts);
		}
	}

	if (opts->string != NULL) {
		print_sha256_buf(opts->string, ft_strlen(opts->string), opts);
	}

	for (size_t i = 0; i < opts->file_num; i++) {
		int fd = open(opts->files[i], O_RDONLY);
		if (fd < 0) {
			(void)set_error(E_ERRNO, opts->files[i]);
			print_error(STDERR_FILENO, PROG_NAME ": sha256");
			clear_error();
			continue;
		}
		print_sha256_fd(fd, opts->files[i], opts);
		close(fd);
	}
	return OK;
}

t_result cmd_md5(char **args) {
	struct digest_args opts = {.files = NULL};
	if (parse_digest_args(args, &opts) != OK
		|| exec_md5(&opts) != OK)
	{
		free(opts.files);
		return FAIL;
	}
	free(opts.files);
	return OK;
}

t_result cmd_sha256(char **args) {
	struct digest_args opts = {.files = NULL};
	if (parse_digest_args(args, &opts) != OK
		|| exec_sha256(&opts) != OK)
	{
		free(opts.files);
		return FAIL;
	}
	free(opts.files);
	return OK;
}

