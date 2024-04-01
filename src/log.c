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

#include <stdarg.h> /* va_list, va_start, va_end */
#include <stdio.h> /* fprintf, vfprintf, stderr */
#include <string.h> /* strlen, malloc, strncpy */
#include <time.h> /* time_t, time, tm, localtime, strftime */

#include "cbrutekrag.h" /* CBRUTEKRAG_VERBOSE_MODE */
#include "log.h"
#include "str.h" /* replace_placeholder */

extern int g_verbose;
extern char *g_output_format;

void print_output(int level, const char *file, int line, const char *head,
		  const char *tail, FILE *stream, const char *format, ...)
{
	if (level == LOG_DEBUG && !(g_verbose & CBRUTEKRAG_VERBOSE_MODE)) {
		return;
	}
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);

	va_list arg;
	char s[20];

	s[strftime(s, sizeof(s), "%Y/%m/%d %H:%M:%S", tm)] = '\0';
	fprintf(stream, "\033[2K\r%s[%s] ", head, s);

#ifndef DEBUG
	if (level == LOG_DEBUG)
#endif
		fprintf(stream, "%s:%d ", file, line);

	va_start(arg, format);
	vfprintf(stream, format, arg);
	va_end(arg);
	fprintf(stream, "%s\n", tail);
	fflush(stream);
}

void log_output(FILE *stream, const char *format, ...)
{
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);

	va_list arg;
	char s[20];

	s[strftime(s, sizeof(s), "%Y/%m/%d %H:%M:%S", tm)] = '\0';
	fprintf(stream, "%s ", s);

	va_start(arg, format);
	vfprintf(stream, format, arg);
	va_end(arg);
	fflush(stream);
}

void btkg_log_successfull_login(FILE *stream, const char *hostname, int port,
				const char *username, const char *password)
{
	int port_len = snprintf(NULL, 0, "%d", port);
	char strport[port_len];

	sprintf(strport, "%d", port);

	// Allocation
	size_t output_len = sizeof(char) * (strlen(g_output_format) + 1);
	char *output = malloc(output_len);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
	strncpy(output, g_output_format, output_len);
#pragma GCC diagnostic pop
	// Timestamp
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	char s[20];

	s[strftime(s, sizeof(s), "%Y/%m/%d %H:%M:%S", tm)] = '\0';

	output = btkg_str_replace_placeholder(output, "%DATETIME%", s);
	output = btkg_str_replace_placeholder(output, "%HOSTNAME%", hostname);
	output = btkg_str_replace_placeholder(output, "%USERNAME%", username);
	output = btkg_str_replace_placeholder(output, "%PASSWORD%", password);
	output = btkg_str_replace_placeholder(output, "%PORT%", strport);

	fprintf(stream, "%s", output);
	free(output);
}
