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

#ifndef BRUTEFORCE_SSH_H
#define BRUTEFORCE_SSH_H

#include "cbrutekrag.h"
#include "target.h"
#include "wordlist.h"

typedef struct {
    btkg_target_list_t* targets;
    wordlist_t* combinations;
} btkg_bruteforce_args_t;

int bruteforce_ssh_login(btkg_context_t* context, const char* hostname, unsigned int port, const char* username,
    const char* password);
int bruteforce_ssh_try_login(btkg_context_t* context, const char* hostname, const int port, const char* username,
    const char* password, int count, int total, FILE* output);

#endif /* BRUTEFORCE_SSH_H */
