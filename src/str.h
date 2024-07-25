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

#ifndef STR_H
#define STR_H

#include <stdlib.h>

/**
 * @brief Safely copies a string to a destination buffer ensuring null termination.
 *
 * @param dst Destination buffer.
 * @param src Source string.
 * @param dst_size Size of the destination buffer.
 */
void btkg_str_copy(char *dst, const char *src, size_t dst_size);

/**
 * @brief Replaces all occurrences of a substring within a string with another substring.
 *
 * @param orig The original string.
 * @param rep The substring to replace.
 * @param with The replacement substring.
 *
 * @return A new string with the replacements. Should be freed by the caller.
 */
char *btkg_str_replace(const char *orig, const char *rep, const char *with);

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
				   const char *replace);

/**
 * @brief Replaces escape sequences in a string with their corresponding characters.
 *
 * @param str The input string containing escape sequences.
 */
void btkg_str_replace_escape_sequences(char *str);

#endif /* STR_H */
