#include <libssh/libssh.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int verbose = 0;

void print_error(char *message)
{
    printf("\033[91m%s\033[0m\n", message);
}

void print_debug(char *message)
{
    if (verbose)
    printf("\033[37m%s\033[0m\n", message);
}

const char *str_repeat(char *str, size_t times)
{
    if (times < 1) return NULL;
    char *ret = malloc(sizeof(str) * times + 1);
    if (ret == NULL) return NULL;
    strcpy(ret, &str);
    while (--times > 0) {
        strcat(ret, &str);
    }
    return ret;
}

void update_progress(int count, int total, char* suffix, int bar_len)
{
    if (bar_len == NULL) bar_len = 60;
    if (suffix == NULL) suffix = "";

    int filled_len = bar_len * count / total;
    int empty_len = bar_len - filled_len;
    float percents = 100.0f * count / total;

    if (filled_len > 0) printf("\033[32m%s", str_repeat('=', filled_len));
    printf("\033[37m%s\033[0m", str_repeat('-', empty_len));
    printf("  %.2f%%   %s\r", percents, suffix);
    fflush(stdout);
}

void print_banner()
{
    printf(
        "\033[92m      _                _       _\n"
        "     | |              | |     | |\n"
        "     | |__  _ __ _   _| |_ ___| | ___ __ __ _  __ _\n"
        "     | '_ \\| '__| | | | __/ _ \\ |/ / '__/ _` |/ _` |\n"
        "     | |_) | |  | |_| | ||  __/   <| | | (_| | (_| |\n"
        "     |_.__/|_|   \\__,_|\\__\\___|_|\\_\\_|  \\__,_|\\__, |\n"
        "            \033[0m\033[1mOpenSSH Brute force tool 0.3.1\033[92m     __/ |\n"
        "          \033[0m(c) Copyright 2014 Jorge Matricali\033[92m  |___/\033[0m\n\n"
    );
}

int main(int argc, char** argv)
{
    int opt;

    while ((opt = getopt(argc, argv, "v")) != -1) {
        switch (opt) {
            case 'v':
                verbose = 1;
                break;
            default:
            fprintf(stderr, "Usage: %s [-ilw] [file...]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    print_banner();

    ssh_session my_ssh_session;
    int verbosity = SSH_LOG_PROTOCOL;
    int port = 22;

    my_ssh_session = ssh_new();

    if (my_ssh_session == NULL) {
        exit(-1);
    }

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "localhost");
    ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &port);

    int r;
    r = ssh_connect(my_ssh_session);
    if (r != SSH_OK) {
        fprintf(
            stderr,
            "Error connecting to localhost: %s\n",
            ssh_get_error(my_ssh_session)
        );
        exit(-1);
    }


    ssh_free(my_ssh_session);
}
