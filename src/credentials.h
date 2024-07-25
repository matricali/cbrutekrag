/*
Copyright (c) 2014-2024 Jorge Matricali

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
	char username[LOGIN_NAME_MAX + 1];
	char password[LOGIN_PASS_MAX + 1];
} btkg_credentials_t;

typedef struct {
	size_t length;
	btkg_credentials_t *credentials;
} btkg_credentials_list_t;

/**
 * @brief Initializes a btkg_credentials_list_t structure.
 *
 * @param credentials Pointer to the btkg_credentials_list_t structure to initialize.
 */
void btkg_credentials_list_init(btkg_credentials_list_t *credentials);

/**
 * @brief Parses a line into a btkg_credentials_t structure.
 *
 * @param line The line to parse.
 * @param dst Pointer to the btkg_credentials_t structure to fill with parsed data.
 *
 * @return 0 on success, non-zero on failure.
 */
int btkg_credentials_parse(char *line, btkg_credentials_t *dst);

/**
 * @brief Loads credentials from a given file and appends them into the given btkg_credentials_list_t.
 *
 * @param credentials_list Pointer to the btkg_credentials_list_t structure to append the loaded credentials to.
 * @param filename The name of the file to load the credentials from.
 */
void btkg_credentials_list_load(btkg_credentials_list_t *credentials_list,
				char *filename);

/**
 * @brief Appends a btkg_credentials_t structure into a given btkg_credentials_list_t.
 *
 * @param credentials_list Pointer to the btkg_credentials_list_t structure to append the credentials to.
 * @param new The btkg_credentials_t structure to append.
 */
void btkg_credentials_list_append(btkg_credentials_list_t *credentials_list,
				  btkg_credentials_t new);

/**
 * @brief Frees the memory allocated for a btkg_credentials_list_t structure.
 *
 * @param credentials_list Pointer to the btkg_credentials_list_t structure to free.
 */
void btkg_credentials_list_destroy(btkg_credentials_list_t *credentials_list);

#endif // __BTKG_CREDENTIALS_H
