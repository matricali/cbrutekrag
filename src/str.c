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

/**
 * @brief Replaces all occurrences of a substring within a string with another substring.
 *
 * @param orig The original string.
 * @param rep The substring to replace.
 * @param with The replacement substring.
 *
 * @return A new string with the replacements. Should be freed by the caller.
 */
char *btkg_str_replace(const char *orig, const char *rep, const char *with)
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
	ins = (char *)orig;
	for (count = 0; (tmp = strstr(ins, rep)); ++count) {
		ins = tmp + len_rep;
	}

	tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);
	if (!result) {
		perror("malloc");
		return NULL;
	}

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

/**
 * @brief Replaces placeholders in a string with specified values.
 *
 * @param input The input string containing placeholders.
 * @param search The placeholder to search for.
 * @param replace The string to replace the placeholder with.
 *
 * @return A new string with placeholders replaced. Should be freed by the caller.
 */
char *btkg_str_replace_placeholder(char *input, const char *search,
				   const char *replace)
{
	char *tmp = btkg_str_replace(input, search, replace);
	if (tmp) {
		if (input)
			free(input);
		return tmp;
	}
	return input;
}

/**
 * @brief Replaces escape sequences in a string with their corresponding characters.
 *
 * @param str The input string containing escape sequences.
 */
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
