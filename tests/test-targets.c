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

#include "../src/compat.h"
#include "../src/target.h"

int g_verbose = 0;
char *g_output_format = NULL;

typedef struct {
	const char *input;
	const char *hostname;
	uint16_t port;
} test_input_t;

enum {
	TEST_PASSED = 0,
	TEST_FAIL_PARSE = 1,
	TEST_FAIL_HOSTNAME = 2,
	TEST_FAIL_PORT = 3
};

static int test_btkg_target_parse(const char *input, const char *hostname,
				  uint16_t port)
{
	char *line = strdup(input);

	printf("Test btkg_credentials_parse('%s') => { '%s', %d }: ", input,
	       hostname, port);

	btkg_target_t target = btkg_target_parse(line);
	free(line);

	if (hostname == NULL && target.host != NULL) {
		printf("FAIL\n-- Expected parsed hostname = NULL, got '%s'\n",
		       target.host);
		return TEST_FAIL_PARSE;
	} else if (hostname != NULL && strcmp(target.host, hostname) != 0) {
		printf("FAIL\n-- Expected parsed hostname = '%s', got '%s'\n",
		       hostname, target.host);
		return TEST_FAIL_HOSTNAME;
	}

	if (target.port != port) {
		printf("FAIL\n-- Expected parsed port = '%d', got '%d'\n", port,
		       target.port);
		return TEST_FAIL_PORT;
	}

	printf("PASSED\n");
	return TEST_PASSED;
}

void run_tests(const test_input_t *tests)
{
	int i = 0;

	while (tests[i].input != NULL) {
		int result = test_btkg_target_parse(
			tests[i].input, tests[i].hostname, tests[i].port);
		if (result != TEST_PASSED) {
			printf("Test %d failed with error code %d\n", i + 1,
			       result);
			exit(result);
		}
		i++;
	}

	printf("All tests passed\n");
}

int main()
{
	printf("Running target.c Test\n");

	test_input_t matrix[] = {
		{ "127.0.0.1", "127.0.0.1",
		  22 }, // Default value por port should be 22
		{ "192.168.0.1", "192.168.0.1", 22 },
		{ "192.168.100.1:2222", "192.168.100.1", 2222 },
		{ "192.168.100.1:22", "192.168.100.1", 22 },
		{ "localhost:22", "localhost", 22 },
		{ "localhost:0", NULL, 22 },
		{ "10.10.10.10:test", NULL, 22 },
		{ "10.10.10.10  :22", NULL, 22 },
		{ NULL, NULL, 0 },
	};

	run_tests(matrix);

	return 0;
}
