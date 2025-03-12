#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "md5.h"
#include "sha256.h"
#include "utils.h"
#include "whirlpool.h"

#define DIGEST_BLOCK_SIZE 512
#define DIGEST_BLOCK_BYTES (DIGEST_BLOCK_SIZE / 8)

struct digest_args {
	char **files;
	size_t file_num;
	char *string;
	bool print;
	bool quiet;
	bool reverse;
};

enum e_digest {
	D_MD5,
	D_SHA256,
	D_WHIRLPOOL,
};

typedef union {
	struct md5_state md5_state;
	struct sha256_state sha256_state;
	struct whirlpool_state whirlpool_state;
} t_digest_state;

typedef union {
	struct hash128 md5;
	struct hash256 sha256;
	struct hash512 whirlpool;
} t_digest_hash;

static t_digest_state digest_state(enum e_digest digest) {
	t_digest_state state;

	switch (digest) {
		case D_MD5:
			state.md5_state = md5_state();
			return state;
		case D_SHA256:
			state.sha256_state = sha256_state();
			return state;
		case D_WHIRLPOOL:
			state.whirlpool_state = whirlpool_state();
			return state;
	}
}

static t_result parse_digest_args(char **args, struct digest_args *opts) {
	*opts = (struct digest_args){
		.files = NULL,
		.file_num = 0,
		.print = false,
		.quiet = false,
		.reverse = false,
		.string = NULL,
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
			if (opts->string != NULL) {
				set_err_object(arg);
				return set_error(E_DUPLICATE_OPT, "Duplicate option");
			}
			if (args[index] == NULL) {
				set_err_object(arg);
				return set_error(E_OPT_MISSING_VALUE, "Option expected value, but it is missing");
			}
			opts->string = args[index];
		}
		else {
			set_err_object(arg);
			return set_error(E_UNEXPECTED_OPT, "Unexpected option");
		}
		index++;
	}
	while (args[index] != NULL) {
		while (args[index + opts->file_num] != NULL) {
			opts->file_num++;
		}
		opts->files = &args[index];
		index += opts->file_num;
	}

	return OK;
}

struct digest_file_stream {
	int fd;
	uint8_t buffer[DIGEST_BLOCK_BYTES];
};

static struct digest_file_stream file_stream(int fd) {
	struct digest_file_stream stream = {
		.fd = fd,
	};
	return stream;
}

static ssize_t read_file_block(struct digest_file_stream *stream) {
	int fd = stream->fd;
	uint8_t *buffer = stream->buffer;

	size_t buffer_filled = 0;
	ssize_t nread = read(fd, buffer, sizeof(stream->buffer));
	while (nread > 0) {
		buffer_filled += nread;
		nread = read(fd, buffer, sizeof(stream->buffer) - buffer_filled);
	};
	return buffer_filled;
}

static t_digest_state digest_round(enum e_digest digest, t_digest_state state, const uint8_t m[DIGEST_BLOCK_BYTES]) {
	switch (digest) {
		case D_MD5:
			state.md5_state = md5_round(state.md5_state, m);
			return state;
		case D_SHA256:
			state.sha256_state = sha256_round(state.sha256_state, m);
			return state;
		case D_WHIRLPOOL:
			state.whirlpool_state = whirlpool_round(state.whirlpool_state, m);
			return state;
	}
}

static t_digest_hash digest_final_round(enum e_digest digest, t_digest_state state, const uint8_t m[DIGEST_BLOCK_BYTES], uint16_t bits) {
	t_digest_hash hash;
	switch (digest) {
		case D_MD5:
			hash.md5 = md5_final_round(state.md5_state, m, bits);
			return hash;
		case D_SHA256:
			hash.sha256 = sha256_final_round(state.sha256_state, m, bits);
			return hash;
		case D_WHIRLPOOL:
			hash.whirlpool = whirlpool_final_round(state.whirlpool_state, m, bits);
			return hash;
	}
}

static void print_hash(int fd, enum e_digest digest, t_digest_hash *hash) {
	switch (digest) {
		case D_MD5: {
			struct hash128_hex hex = hash128_hex(&hash->md5);
			ft_putstr(fd, hex.hex);
			return;
		}
		case D_SHA256: {
			struct hash256_hex hex = hash256_hex(&hash->sha256);
			ft_putstr(fd, hex.hex);
			return;
		}
		case D_WHIRLPOOL: {
			struct hash512_hex hex = hash512_hex(&hash->whirlpool);
			ft_putstr(fd, hex.hex);
			return;
		}
	}
}

static char const *digest_name(enum e_digest digest) {
	static char const *const names[] = {
		[D_MD5] = "MD5",
		[D_SHA256] = "SHA256",
		[D_WHIRLPOOL] = "WHIRLPOOL",
	};

	return names[digest];
}

