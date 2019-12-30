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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iprange.h"
#include "log.h"
#include "wordlist.h"

wordlist_t wordlist_load(char *filename)
{
	FILE *fp;
	wordlist_t ret;
	ssize_t read;
	char *temp = 0;
	size_t len;

	/* Initialize wordlist */
	ret.length = 0;
	ret.words = NULL;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		log_error("Error opening file. (%s)", filename);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; (read = getline(&temp, &len, fp)) != -1; i++) {
		strtok(temp, "\n");
		wordlist_append(&ret, temp);
	}
	fclose(fp);

	if (temp != NULL) {
		free(temp);
	}

	return ret;
}

int wordlist_append(wordlist_t *wordlist, const char *string)
{
	char **words = wordlist->words;

	if (words == NULL) {
		words = malloc(sizeof(string));
		*words = strdup(string);
	} else {
		words = realloc(words, sizeof(string) * (wordlist->length + 1));
		*(words + wordlist->length) = strdup(string);
	}

	wordlist->length = wordlist->length + 1;
	wordlist->words = words;

	return 0;
}

int wordlist_append_range(wordlist_t *wordlist, const char *range)
{
	char *netmask_s;
	netmask_s = strchr(range, '/');

	if (netmask_s == NULL) {
		wordlist_append(wordlist, range);
		return 0;
	}

	struct in_addr in;
	in_addr_t lo, hi;
	network_addr_t netaddr;

	netaddr = str_to_netaddr(range);
	lo = netaddr.addr;
	hi = broadcast(netaddr.addr, netaddr.pfx);

	for (in_addr_t x = lo; x < hi; x++) {
		in.s_addr = htonl(x);
		wordlist_append(wordlist, inet_ntoa(in));
	}
	return 0;
}

int wordlist_append_from_file(wordlist_t *wordlist, char *filename)
{
	FILE *fp;
	ssize_t read;
	char *temp = 0;
	size_t len;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		log_error("Error opening file. (%s)", filename);
		return 1;
	}
	for (int i = 0; (read = getline(&temp, &len, fp)) != -1; i++) {
		strtok(temp, "\n");
		wordlist_append_range(wordlist, temp);
	}
	fclose(fp);
	return 0;
}

void wordlist_destroy(wordlist_t *wordlist)
{
	for (int i = 0; i < wordlist->length; ++i) {
		free(wordlist->words[i]);
	}
	free(wordlist->words);
}
