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

#include <stdio.h>
#include <string.h>

#include "compat.h"
#include "credentials.h"
#include "log.h"
#include "str.h"

char *g_blankpass_placeholder = "$BLANKPASS";

/**
 * @brief Parses a line into a btkg_credentials_t structure.
 *
 * @param line The line to parse.
 * @param dst Pointer to the btkg_credentials_t structure to fill with parsed data.
 *
 * @return 0 on success, non-zero on failure.
 */
int btkg_credentials_parse(char *line, btkg_credentials_t *dst)
{
	dst->username[0] = '\0';
	dst->password[0] = '\0';

	char *username = strtok(line, " ");
	if (username == NULL)
		return -1;
	btkg_str_copy(dst->username, username, LOGIN_NAME_MAX);

	char *password = strtok(NULL, "\n");
	if (password == NULL)
		return 0;

	if (strcmp(password, g_blankpass_placeholder) != 0)
		btkg_str_copy(dst->password, password, LOGIN_PASS_MAX);

	return 0;
}

/**
 * @brief Initializes a btkg_credentials_list_t structure.
 *
 * @param credentials Pointer to the btkg_credentials_list_t structure to initialize.
 */
void btkg_credentials_list_init(btkg_credentials_list_t *credentials)
{
	credentials->length = 0;
	credentials->credentials = NULL;
}

/**
 * @brief Loads credentials from a given file and appends them into the given
 *	      btkg_credentials_list_t.
 *
 * @param credentials_list Pointer to the btkg_credentials_list_t structure to append the loaded credentials to.
 * @param filename The name of the file to load the credentials from.
 */
void btkg_credentials_list_load(btkg_credentials_list_t *credentials_list,
				char *filename)
{
	FILE *fp;
	ssize_t read;
	char *temp = NULL;
	size_t len = 0;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		log_error("Error opening file. (%s)", filename);
		return;
	}

	for (int lines = 1; (read = getline(&temp, &len, fp)) != -1; lines++) {
		temp[strcspn(temp, "\n")] = '\0';

		btkg_credentials_t credentials;

		if (btkg_credentials_parse(temp, &credentials) != 0) {
			log_error("WARNING: Error parsing '%s' on line #%d",
				  filename, lines);
			continue;
		}

		btkg_credentials_list_append(credentials_list, credentials);
	}

	free(temp);
	fclose(fp);
}

/**
 * @brief Appends a btkg_credentials_t structure into a given btkg_credentials_list_t.
 *
 * @param credentials_list Pointer to the btkg_credentials_list_t structure to append the credentials to.
 * @param new The btkg_credentials_t structure to append.
 */
void btkg_credentials_list_append(btkg_credentials_list_t *credentials_list,
				  btkg_credentials_t new)
{
	btkg_credentials_t *credentials = credentials_list->credentials;

	if (credentials == NULL) {
		credentials = malloc(sizeof(new));
		*credentials = new;
	} else {
		credentials =
			realloc(credentials,
				sizeof(new) * (credentials_list->length + 1));
		*(credentials + credentials_list->length) = new;
	}

	credentials_list->length = credentials_list->length + 1;
	credentials_list->credentials = credentials;
}

/**
 * @brief Frees the memory allocated for a btkg_credentials_list_t structure.
 *
 * @param credentials_list Pointer to the btkg_credentials_list_t structure to free.
 */
void btkg_credentials_list_destroy(btkg_credentials_list_t *credentials_list)
{
	free(credentials_list->credentials);
	credentials_list->credentials = NULL;
	credentials_list->length = 0;
}
