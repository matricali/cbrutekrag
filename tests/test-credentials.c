#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../src/compat.h"
#include "../src/credentials.h"

int g_verbose = 0;
char *g_output_format = NULL;

typedef struct {
	const char *input;
	const char *username;
	const char *password;
} test_input_t;

static int test_btkg_credentials_parse(const char *input, const char *username,
				       const char *password)
{
	btkg_credentials_t credentials;
	char *line = strdup(input);

	printf("Test btkg_credentials_parse('%s','%s','%s'): ", input, username,
	       password);

	int ret = btkg_credentials_parse(line, &credentials);
	if (ret != 0) {
		printf("FAIL\n-- returns '%d'\n", ret);
		return 1;
	}

	if (strcmp(credentials.username, username) != 0) {
		printf("FAIL\n-- Expected parsed username = '%s', got '%s'\n",
		       username, credentials.username);
		return 2;
	}

	if (strcmp(credentials.password, password) != 0) {
		printf("FAIL\n-- Expected parsed password = '%s', got '%s'\n",
		       password, credentials.password);
		return 3;
	}

	printf("PASSED\n");

	return 0;
}

int main()
{
	printf("Running Targets Test\n");

	test_input_t matrix[] = {
		{"root password", "root", "password"},
		{"root $BLANKPASS", "root", ""},
		{"root", "root", ""},
		{"root ", "root", ""},
		{"root      ", "root", "     "},
		{"admin password with spaces", "admin", "password with spaces"},
		{NULL, NULL, NULL},
	};

	int i = 0;

	 while (matrix[i].input != NULL) {
		 if (test_btkg_credentials_parse(matrix[i].input, matrix[i].username, matrix[i].password) != 0) {
			 return i+1;
		 }
		 i++;
	 }

	printf("Test passed\n");

	return 0;
}
