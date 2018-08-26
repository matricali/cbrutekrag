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

#ifndef LOGGER_H
#define LOGGER_H

extern int g_verbose;

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#define log_error(...) print_output(LOG_ERROR, __FILE__, __LINE__, \
    "\033[91m", "\033[0m", stderr, __VA_ARGS__)
#define log_debug(...) print_output(LOG_DEBUG, __FILE__, __LINE__, \
    "\033[37m", "\033[0m", stderr, __VA_ARGS__)
#define log_output(...) print_output(LOG_DEBUG, __FILE__, __LINE__, \
    "", "", __VA_ARGS__)

void print_output(int level, const char *file, int line, const char *head,
    const char *tail, FILE *stream, const char *format, ...);

#endif /* LOGGER_H */
