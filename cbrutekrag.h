#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <libssh/libssh.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <errno.h>

typedef struct {
    size_t lenght;
    char **words;
} wordlist_t;

char** str_split(char* a_str, const char a_delim);
void print_error(const char *format, ...);
void print_debug(const char *format, ...);
const char *str_repeat(char *str, size_t times);
void update_progress(int count, int total, char* suffix, int bar_len);
void print_banner();
void usage(const char *p);
int try_login(const char *hostname, const char *username, const char *password);
wordlist_t load_wordlist(char *filename);
