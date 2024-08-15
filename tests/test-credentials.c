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

#include "../src/credentials.h"

typedef struct {
	const char *input;
	const char *username;
	const char *password;
} test_input_t;

enum {
	TEST_PASSED = 0,
	TEST_FAIL_PARSE = 1,
	TEST_FAIL_USERNAME = 2,
	TEST_FAIL_PASSWORD = 3
};

static int test_btkg_credentials_parse(const char *input, const char *username,
				       const char *password)
{
	btkg_credentials_t credentials;
	char *line = strdup(input);

	printf("Test btkg_credentials_parse('%s','%s','%s'): ", input, username,
	       password);

	int ret = btkg_credentials_parse(line, &credentials);
	free(line);

	if (ret != 0) {
		printf("FAIL\n-- returns '%d'\n", ret);
		return TEST_FAIL_PARSE;
	}

	if (strcmp(credentials.username, username) != 0) {
		printf("FAIL\n-- Expected parsed username = '%s', got '%s'\n",
		       username, credentials.username);
		return TEST_FAIL_USERNAME;
	}

	if (strcmp(credentials.password, password) != 0) {
		printf("FAIL\n-- Expected parsed password = '%s', got '%s'\n",
		       password, credentials.password);
		return TEST_FAIL_PASSWORD;
	}

	printf("PASSED\n");
	return TEST_PASSED;
}

void run_tests(const test_input_t *tests)
{
	int i = 0;

	while (tests[i].input != NULL) {
		int result = test_btkg_credentials_parse(
			tests[i].input, tests[i].username, tests[i].password);
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
	printf("Running btkg_credentials_parse Test\n");

	test_input_t matrix[] = {
		{ "root password", "root", "password" },
		{ "root $BLANKPASS", "root", "" },
		{ "root", "root", "" },
		{ "root ", "root", "" },
		{ "root      ", "root", "     " },
		{ "admin password with spaces", "admin",
		  "password with spaces" },
		{ NULL, NULL, NULL },
	};

	run_tests(matrix);

	return 0;
}
