/*
Copyright (c) 2024 Jorge Matricali

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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/progressbar.h"

typedef struct {
	size_t total;
	size_t size;
} test_input_t;

void run_tests(test_input_t *tests)
{
	int i = 0;

	// Test matrix
	while (!(tests[i].total == 0 && tests[i].size == 0)) {
		for (int j = 0; j <= tests[i].total; j++) {
			progressbar_render(j, tests[i].total, NULL, tests[i].size);
		}
		printf("\n");
		i++;
	}

	// Test empty bar
	progressbar_render(0, 0, NULL, 0);
	printf("\n");

	printf("All tests passed\n");
}

int main()
{
	printf("Running progressbar.c Test\n");

	test_input_t matrix[] = {
		{10, 0},
		{100, 50},
		{150, 80},
		{0, 50},
		{50, 0},
		{0, 0},
	};

	run_tests(matrix);

	return 0;
}
