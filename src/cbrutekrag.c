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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h> /* waitpid */
#include <unistd.h> /* fork */

#include "bruteforce_ssh.h"
#include "cbrutekrag.h"
#include "detection.h"
#include "log.h"
#include "progressbar.h"
#include "str.h"
#include "target.h"
#include "wordlist.h"

void print_banner()
{
    printf(
        "\033[92m           _                _       _\n"
        "          | |              | |     | |\n"
        "\033[37m      ___\033[92m | |__  _ __ _   _| |_ ___| | ___ __ __ _  __ _\n"
        "\033[37m     / __|\033[92m| '_ \\| '__| | | | __/ _ \\ |/ / '__/ _` |/ _` |\n"
        "\033[37m    | (__ \033[92m| |_) | |  | |_| | ||  __/   <| | | (_| | (_| |\n"
        "\033[37m     \\___|\033[92m|_.__/|_|   \\__,_|\\__\\___|_|\\_\\_|  \\__,_|\\__, |\n"
        "              \033[0m\033[1mOpenSSH Brute force tool 0.5.0\033[0m\033[92m        __/ |\n"
        "          \033[0m(c) Copyright 2014-2018 Jorge Matricali\033[92m  |___/\033[0m\n\n");
}

void usage(const char* p)
{
    printf("\nusage: %s [-h] [-v] [-D] [-P] [-p PORT] [-T TARGETS.lst] [-C combinations.lst]\n"
           "\t\t[-t THREADS] [-o OUTPUT.txt] [TARGETS...]\n\n", p);
}

int main(int argc, char** argv)
{
    int opt;
    int total = 0;
    int THREADS = 1;
    char* hostnames_filename = NULL;
    char* combos_filename = NULL;
    char* output_filename = NULL;
    int port = 22;
    FILE* output = NULL;
    char* g_blankpass_placeholder = "$BLANKPASS";
    btkg_context_t context = { 3, 0 };

    while ((opt = getopt(argc, argv, "p:T:C:t:o:DsvVPh")) != -1) {
        switch (opt) {
            case 'v':
                context.verbose |= CBRUTEKRAG_VERBOSE_MODE;
                break;
            case 'V':
                context.verbose |= CBRUTEKRAG_VERBOSE_SSHLIB;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'T':
                hostnames_filename = optarg;
                break;
            case 'C':
                combos_filename = optarg;
                break;
            case 't':
                THREADS = atoi(optarg);
                break;
            case 'o':
                output_filename = optarg;
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
                       "  -p <port>         Port (default: 22)\n"
                       "  -T <targets>      Targets file\n"
                       "  -C <combinations> Username and password file\n"
                       "  -t <threads>      Max threads\n"
                       "  -o <output>       Output log file\n");
                exit(EXIT_SUCCESS);
            default:
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    print_banner();

    if (!btkg_target_port_is_valid(port)) {
        log_error("Invalid port. (%d)", port);
        exit(EXIT_FAILURE);
    }

    /* Targets */
    btkg_target_list_t target_list;
    btkg_target_list_init(&target_list);

    while (optind < argc) {
        btkg_target_t ret = target_parse(argv[optind]);

        if (ret.host == NULL) {
            log_error("WARNING: An error ocurred parsing target '%s' on argument #%d", argv[optind], optind);
            continue;
        }

        btkg_target_list_append_range(&target_list, ret.host, ret.port);
        free(ret.host);
        optind++;
    }
    if (target_list.targets == NULL && hostnames_filename == NULL) {
        hostnames_filename = strdup("hostnames.txt");
    }
    if (hostnames_filename != NULL) {
        btkg_target_list_load(&target_list, hostnames_filename);
    }

    /* Load username/password combinations */
    if (combos_filename == NULL) {
        combos_filename = strdup("combos.txt");
    }
    wordlist_t combos = wordlist_load(combos_filename);
    free(combos_filename);

    /* Calculate total attemps */
    total = target_list.length * combos.length;

    printf("\nAmount of username/password combinations: %zu\n", combos.length);
    printf("Number of targets: %zu\n", target_list.length);
    printf("Port: %d\n", port);
    printf("Total attemps: %d\n", total);
    printf("Max threads: %d\n\n", THREADS);

    if (total == 0) {
        log_error("No work to do.");
        exit(EXIT_FAILURE);
    }

    /* Output file */
    if (output_filename != NULL) {
        output = fopen(output_filename, "a");
        if (output == NULL) {
            log_error("Error opening output file. (%s)", output_filename);
            exit(EXIT_FAILURE);
        }
    }

    /* Port scan and honeypot detection */
    if (context.perform_scan) {
        printf("Starting servers discoverage process...\n\n");
        detection_start(&context, &target_list, &target_list, THREADS);
        printf("\n\nNumber of targets after filtering: %zu\n", target_list.length);
    }

    if (THREADS > target_list.length) {
        printf("Decreasing max threads to %zu.\n", target_list.length);
        THREADS = target_list.length;
    }

    /* Bruteforce */
    pid_t pid = 0;
    int p = 0;
    int count = 0;

    for (int x = 0; x < combos.length; x++) {
        char** login_data = str_split(combos.words[x], ' ');
        if (login_data == NULL) {
            continue;
        }
        if (strcmp(login_data[1], g_blankpass_placeholder) == 0) {
            login_data[1] = strdup("");
        }
        for (int y = 0; y < target_list.length; y++) {

            if (p >= THREADS) {
                waitpid(-1, NULL, 0);
                p--;
            }

            btkg_target_t current_target = target_list.targets[y];

            pid = fork();

            if (pid) {
                p++;
            } else if (pid == 0) {
                if (!context.dry_run) {
                    bruteforce_ssh_try_login(&context, current_target.host, current_target.port, login_data[0],
                        login_data[1], count, total, output);
                }
                exit(EXIT_SUCCESS);
            } else {
                log_error("Fork failed!");
            }

            count++;
        }
    }

    pid = 0;

    wordlist_destroy(&combos);

    if (output != NULL) {
        fclose(output);
    }

    if (context.progress_bar) {
        progressbar_render(count, total, NULL, -1);
        printf("\f");
    }

    exit(EXIT_SUCCESS);
}
