/*
Copyright (c) 2014-2024 Jorge Matricali

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> /* clock */

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <execinfo.h> /* backtrace, backtrace_symbols_fd */
#include <signal.h>
#include <sys/resource.h> /* rlimit */
#include <sys/wait.h> /* waitpid */
#include <unistd.h> /* STDERR_FILENO */
#endif

#include <pthread.h>

#include "bruteforce_ssh.h"
#include "cbrutekrag.h"
#include "credentials.h"
#include "detection.h"
#include "log.h"
#include "progress.h"
#include "str.h"
#include "target.h"

#define OPTIONAL_ARGUMENT_IS_PRESENT                                           \
	((optarg == NULL && optind < argc && argv[optind][0] != '-') ?         \
		 (bool)(optarg = argv[optind++]) :                             \
		 (optarg != NULL))

/* Long options for getopt_long */
static struct option long_options[] = {
	{ "help", no_argument, NULL, 'h' },
	{ "config", required_argument, NULL, 'c' },
	{ "verbose", no_argument, NULL, 'v' },
	{ "verbose-sshlib", no_argument, NULL, 'V' },
	{ "scan", no_argument, NULL, 's' },
	{ "dry-run", no_argument, NULL, 'D' },
	{ "progress", no_argument, NULL, 'P' },
	{ "targets", required_argument, NULL, 'T' },
	{ "credentials", required_argument, NULL, 'C' },
	{ "threads", required_argument, NULL, 't' },
	{ "output", required_argument, NULL, 'o' },
	{ "format", required_argument, NULL, 'f' },
	{ "scan-output", required_argument, NULL, 'O' },
	{ "scan-format", required_argument, NULL, 'F' },
	{ "allow-non-openssh", no_argument, NULL, 'a' },
	{ "allow-honeypots", no_argument, NULL, 'A' },
	{ "timeout", required_argument, NULL, 11 },
	{ "check-http", optional_argument, NULL, 13 },
	{ NULL, 0, NULL, 0 }
};

static void print_banner(void)
{
	printf("\033[92m           _                _       _\n"
	       "          | |              | |     | |\n"
	       "\033[37m      ___\033[92m | |__  _ __ _   _| |_ ___| | ___ __ __ _  __ _\n"
	       "\033[37m     / __|\033[92m| '_ \\| '__| | | | __/ _ \\ |/ / '__/ _` |/ _` |\n"
	       "\033[37m    | (__ \033[92m| |_) | |  | |_| | ||  __/   <| | | (_| | (_| |\n"
	       "\033[37m     \\___|\033[92m|_.__/|_|   \\__,_|\\__\\___|_|\\_\\_|  \\__,_|\\__, |\n"
	       "              \033[0m\033[1mOpenSSH Brute force tool 0.6.0\033[0m\033[92m        __/ |\n"
	       "          \033[0m(c) Copyright 2014-2024 Jorge Matricali\033[92m  |___/\033[0m\n\n"
	       "          \033[36mhttps://github.com/matricali/cbrutekrag\n"
	       "\033[0m\n");
}

static void usage(const char *p)
{
	printf("\nusage: %s [-h] [-v] [-aA] [-D] [-P] [-T TARGETS.lst] [-C credentials.lst]\n"
	       "\t\t[-t THREADS] [-f OUTPUT FORMAT] [-o OUTPUT.txt] [-F SCAN OUTPUT FORMAT] [-O SCAN_OUTPUT.txt] [TARGETS...]\n\n",
	       p);
}

void err_handler(int sig)
{
	log_error("Error: signal %d:\n", sig);

#ifndef _WIN32
	void *array[10];
	int size;

	size = backtrace(array, 10);

	backtrace_symbols_fd(array, size, STDERR_FILENO);
#endif
	exit(EXIT_FAILURE);
}

static size_t btkg_get_max_threads(void)
{
#ifndef _WIN32
	struct rlimit limit;

	/* Increase the maximum file descriptor number that can be opened by this process. */
	getrlimit(RLIMIT_NOFILE, &limit);
	limit.rlim_cur = limit.rlim_max;
	setrlimit(RLIMIT_NOFILE, &limit);

	return (limit.rlim_cur > 1024) ? 1024 : limit.rlim_cur - 8;
#else
	// TODO: Get real max threads available under Windows
	return 1024;
#endif
}

#ifdef _WIN32
/**
 * @brief Sets up the console to support ANSI escape sequences on Windows.
 *
 * This function enables the ENABLE_VIRTUAL_TERMINAL_PROCESSING mode in the
 * Windows console, allowing it to process ANSI escape sequences for colored
 * output. If the console handle is invalid or if there is an error while
 * getting or setting the console mode, an error message is printed to stderr.
 */
