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

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <string.h>

/**
 * Get the terminal width
 * @return The width of the terminal
 */
static size_t get_terminal_width(void)
{
	size_t max_cols = 80; // Default width if we can't get the size

#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
				       &csbi)) {
		max_cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	}
#else
	struct winsize w;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
		max_cols = w.ws_col;
	}
#endif

	return max_cols;
}

/**
 * Render progress bar like this:
 * |███░░░░░░░░░░░░░░░░░░░░░|  14.51%   [37] 192.168.100.37 root root
 *
 * @author Jorge Matricali <jorgematricali@gmail.com>
 * @param count   Current iteration
 * @param total   Total iterations
 * @param suffix  String suffix
 * @param bar_len Bar length
 */
void progressbar_render(size_t count, size_t total, const char *suffix,
			size_t bar_len)
{
	size_t max_cols = get_terminal_width();

	if (bar_len == 0)
		bar_len = max_cols - 80;
	if (suffix == NULL)
		suffix = "";

	size_t filled_len = 0;
	double percentage = 0;

	if (total > 0) {
		filled_len = bar_len * count / total;
		percentage = ((double)count / (double)total) * 100.0;
	}

	size_t empty_len = bar_len - filled_len;
	size_t fill = max_cols - bar_len - strlen(suffix) - 16;

	printf("\b%c[2K\r", 27);
	if (bar_len > 0) {
		printf("\033[37m|");
		if (filled_len > 0) {
			printf("\033[32m");
			for (size_t i = 0; i < filled_len; ++i) {
#ifdef _WIN32
				printf("#");
#else
				printf("\u2588");
#endif
			}
		}
		if (empty_len > 0) {
			printf("\033[37m");
			for (size_t i = 0; i < empty_len; ++i) {
#ifdef _WIN32
				printf("-");
#else
				printf("\u2591");
#endif
			}
		}
		printf("\033[37m|\033[0m");
	}

	if (max_cols > 60)
		printf("  %.2f%%   %s", percentage, suffix);

	if (fill > 0) {
		for (size_t i = 0; i < fill; ++i) {
			printf(" ");
		}
	}

	printf("\r");
	fflush(stdout);
}
