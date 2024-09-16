#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "./digest.h"
#include "./utils.h"

#define PROG_NAME "ft_ssl"


// #include <stdio.h>
// void print_buf(uint8_t *buf_bytes, size_t size) {
// 	for (size_t ii = 0; ii < size; ii++) {
// 		for (size_t jj = 0; jj < 8; jj++) {
// 			printf("%c", (buf_bytes[ii] >> (8 - jj - 1)) & 1 ? '1' : '0');
// 		}
// 		printf(" ");
// 		if (ii % 8 == 7) {
// 			printf("\n");
// 		}
// 		// printf("%.2x ", buf_bytes[ii]);
// 	}
// 	printf("\n");
//
// }



// #include <stdio.h>
// #include <limits.h>
// #include <ctype.h>
// #include "sha256.h"
// #include "hash.h"
// void cmd_test(char **args) {
// 	(void)args;
// 	printf("%c => %#.2x\n", 'a', 'a');
// 	printf("%c => %#.2x\n", 'b', 'b');
// 	printf("%c => %#.2x\n", 'c', 'c');
// 	struct hash256 hash256 = sha256_buf("", 0);
// 	write_hash256(STDOUT_FILENO, hash256);
// 	dprintf(STDOUT_FILENO, "\n");
// 	// for (int i = CHAR_MIN; i <= CHAR_MAX; i++) {
// 	// 	printf("%.3i = 0x%.2x = 0b", i, i);
// 	// 	for (int b = 8; b > 0; b--) {
// 	// 		printf("%c", ((i >> b) & 1) ? '1' : '0');
// 	// 	}
// 	// 	char s[] = {i, '\0'};
// 	// 	printf(" = '");
// 	// 	fflush(stdout);
// 	// 	print_escaped(STDOUT_FILENO, s, 1);
// 	// 	printf("' ---> %s\n", isprint(i) ? "true" : "false");
// 	// }
// }

static t_result run_cmd(char *cmd, char **args) {
	static struct {
		char *cmd_name;
		t_result(*cmd_fn)(char **args);
	} dispatch_table[] = {
		{ "md5", &cmd_md5 },
		{ "sha256", &cmd_sha256 },
		// { "test", &cmd_test },
	};

	for (size_t i = 0; i < sizeof(dispatch_table) / sizeof(*dispatch_table); i++) {
		if (ft_streq(cmd, dispatch_table[i].cmd_name)) {
			if (dispatch_table[i].cmd_fn(args) != OK) {
				return FAIL;
			}
			return OK;
		}
	}
	(void)set_error(E_INVALID_CMD, "'");
	append_error_msg(cmd);
	append_error_msg("' is an invalid command");
	return FAIL;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		ft_putstr(STDERR_FILENO, "usage: " PROG_NAME " command [flag] [file/string]\n");
		return EXIT_FAILURE;
	}
	if (run_cmd(argv[1], &argv[2]) != OK) {
		print_error(STDERR_FILENO, ERR_PREFIX);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
