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

/**
 * @file compat.h
 * @brief Compatibility layer for systems lacking certain standard library functions.
 *
 * This header provides declarations for functions that may not be available
 * on all platforms. If the functions are not available, they are implemented
 * elsewhere in the codebase to ensure compatibility across different systems.
 */

#ifndef COMPAT_H
#define COMPAT_H

#include <stdio.h>

/**
 * @brief Read a delimited record from a stream.
 *
 * This function reads a line from the specified stream, delimited by the
 * specified character. It is intended for platforms that do not support
 * the `getdelim` function.
 *
 * @param buf Pointer to the buffer where the line should be stored.
 * @param bufsiz Pointer to the size of the buffer.
 * @param delimiter The character that delimits the record.
 * @param fp The file stream to read from.
 * @return The number of characters read, or -1 on error or end of file.
 */
#if !HAVE_GETDELIM
ssize_t getdelim(char **buf, size_t *bufsiz, int delimiter, FILE *fp);
#endif

/**
 * @brief Read an entire line from a stream.
 *
 * This function reads a line from the specified stream, stopping at the end
 * of the line or the end of the file. It is intended for platforms that do not
 * support the `getline` function.
 *
 * @param buf Pointer to the buffer where the line should be stored.
 * @param bufsiz Pointer to the size of the buffer.
 * @param fp The file stream to read from.
 * @return The number of characters read, or -1 on error or end of file.
 */
#if !HAVE_GETLINE
ssize_t getline(char **buf, size_t *bufsiz, FILE *fp);
#endif

#endif /* COMPAT_H */
