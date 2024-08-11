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

#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

/**
 * @brief Logging levels.
 */
enum {
	LOG_TRACE, /**< Trace level for detailed debugging. */
	LOG_DEBUG, /**< Debug level for general debugging information. */
	LOG_INFO, /**< Info level for informational messages. */
	LOG_WARN, /**< Warn level for warnings and potential issues. */
	LOG_ERROR, /**< Error level for error messages. */
	LOG_FATAL /**< Fatal level for critical errors that may cause termination. */
};

/**
 * @brief Log an error message with red color and file/line information.
 *
 * This macro calls `print_output` with the LOG_ERROR level, red color for
 * the message, and includes file and line information.
 *
 * @param ... Format string and arguments for the error message.
 */
#define log_error(...)                                                         \
	print_output(LOG_ERROR, __FILE__, __LINE__, "\033[91m", "\033[0m",     \
		     stderr, __VA_ARGS__)

/**
 * @brief Log a warning message without color and with file/line information.
 *
 * This macro calls `print_output` with the LOG_WARN level and includes file
 * and line information.
 *
 * @param ... Format string and arguments for the warning message.
 */
#define log_warn(...)                                                          \
	print_output(LOG_WARN, __FILE__, __LINE__, "", "", stderr, __VA_ARGS__)

/**
 * @brief Log a debug message with gray color and file/line information.
 *
 * This macro calls `print_output` with the LOG_DEBUG level, gray color for
 * the message, and includes file and line information.
 *
 * @param ... Format string and arguments for the debug message.
 */
#define log_debug(...)                                                         \
	print_output(LOG_DEBUG, __FILE__, __LINE__, "\033[37m", "\033[0m",     \
		     stderr, __VA_ARGS__)

/**
 * @brief Log an informational message without color and with no file/line
 *        information.
 *
 * This macro calls `print_output` with the LOG_INFO level and writes to
 * stdout.
 *
 * @param ... Format string and arguments for the informational message.
 */
#define log_info(...)                                                          \
	print_output(LOG_INFO, __FILE__, __LINE__, "", "", stdout, __VA_ARGS__)

/**
 * @brief Print a formatted output message to a specified stream with optional
 *        logging information such as file and line number.
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
		  const char *tail, FILE *stream, const char *format, ...);

/**
 * @brief Print a formatted output message with the current timestamp to a
 *        specified stream.
 *
 * @param stream The output stream (e.g., stdout or stderr).
 * @param format The format string for the log message.
 * @param ... Additional arguments for the format string.
 */
void log_output(FILE *stream, const char *format, ...);

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
void log_set_level(int level);

/**
 * @brief Log a successful login attempt with details formatted according to
 *        a global output format string.
 *
 * @param stream The output stream (e.g., stdout or stderr).
 * @param hostname The hostname or IP address where the login was successful.
 * @param port The port number used in the login attempt.
 * @param username The username used in the login attempt.
 * @param password The password used in the login attempt.
 */
void btkg_log_successfull_login(FILE *stream, const char *hostname, int port,
				const char *username, const char *password);

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
void btkg_log_target_found(FILE *stream, const char *hostname, int port,
			   const char *banner);

#endif // LOGGER_H
