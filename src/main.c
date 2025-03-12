#include <stdlib.h>
#include <unistd.h>

#include "digest/digest.h"
#include "utils.h"

typedef t_result (t_command_fn)(char **args);

typedef struct {
	char const *cmd;
	t_command_fn *fn;
} t_command;

static void print_help(void) {
	ft_putstr(STDERR_FILENO,
		"Commands:\n"
		"md5\n"
		"sha256\n"
		"\n"
		"Flags:\n"
		"-p -q -r -s\n"
	);
}

int main(int argc, char **argv) {
	static t_command const commands[] = {
		{ "md5", &md5_digest},
		{ "sha256", &sha256_digest },
	};

	if (argc < 2) {
		print_help();
		return EXIT_FAILURE;
	}

	char const *cmd = argv[1];
	t_command_fn *command_fn = NULL;
	for (size_t i = 0; i < sizeof(commands) / sizeof(*commands); i++) {
		if (ft_streq(cmd, commands[i].cmd)) {
			command_fn = commands[i].fn;
			break;
		}
	}
	if (command_fn == NULL) {
		ft_putstrs(STDERR_FILENO, (char const *[]){PROGRAM ": Error: '", cmd, "' is an invalid command\n\n", NULL});
		print_help();
		return EXIT_FAILURE;
	}
	if (command_fn(&argv[2]) != OK) {
		print_error(STDERR_FILENO);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
