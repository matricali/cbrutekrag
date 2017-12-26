#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <libssh/libssh.h>
#include <sys/wait.h>

#define BUFFSIZE 1024

typedef struct {
    char *hostname;
    char *username;
    char *password;
} intento_t;

typedef struct {
    size_t lenght;
    char **words;
} wordlist_t;

char** str_split(char* a_str, const char a_delim);
void print_error(char *message);
void print_debug(char *message);
const char *str_repeat(char *str, size_t times);
void update_progress(int count, int total, char* suffix, int bar_len);
void print_banner();
int try_login(const char *hostname, const char *username, const char *password);
wordlist_t load_wordlist(char *filename);
