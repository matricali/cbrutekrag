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
#include <limits.h>

/**
 * Get the terminal width.
 *
 * This function retrieves the width of the terminal in columns. If the
 * width cannot be determined, a default value of 80 columns is used.
 *
 * @return The width of the terminal in columns.
 */
static size_t get_terminal_width(void)
{
	size_t max_cols = 80; // Default width if we can't get the size

#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
				       &csbi)) {
		max_cols =
			(size_t)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
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
 * Render a progress bar to the terminal.
 *
 * This function displays a progress bar with a filled and empty section
 * to represent progress, as well as a percentage and optional suffix.
 * The bar is rendered to fit within the width of the terminal.
 *
 * |███░░░░░░░░░░░░░░░░░░░░░|  14.51%   [37] 192.168.100.37 root root
 *
 * @param count   The current iteration count.
 * @param total   The total number of iterations.
 * @param suffix  A string suffix to append after the progress bar.
 * @param suffix_max The width for suffix.
 */
void progressbar_render(size_t count, size_t total, const char *suffix,
			size_t suffix_max)
{
	size_t max_cols = get_terminal_width();
	size_t parcentage_len = 12; // " %6.2f%%  " + ||

	if (suffix_max + parcentage_len > max_cols)
		suffix_max = max_cols - parcentage_len;

	size_t bar_len = max_cols - parcentage_len - suffix_max;

	if (suffix == NULL)
		suffix = "";

	double percentage = 0;
	size_t filled_len = 0;

	if (total > 0) {
		filled_len = bar_len * count / total;
		percentage = ((double)count / (double)total) * 100.0;
	}

	printf("\b%c[2K\r", 27);

	if (bar_len > 0) {
		printf("\033[37m|");
		if (filled_len > 0) {
			printf("\033[32m");
			for (size_t i = 0; i < filled_len; ++i) {
				printf("\u2588");
			}
		}
		size_t empty_len =
			bar_len > filled_len ? bar_len - filled_len : 0;
		if (empty_len > 0) {
			printf("\033[37m");
			for (size_t i = 0; i < empty_len; ++i) {
				printf("\u2591");
			}
		}
		printf("\033[37m|\033[0m");
	}

	printf(" %6.2f%%  %-*s", percentage,
	       suffix_max > INT_MAX ? INT_MAX : (int)suffix_max, suffix);

	printf("\r");
	fflush(stdout);
}