static void print_digest_buf(enum e_digest digest, uint8_t *buf, size_t size, struct digest_args *const opts) {
	t_digest_state state = digest_state(digest);

	if (!opts->quiet && !opts->reverse) {
		ft_putstrs(STDOUT_FILENO, (char const*[]){digest_name(digest), "(\"", NULL});
		print_escaped(STDOUT_FILENO, buf, size);
		ft_putstr(STDOUT_FILENO, "\")= ");
	}

	size_t buf_index = 0;
	while (size - buf_index > DIGEST_BLOCK_BYTES) {
		state = digest_round(digest, state, buf + buf_index);
	}
	t_digest_hash hash = digest_final_round(digest, state, buf + buf_index, (size - buf_index) * 8);
	print_hash(STDOUT_FILENO, digest, &hash);

	if (!opts->quiet && opts->reverse) {
		ft_putstr(STDOUT_FILENO, " \"");
		print_escaped(STDOUT_FILENO, buf, size);
		ft_putstr(STDOUT_FILENO, "\"");
	}
	ft_putstr(STDOUT_FILENO, "\n");
}

static t_result print_digest_file(enum e_digest digest, int fd, char *filename, struct digest_args *const opts) {
	t_digest_state state = digest_state(digest);
	struct digest_file_stream stream = file_stream(fd);

	if (!opts->quiet && !opts->reverse) {
		ft_putstrs(STDOUT_FILENO, (char const*[]){digest_name(digest), "(", filename, ")= ", NULL});
	}

	while (true) {
		ssize_t nread = read_file_block(&stream);
		if (nread < 0) {
			return set_error(E_ERRNO, "");
		}

		if (nread == DIGEST_BLOCK_BYTES) {
			state = digest_round(digest, state, stream.buffer);
		}
		else {
			t_digest_hash hash = digest_final_round(digest, state, stream.buffer, nread * 8);
			print_hash(STDOUT_FILENO, digest, &hash);
			break;
		}
	}

	if (!opts->quiet && opts->reverse) {
		ft_putstrs(STDOUT_FILENO, (char const*[]){" ", filename, NULL});
	}
	ft_putstr(STDOUT_FILENO, "\n");
	return OK;
}

static t_result print_digest_stdin(enum e_digest digest, struct digest_args *const opts) {
	if (!opts->print) {
		return print_digest_file(digest, STDIN_FILENO, "<stdin>", opts);
	}

	t_digest_state state = digest_state(digest);
	struct digest_file_stream stream = file_stream(STDIN_FILENO);

	if (!opts->quiet) {
		ft_putstrs(STDOUT_FILENO, (char const*[]){digest_name(digest), "(", NULL});
	}
	ft_putstr(STDOUT_FILENO, "\"");

	while (true) {
		ssize_t nread = read_file_block(&stream);
		if (nread < 0) {
			return set_error(E_ERRNO, "");
		}

		print_escaped(STDOUT_FILENO, stream.buffer, nread);

		if (nread == DIGEST_BLOCK_BYTES) {
			state = digest_round(digest, state, stream.buffer);
		}
		else {
			ft_putstr(STDOUT_FILENO, "\"");
			if (!opts->quiet) {
				ft_putstr(STDOUT_FILENO, ")= ");
			}
			else {
				ft_putstr(STDOUT_FILENO, "\n");
			}
			t_digest_hash hash = digest_final_round(digest, state, stream.buffer, nread * 8);
			print_hash(STDOUT_FILENO, digest, &hash);
			break;
		}
	}
	ft_putstr(STDOUT_FILENO, "\n");
	return OK;
}

static t_result exec_digest(enum e_digest digest, struct digest_args *const opts) {
	if ((opts->file_num == 0 && !opts->string) || opts->print) {
		set_err_object("<stdin>");
		if (print_digest_stdin(digest, opts) != OK) {
			return propagate_error();
		}
		reset_err_object();
	}

	if (opts->string != NULL) {
		print_digest_buf(digest, (uint8_t*)opts->string, ft_strlen(opts->string), opts);
	}

	for (size_t i = 0; i < opts->file_num; i++) {
		set_err_object(opts->files[i]);
		int fd = open(opts->files[i], O_RDONLY);
		if (fd < 0) {
			print_error_local(STDERR_FILENO, NULL, E_ERRNO, NULL, NULL);
			continue;
		}
		if (print_digest_file(digest, fd, opts->files[i], opts) != OK) {
			close(fd);
			return propagate_error();
		}
		close(fd);
	}
	reset_err_object();
	return OK;
}

t_result md5_digest(char **args) {
	set_err_prefix("md5");
	struct digest_args opts;
	if (
		parse_digest_args(args, &opts) != OK ||
		exec_digest(D_MD5, &opts) != OK
	) {
		print_error(STDERR_FILENO);
		exit(1);
	}
	reset_err_prefix();
	return reset_error();
}

t_result sha256_digest(char **args) {
	set_err_prefix("sha256");
	struct digest_args opts;
	if (
		parse_digest_args(args, &opts) != OK ||
		exec_digest(D_SHA256, &opts) != OK
	) {
		print_error(STDERR_FILENO);
		exit(1);
	}
	reset_err_prefix();
	return reset_error();
}

t_result whirlpool_digest(char **args) {
	set_err_prefix("whirlpool");
	struct digest_args opts;
	if (
		parse_digest_args(args, &opts) != OK ||
		exec_digest(D_WHIRLPOOL, &opts) != OK
	) {
		print_error(STDERR_FILENO);
		exit(1);
	}
	reset_err_prefix();
	return reset_error();
}
