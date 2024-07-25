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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"

/**
 * @brief Safely copies a string to a destination buffer ensuring null termination.
 *
 * @param dst Destination buffer.
 * @param src Source string.
 * @param dst_size Size of the destination buffer.
 */
void btkg_str_copy(char *dst, const char *src, size_t dst_size)
{
	if (dst_size > 0) {
		strncpy(dst, src, dst_size - 1);
		dst[dst_size - 1] = '\0'; // Ensure null-termination
	}
}

char **str_split(char *a_str, const char a_delim)
{
	char **result = 0;
	size_t count = 0;
	char *tmp = a_str;
	char *last_comma = 0;
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

	result = malloc(sizeof(char *) * count);

	if (result) {
		size_t idx = 0;
		char *token = strtok(a_str, delim);

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

// You must free the result if result is non-NULL.
const char *str_repeat(char *str, size_t times)
{
	if (times < 1)
		return NULL;
	char *ret = malloc(sizeof(str) * times + 1);
	if (ret == NULL)
		return NULL;
	strcpy(ret, str);
	while (--times > 0) {
		strcat(ret, str);
	}
	return ret;
}

// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with)
{
	char *result;
	char *ins;
	char *tmp;
	size_t len_rep;
	size_t len_with;
	size_t len_front;
	size_t count;

	// sanity checks and initialization
	if (!orig || !rep)
		return NULL;
	len_rep = strlen(rep);
	if (len_rep == 0)
		return NULL; // empty rep causes infinite loop during count
	if (!with)
		with = "";
	len_with = strlen(with);

	// count the number of replacements needed
	ins = orig;
	for (count = 0; (tmp = strstr(ins, rep)); ++count) {
		ins = tmp + len_rep;
	}

	tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

	if (!result)
		return NULL;

	// first time through the loop, all the variable are set correctly
	// from here on,
	//    tmp points to the end of the result string
	//    ins points to the next occurrence of rep in orig
	//    orig points to the remainder of orig after "end of rep"
	while (count--) {
		ins = strstr(orig, rep);
		len_front = (size_t)(ins - orig);
		tmp = strncpy(tmp, orig, len_front) + len_front;
		tmp = strcpy(tmp, with) + len_with;
		orig += len_front + len_rep; // move to next "end of rep"
	}
	strcpy(tmp, orig);
	return result;
}

char *btkg_str_replace_placeholder(char *input, const char *search,
				   const char *replace)
{
	char *tmp = NULL;
	tmp = str_replace(input, (char *)search, (char *)replace);
	if (tmp) {
		if (input)
			free(input);
		return tmp;
	}
	return input;
}

void btkg_str_replace_escape_sequences(char *str)
{
	char *read = str;
	char *write = str;
	while (*read) {
		if (*read == '\\' && *(read + 1)) {
			read++;
			switch (*read) {
				case 'n':
					*write++ = '\n';
					break;
				case 't':
					*write++ = '\t';
					break;
				// --
				default:
					*write++ = '\\';
					*write++ = *read;
			}
		} else {
			*write++ = *read;
		}
		read++;
	}
	*write = '\0';
}
