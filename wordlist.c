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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "wordlist.h"
#include "log.h"

wordlist_t wordlist_load(char *filename)
{
    FILE *fp;
    wordlist_t ret;
    char **words = NULL;
    ssize_t read;
    char *temp = 0;
    size_t len;

    /* Initialize wordlist */
    ret.lenght = 0;
    ret.words = NULL;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        log_error("Error opening file. (%s)", filename);
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
