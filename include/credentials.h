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

#ifndef __BTKG_CREDENTIALS_H
#define __BTKG_CREDENTIALS_H

#include <stdlib.h>

#ifndef LOGIN_NAME_MAX
#define LOGIN_NAME_MAX 32
#endif
#ifndef LOGIN_PASS_MAX
#define LOGIN_PASS_MAX 100
#endif

typedef struct {
	char username[LOGIN_NAME_MAX];
	char password[LOGIN_PASS_MAX];
} btkg_credentials_t;

typedef struct {
	size_t length;
	btkg_credentials_t *credentials;
} btkg_credentials_list_t;

/**
 * Initialize btkg_credentials_list_t
 */
void btkg_credentials_list_init(btkg_credentials_list_t *credentials);

/**
 * Parse line into btkg_credentials_t structure
 */
int btkg_credentials_parse(char *line, btkg_credentials_t *dst);

/**
 * Loads credentials from a given file and append them into the given
 * btkg_credentials_list_t
 */
void btkg_credentials_list_load(btkg_credentials_list_t *credentials_list,
				char *filename);

/**
 * Append btkg_credentials_t into given btkg_credentials_list_t
 */
void btkg_credentials_list_append(btkg_credentials_list_t *credentials_list,
				  btkg_credentials_t new);

/**
 * Free btkg_credentials_list_t
 */
void btkg_credentials_list_destroy(btkg_credentials_list_t *credentials_list);

#endif /* __BTKG_CREDENTIALS_H */
