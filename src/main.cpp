#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "version.hpp"
#include "main.hpp"
#include "shadlp/config.hpp"
#include "shadlp/server.hpp"

static int running = 0;
static char *command;

enum {
	OPT_VERSION = 1,
};

static void usage(char *command)
{
	printf("Usage: %s [OPTION]... \n"
	       "\n"
	       "-h, --help              help\n"
	       "    --version           print current version\n"
	       "-c, --config=FILE       use this config file\n"
	       "-p, --port=#            server port\n",
	       command);
}

static void version(void)
{
	printf("%s: version " SHADLP_VERSION_STR
	       " by Neko Life <nekolife123579@gmail.com>\n",
	       command);
}

int main(int argc, char *argv[])
{
	int option_index;
	static const char short_options[] = "hc:p:";
	static const struct option long_options[] = { { "help", 0, 0, 'h' },
						      { "version", 0, 0,
							OPT_VERSION },
						      { "config", 1, 0, 'c' },
						      { "port", 1, 0, 'p' },
						      { 0, 0, 0, 0 } };

	int tmp, err, c;

	command = argv[0];
	if (strstr(argv[0], "shadlp")) {
		command = "shadlp";
	} else {
		error("command should be named shadlp");
		return 1;
	}

	config::load_config("shadlp_config.json");

	while ((c = getopt_long(argc, argv, short_options, long_options,
				&option_index)) != -1) {
		switch (c) {
		case 'h':
			usage(command);
			return 0;
		case OPT_VERSION:
			version();
			return 0;
		case 'c':
			err = config::load_config(optarg);
			if (err)
				return err;
			break;
		case 'p': {
			err = config::set_port(atoi(optarg));
			if (err)
				return err;
			break;
		}
		default:
			fprintf(stderr,
				"Try `%s --help' for more information.\n",
				command);
			return 1;
		}
	}

	running = 1;

	err = server::run_server();

	running = 0;

	return err;
}

// vim: ts=8 sw=8 noet
