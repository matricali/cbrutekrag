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

#include <stdarg.h> /* va_list, va_start, va_end */
#include <stdio.h> /* fprintf, vfprintf, stderr */
#include <string.h> /* strlen, malloc */
#include <time.h> /* time_t, time, tm, localtime, strftime */

#include "cbrutekrag.h" /* CBRUTEKRAG_VERBOSE_MODE */
#include "log.h"
#include "str.h" /* btkg_str_replace_placeholder */

/** Global verbosity level. */
static int g_verbose;

#define TIMESTAMP_BUFFER_SIZE 20

/**
 * @brief Get the current timestamp in the format YYYY/MM/DD HH:MM:SS.
 *
 * @return A pointer to a static buffer containing the timestamp.
 */
static inline const char *get_current_timestamp(void)
{
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	static char buffer[TIMESTAMP_BUFFER_SIZE];

	buffer[strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", tm)] =
		'\0';

	return buffer;
}

/**
 * @brief Print formatted output to a specified stream with optional logging
 *        information such as file and line number.
 *
 * @param level The logging level of the message (e.g., LOG_DEBUG).
 * @param file The source file from which the log is coming.
 * @param line The line number in the source file.
 * @param head A prefix string to be printed before the log message.
 * @param tail A suffix string to be printed after the log message.
 * @param stream The output stream (e.g., stdout or stderr).
 * @param format The format string for the log message.
 * @param ... Additional arguments for the format string.
 */
void print_output(int level, const char *file, int line, const char *head,
		  const char *tail, FILE *stream, const char *format, ...)
{
	if (level == LOG_DEBUG && !(g_verbose & CBRUTEKRAG_VERBOSE_MODE)) {
		return;
	}

	va_list arg;

	fprintf(stream, "\033[2K\r%s[%s] ", head, get_current_timestamp());

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

/**
 * @brief Print formatted output to a specified stream with the current timestamp.
 *
 * @param stream The output stream (e.g., stdout or stderr).
 * @param format The format string for the log message.
 * @param ... Additional arguments for the format string.
 */
void log_output(FILE *stream, const char *format, ...)
{
	va_list arg;

	fprintf(stream, "%s ", get_current_timestamp());

	va_start(arg, format);
	vfprintf(stream, format, arg);
	va_end(arg);
	fflush(stream);
}

/**
 * @brief Log a successful login attempt with detailed information such as
 *        hostname, port, username, and password, formatted according to
 *        a global output format string.
 *
 * @param stream The output stream (e.g., stdout or stderr).
 * @param hostname The hostname or IP address where the login was successful.
 * @param port The port number used in the login attempt.
 * @param username The username used in the login attempt.
 * @param password The password used in the login attempt.
 */
void btkg_log_successfull_login(FILE *stream, const char *format,
				const char *hostname, int port,
				const char *username, const char *password)
{
	if (format == NULL) {
		log_error("bruteforce_output_format is NULL");
		return;
	}

	int port_len = snprintf(NULL, 0, "%d", port);
	char strport[port_len + 1]; // +1 for the null terminator

	snprintf(strport, sizeof(strport), "%d", port);

	// Allocation
	size_t output_len = strlen(format) + 1;
	char *output = malloc(output_len);

	if (output == NULL) {
		log_error("Error allocating memory");
		return;
	}

	snprintf(output, output_len, "%s", format);

	output = btkg_str_replace_placeholder(output, "%DATETIME%",
					      get_current_timestamp());
	if (output == NULL)
		goto error;

	output = btkg_str_replace_placeholder(output, "%HOSTNAME%", hostname);
	if (output == NULL)
		goto error;

	output = btkg_str_replace_placeholder(output, "%USERNAME%", username);
	if (output == NULL)
		goto error;

	output = btkg_str_replace_placeholder(output, "%PASSWORD%", password);
	if (output == NULL)
		goto error;

	output = btkg_str_replace_placeholder(output, "%PORT%", strport);
	if (output == NULL)
		goto error;

	// Print buffer
	fprintf(stream, "%s", output);
	free(output);
	fflush(stream);

	return;

error:
	log_error("Error replacing placeholders");
	free(output);
}

/**
 * @brief Log elegible target found with detailed information formatted
 *        according to a global output format string.
 *
 * @param stream The output stream (e.g., stdout or stderr).
 * @param hostname The hostname or IP address where the login was successful.
 * @param port The port number used in the login attempt.
 * @param banner The server banner.
 * @param password The password used in the login attempt.
 */
void btkg_log_target_found(FILE *stream, const char *format,
			   const char *hostname, int port, const char *banner)
{
	if (format == NULL) {
		log_error("scanner_output_format is NULL");
		return;
	}

	int port_len = snprintf(NULL, 0, "%d", port);
	char strport[port_len + 1]; // +1 for the null terminator

	snprintf(strport, sizeof(strport), "%d", port);

	// Allocation
	size_t output_len = strlen(format) + 1;
	char *output = malloc(output_len);

	if (output == NULL) {
		log_error("Error allocating memory");
		return;
	}

	snprintf(output, output_len, "%s", format);

	output = btkg_str_replace_placeholder(output, "%DATETIME%",
					      get_current_timestamp());
	if (output == NULL)
		goto error;

	output = btkg_str_replace_placeholder(output, "%HOSTNAME%", hostname);
	if (output == NULL)
		goto error;

	output = btkg_str_replace_placeholder(output, "%PORT%", strport);
	if (output == NULL)
		goto error;

	output = btkg_str_replace_placeholder(output, "%BANNER%", banner);
	if (output == NULL)
		goto error;

	// Print buffer
	fprintf(stream, "%s", output);
	free(output);
	fflush(stream);

	return;

error:
	log_error("Error replacing placeholders");
	free(output);
}

/**
 * @brief Sets the logging verbosity level.
 *
 * This function sets the global logging verbosity level based on the given
 * level parameter. The verbosity level can be used to control the amount of
 * detail output in log messages.
 *
 * The verbosity level is typically a bitwise OR of different flags that
 * enable various levels of detail. For example, the following flags might
 * be combined:
 * - `CBRUTEKRAG_VERBOSE_MODE` (0x1): Enables basic verbose logging.
 * - `CBRUTEKRAG_VERBOSE_SSHLIB` (0x2): Enables verbose logging for the SSH library.
 *
 * Example usage:
 * @code
 * options->verbose |= CBRUTEKRAG_VERBOSE_MODE;
 * log_set_level(options->verbose);
 * @endcode
 *
 * @param level The verbosity level to set. This is typically a combination of
 *        flags defined as bitwise values (e.g., `CBRUTEKRAG_VERBOSE_MODE`).
 */
void log_set_level(int level)
{
	g_verbose = level;
}