static void btkg_console_setup(void)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole != INVALID_HANDLE_VALUE) {
		DWORD dwMode = 0;

		if (!GetConsoleMode(hConsole, &dwMode)) {
			fprintf(stderr, "Error getting console mode: %lu\n",
				GetLastError());
			return;
		}

		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

		if (!SetConsoleMode(hConsole, dwMode)) {
			fprintf(stderr, "Error setting console mode: %lu\n",
				GetLastError());
		}
	}
}
#endif

int main(int argc, char **argv)
{
	int opt;
	int option_index = 0;
	char *output_filename = NULL;
	char *scan_output_filename = NULL;
	int tempint;
	pthread_t progress_watcher;

	/* Error handler */
	signal(SIGSEGV, err_handler);

#ifndef _WIN32
	/* Ignore SIGPIPE */
	signal(SIGPIPE, SIG_IGN);
#endif

#ifdef _WIN32
	/* Setup console */
	btkg_console_setup();
#endif

	/* Initialize context */
	btkg_context_t context;
	btkg_context_init(&context);

	/* Calculate maximum number of threads. */
	btkg_options_t *options = &context.options;
	options->max_threads = btkg_get_max_threads();

	btkg_target_list_t *targets = &context.targets;
	btkg_credentials_list_t *credentials = &context.credentials;

	while ((opt = getopt_long(argc, argv, "aAT:C:t:o:f:O:F:DsvVPh",
				  long_options, &option_index)) != -1) {
		switch (opt) {
			case 'a':
				options->non_openssh = 1;
				break;
			case 'A':
				options->allow_honeypots = 1;
				break;
			case 'v':
				options->verbose |= CBRUTEKRAG_VERBOSE_MODE;
				log_set_level(options->verbose);
				break;
			case 'V':
				options->verbose |= CBRUTEKRAG_VERBOSE_SSHLIB;
				break;
			case 'T':
				btkg_target_list_load(targets, optarg);
				break;
			case 'C':
				btkg_credentials_list_load(credentials, optarg);
				break;
			case 't':
				tempint = atoi(optarg);
				if (tempint < 1) {
					log_error("Invalid threads size. (%d)",
						  tempint);
					exit(EXIT_FAILURE);
				}
				options->max_threads = (size_t)tempint;
				break;
			case 'f':
				options->bruteforce_output_format = strdup(optarg);
				btkg_str_replace_escape_sequences(
					options->bruteforce_output_format);
				break;
			case 'o':
				output_filename = strdup(optarg);
				break;
			case 's':
				options->perform_scan = 1;
				break;
			case 'O':
				scan_output_filename = strdup(optarg);
				break;
			case 'F':
				options->scanner_output_format = strdup(optarg);
				btkg_str_replace_escape_sequences(
					options->scanner_output_format);
				break;
			case 'D':
				options->dry_run = 1;
				break;
			case 'P':
				options->progress_bar = 1;
				break;
			case 11: // --timeout
				tempint = atoi(optarg);
				if (tempint < 1) {
					log_error("Invalid timeout value. (%d)",
						  tempint);
					exit(EXIT_FAILURE);
				}
				options->timeout = tempint;
				break;
			case 13:
				options->check_http =
					strdup((!OPTIONAL_ARGUMENT_IS_PRESENT) ?
						       "wwww.google.com" :
						       optarg);
				break;
			case 'h':
				print_banner();
				usage(argv[0]);
				printf("  -h, --help                This help\n"
				       "  -v, --verbose             Verbose mode\n"
				       "  -V, --verbose-sshlib      Verbose mode (sshlib)\n"
				       "  -s, --scan                Scan mode\n"
				       "  -D, --dry-run             Dry run\n"
				       "  -P, --progress            Progress bar\n"
				       "  -T, --targets <file>      Targets file\n"
				       "  -C, --credentials <file>  Username and password file\n"
				       "  -t, --threads <threads>   Max threads\n"
				       "  -o, --output <file>       Output log file\n"
				       "  -F, --format <pattern>    Output log format\n"
				       "                            Available placeholders:\n"
				       "                            %%DATETIME%%, %%HOSTNAME%%\n"
				       "                            %%PORT%%, %%USERNAME%%, %%PASSWORD%%\n"
				       "  -O, --scan-output <file>  Output log file for scanner\n"
				       "  -F, --scan-format <pattern> Output log format for scanner\n"
				       "                            Available placeholders:\n"
				       "                            %%DATETIME%%, %%HOSTNAME%%\n"
				       "                            %%PORT%%, %%BANNER%%.\n"
				       "                            Default:\n"
				       "                            \"%%HOSTNAME%%:%%PORT%%\\t%%BANNER%%\\n\"\n"
				       "  -a, --allow-non-openssh   Accepts non OpenSSH servers\n"
				       "  -A, --allow-honeypots     Allow servers detected as honeypots\n"
				       "      --timeout <seconds>   Sets connection timeout (Default: 3)\n"
				       "      --check-http <host>   Tries to open a TCP Tunnel after successful login\n");
				exit(EXIT_SUCCESS);
			default:
				usage(argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	print_banner();

	/* Targets */
	while (optind < argc) {
		btkg_target_t *ret = btkg_target_parse(argv[optind]);

		if (ret == NULL) {
			log_error(
				"WARNING: An error occurred parsing target '%s' on argument #%d",
				argv[optind], optind);
			continue;
		}

		btkg_target_list_append_range(targets, ret->host, ret->port);
		free(ret->host);
		free(ret);
		optind++;
	}

	/* Load targets from default path */
	if (targets->targets == NULL)
		btkg_target_list_load(targets, "hostnames.txt");

	/* Load targets from default path */
	if (credentials->credentials == NULL)
		btkg_credentials_list_load(credentials, "combos.txt");

	/* Calculate total attempts */
	context.total = targets->length * credentials->length;

	printf("\nAmount of username/password combinations: %zu\n",
	       credentials->length);
	printf("Number of targets: %zu\n", targets->length);
	printf("Total attempts: %zu\n", context.total);
	printf("Max threads: %zu\n\n", options->max_threads);

	if (context.total == 0) {
		log_error("No work to do.");
		exit(EXIT_FAILURE);
	}

	if (options->max_threads > targets->length) {
		log_info("Decreasing max threads to %zu.", targets->length);
		options->max_threads = targets->length;
	}

	/* Output Format */
	if (options->bruteforce_output_format == NULL) {
		options->bruteforce_output_format = strdup(
			"%DATETIME%\t%HOSTNAME%:%PORT%\t%USERNAME%\t%PASSWORD%\n");
	}

	/* Output file */
	if (output_filename != NULL) {
		context.output = fopen(output_filename, "a");
		if (context.output == NULL) {
			log_error("Error opening output file. (%s)",
				  output_filename);
			exit(EXIT_FAILURE);
		}
	}
#ifdef _WIN32
	/* Initialize WinSock */
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		fprintf(stderr, "WSAStartup failed.\n");
		exit(1);
	}
#endif

	struct timespec start;
	double elapsed;

	/* Port scan and honeypot detection */
	if (options->perform_scan) {
		log_info("Starting servers discoverage process...");
		clock_gettime(CLOCK_MONOTONIC, &start);

		context.total = context.targets.length;
		context.count = 0;

		/* Scan output file */
		if (scan_output_filename != NULL) {
			context.scan_output = fopen(scan_output_filename, "a");
			if (context.scan_output == NULL) {
				log_error(
					"Error opening scan output file. (%s)",
					scan_output_filename);
				exit(EXIT_FAILURE);
			}
			free(scan_output_filename);

			/* Scanner Output Format */
			if (options->scanner_output_format == NULL) {
				options->scanner_output_format =
					strdup("%HOSTNAME%:%PORT%\t%BANNER%\n");
			}
		}

		btkg_progress_watcher_start(&context, &progress_watcher);

		detection_start(&context, targets, targets,
				options->max_threads);

		if (context.scan_output != NULL) {
			fclose(context.scan_output);
			context.scan_output = NULL;
		}

		if (options->scanner_output_format != NULL) {
			free(options->scanner_output_format);
			options->scanner_output_format = NULL;
		}

		btkg_progress_watcher_wait(&progress_watcher);

		elapsed = btkg_elapsed_time(&start);

		context.total = targets->length * credentials->length;

		log_info("Detection process took %f seconds.", elapsed);
		log_info("Number of targets after filtering: %zu.",
			 targets->length);
	}

	if (targets->length == 0) {
		log_info("No work to do.");
		goto _finalize;
	}

	if (options->max_threads > targets->length) {
		log_info("Decreasing max threads to %zu.", targets->length);
		options->max_threads = targets->length;
	}

	/* Bruteforce */
	log_info("Starting brute-force process...");
	clock_gettime(CLOCK_MONOTONIC, &start);

	btkg_progress_watcher_start(&context, &progress_watcher);

	btkg_bruteforce_start(&context);

	btkg_progress_watcher_wait(&progress_watcher);

	elapsed = btkg_elapsed_time(&start);

	log_info("Brute-force process took %f seconds.", elapsed);

_finalize:
	btkg_context_destroy(&context);


	return EXIT_SUCCESS;
}
