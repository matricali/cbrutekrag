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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/target.h"

char *g_output_format = NULL;
char *g_scan_output_format = NULL;

typedef struct {
	const char *input;
	btkg_target_t *expected;
} test_input_t;

enum {
	TEST_PASSED = 0,
	TEST_FAIL_PARSE = 1,
	TEST_FAIL_HOSTNAME = 2,
	TEST_FAIL_PORT = 3
};

static int test_btkg_target_parse(const char *input, btkg_target_t *expected)
{
	char *line = strdup(input);

	if (expected == NULL) {
		printf("Test btkg_credentials_parse('%s') => NULL: ", input);
	} else {
		printf("Test btkg_credentials_parse('%s') => { '%s', %d }: ",
		       input, expected->host, expected->port);
	}

	btkg_target_t *target = btkg_target_parse(line);
	free(line);

	if (expected == NULL) {
		if (target != NULL) {
			printf("FAIL\n-- Expected parsed target = NULL, got '{ %s, %d }'\n",
			       target->host, target->port);
			free(target->host);
			free(target);
			return TEST_FAIL_PARSE;
		}
	} else {
		if (target == NULL) {
			printf("FAIL\n-- Expected parsed target = { '%s', %d }, got NULL\n",
			       expected->host, expected->port);
			return TEST_FAIL_PARSE;
		}

		if (strcmp(target->host, expected->host) != 0) {
			printf("FAIL\n-- Expected parsed hostname = '%s', got '%s'\n",
			       expected->host, target->host);
			free(target->host);
			free(target);
			return TEST_FAIL_HOSTNAME;
		}

		if (target->port != expected->port) {
			printf("FAIL\n-- Expected parsed port = '%d', got '%d'\n",
			       expected->port, target->port);
			free(target->host);
			free(target);
			return TEST_FAIL_PORT;
		}
	}

	printf("PASSED\n");
	return TEST_PASSED;
}

void run_tests(test_input_t *tests)
{
	int i = 0;

	while (tests[i].input != NULL) {
		int result = test_btkg_target_parse(tests[i].input,
						    tests[i].expected);
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
		// Default value por port should be 22
		{ "127.0.0.1", &(btkg_target_t){ "127.0.0.1", 22 } },
		{ "192.168.0.1", &(btkg_target_t){ "192.168.0.1", 22 } },
		// Custom ports
		{ "192.168.100.1:2222",
		  &(btkg_target_t){ "192.168.100.1", 2222 } },
		{ "192.168.100.1:22", &(btkg_target_t){ "192.168.100.1", 22 } },
		{ "localhost:22", &(btkg_target_t){ "localhost", 22 } },
		// Invalid
		{ "localhost:0", NULL },
		{ "10.10.10.10:test", NULL },
		{ "10.10.10.10  :22", NULL },
		{ NULL, NULL },
	};

	run_tests(matrix);

	return 0;
}
