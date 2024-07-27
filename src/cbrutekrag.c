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
#include "str.h"
#include "target.h"

#define NANO_PER_SEC 1000000000.0

int g_verbose = 0;
char *g_output_format = NULL;

/* Long options for getopt_long */
static struct option long_options[] = {
	{ "help", no_argument, NULL, 'h' },
	{ "verbose", no_argument, NULL, 'v' },
	{ "verbose-sshlib", no_argument, NULL, 'V' },
	{ "scan", no_argument, NULL, 's' },
	{ "dry-run", no_argument, NULL, 'D' },
	{ "progress", no_argument, NULL, 'P' },
	{ "targets", required_argument, NULL, 'T' },
	{ "credentials", required_argument, NULL, 'C' },
	{ "threads", required_argument, NULL, 't' },
	{ "output", required_argument, NULL, 'o' },
	{ "format", required_argument, NULL, 'F' },
	{ "allow-non-openssh", no_argument, NULL, 'a' },
	{ "allow-honeypots", no_argument, NULL, 'A' },
	{ "timeout", required_argument, NULL, 11 },
	{ NULL, 0, NULL, 0 }
};

void print_banner()
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

void usage(const char *p)
{
	printf("\nusage: %s [-h] [-v] [-aA] [-D] [-P] [-T TARGETS.lst] [-C credentials.lst]\n"
	       "\t\t[-t THREADS] [-F OUTPUT FORMAT] [-o OUTPUT.txt] [TARGETS...]\n\n",
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
static void btkg_console_setup()
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

void btkg_options_init(btkg_options_t *options)
{
	if (options == NULL)
		return;

	options->timeout = 3;
	options->max_threads = 1;
	options->progress_bar = 0;
	options->verbose = 0;
	options->dry_run = 0;
	options->perform_scan = 0;
	options->non_openssh = 0;
	options->allow_honeypots = 0;
}

void btkg_context_init(btkg_context_t *context)
{
	if (context == NULL) {
		return;
	}

	btkg_options_init(&context->options);

	context->output = NULL;
	context->count = 0;
	context->successful = 0;
	context->total = 0;
	context->credentials_idx = 0;
	context->targets_idx = 0;

	btkg_credentials_list_init(&context->credentials);
	btkg_target_list_init(&context->targets);
}

int main(int argc, char **argv)
{
	int opt;
	int option_index = 0;
	char *hostnames_filename = NULL;
	char *credentials_filename = NULL;
	char *output_filename = NULL;
	int tempint;

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

	while ((opt = getopt_long(argc, argv, "aAT:C:t:o:F:DsvVPh",
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
				g_verbose = options->verbose;
				break;
			case 'V':
				options->verbose |= CBRUTEKRAG_VERBOSE_SSHLIB;
				break;
			case 'T':
				hostnames_filename = strdup(optarg);
				break;
			case 'C':
				credentials_filename = strdup(optarg);
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
			case 'F':
				g_output_format = strdup(optarg);
				btkg_str_replace_escape_sequences(
					g_output_format);
				break;
			case 'o':
				output_filename = strdup(optarg);
				break;
			case 's':
				options->perform_scan = 1;
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
				       "  -a, --allow-non-openssh   Accepts non OpenSSH servers\n"
				       "  -A, --allow-honeypots     Allow servers detected as honeypots\n"
				       "      --timeout <seconds>   Sets connection timeout (Default: 3)\n");
				exit(EXIT_SUCCESS);
			default:
				usage(argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	print_banner();

	btkg_target_list_t *targets = &context.targets;

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

	if (targets->targets == NULL && hostnames_filename == NULL)
		hostnames_filename = strdup("hostnames.txt");

	if (hostnames_filename != NULL) {
		btkg_target_list_load(targets, hostnames_filename);
		free(hostnames_filename);
	}

	if (credentials_filename == NULL)
		credentials_filename = strdup("combos.txt");

	/* Load username/password combinations */
	btkg_credentials_list_load(&context.credentials, credentials_filename);
	free(credentials_filename);

	/* Calculate total attempts */
	btkg_credentials_list_t *credentials = &context.credentials;

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
	if (g_output_format == NULL) {
		g_output_format = strdup(
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

	struct timespec start, end;
	double elapsed;

	/* Port scan and honeypot detection */
	if (options->perform_scan) {
		log_info("Starting servers discoverage process...");
		clock_gettime(CLOCK_MONOTONIC, &start);

		detection_start(&context, targets, targets,
				options->max_threads);

		clock_gettime(CLOCK_MONOTONIC, &end);
		elapsed = (double)(end.tv_sec - start.tv_sec);
		elapsed += (double)(end.tv_nsec - start.tv_nsec) / NANO_PER_SEC;

		context.total = targets->length * credentials->length;

		log_info("Detection process took %f seconds.", elapsed);
		log_info("Number of targets after filtering: %zu.",
			 targets->length);
	}

	if (targets->length == 0) {
		log_info("No work to do.");
		exit(EXIT_SUCCESS);
	}

	if (options->max_threads > targets->length) {
		log_info("Decreasing max threads to %zu.", targets->length);
		options->max_threads = targets->length;
	}

	/* Bruteforce */
	log_info("Starting brute-force process...");
	clock_gettime(CLOCK_MONOTONIC, &start);

	btkg_bruteforce_start(&context);

	clock_gettime(CLOCK_MONOTONIC, &end);
	elapsed = (double)(end.tv_sec - start.tv_sec);
	elapsed += (double)(end.tv_nsec - start.tv_nsec) / NANO_PER_SEC;

	log_info("Brute-force process took %f seconds.", elapsed);

	btkg_credentials_list_destroy(credentials);
	btkg_target_list_destroy(targets);

	if (context.output != NULL)
		fclose(context.output);

	if (g_output_format != NULL) {
		free(g_output_format);
		g_output_format = NULL;
	}

	exit(EXIT_SUCCESS);
}
