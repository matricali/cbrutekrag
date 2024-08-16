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
#include <winsock2.h>
#include <ws2tcpip.h>
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif // _MSC_VER
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "iprange.h"
#include "log.h"
#include "target.h"

#ifdef _WIN32
#define close closesocket
#define ssize_t SSIZE_T
#endif

#define MAX_PORT 65535
#define DEFAULT_PORT 22

/**
 * @brief Check if a given port number is valid.
 *
 * @param port The port number to check.
 *
 * @return 1 if the port is valid (between 1 and 65535), otherwise 0.
 */
int btkg_target_port_is_valid(const long port)
{
	return port >= 1 && port <= MAX_PORT;
}

/**
 * @brief Initialize a btkg_target_list_t structure.
 *
 * @param target_list Pointer to the target list to initialize.
 */
void btkg_target_list_init(btkg_target_list_t *target_list)
{
	target_list->length = 0;
	target_list->targets = NULL;
}

/**
 * @brief Allocate and initialize a new btkg_target_list_t structure.
 *
 * @return Pointer to the newly allocated btkg_target_list_t structure,
 *         or NULL if the allocation fails.
 */
btkg_target_list_t *btkg_target_list_create(void)
{
	btkg_target_list_t *targets = malloc(sizeof(btkg_target_list_t));

	if (targets != NULL)
		btkg_target_list_init(targets);

	return targets;
}

/**
 * @brief Destroy and free a btkg_target_list_t structure.
 *
 * @param target_list Pointer to the target list to destroy.
 */
void btkg_target_list_destroy(btkg_target_list_t *target_list)
{
	if (target_list == NULL) {
		return;
	}

	for (size_t i = 0; i < target_list->length; i++) {
		if (target_list->targets[i].host != NULL) {
			free(target_list->targets[i].host);
		}
	}

	free(target_list->targets);
	target_list->length = 0;
	target_list->targets = NULL;
}

/**
 * @brief Parse a target string and create a btkg_target_t structure.
 *
 * @param line The target string (e.g., "192.168.1.1:8080").
 *
 * @return Pointer to the newly created btkg_target_t structure,
 *         or NULL if the string is invalid.
 *
 * @note Exits the program with an error message if memory allocation fails.
 */
btkg_target_t *btkg_target_parse(char *line)
{
	btkg_target_t *ret = malloc(sizeof(btkg_target_t));
	if (ret == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	ret->host = NULL;
	ret->port = DEFAULT_PORT;

	char *ptr = strtok(line, ":");

	if (ptr != NULL) {
		ret->host = strdup(ptr);
		if (ret->host == NULL) {
			perror("strdup");
			free(ret);
			exit(EXIT_FAILURE);
		}

		ptr = strtok(NULL, ":");
		if (ptr != NULL) {
			char *endptr;
			long port = strtol(ptr, &endptr, 10);
			if (*endptr != '\0' ||
			    !btkg_target_port_is_valid(port)) {
				log_error("WARNING: Invalid port (%s)", ptr);
				free(ret->host);
				free(ret);
				return NULL;
			}
			ret->port = (uint16_t)port;
		}
	}

	return ret;
}

/**
 * @brief Append a btkg_target_t structure to a btkg_target_list_t.
 *
 * @param target_list Pointer to the target list.
 * @param target Pointer to the target to append.
 */
void btkg_target_list_append(btkg_target_list_t *target_list,
			     btkg_target_t *target)
{
	btkg_target_t *new_targets =
		realloc(target_list->targets,
			sizeof(btkg_target_t) * (target_list->length + 1));

	if (new_targets == NULL) {
		perror("realloc");
		exit(EXIT_FAILURE);
	}

	target_list->targets = new_targets;
	target_list->targets[target_list->length] = *target;
	target_list->length++;
}

/**
 * @brief Parse a CIDR block and append all addresses in the range as
 *        btkg_target_t structures to the given btkg_target_list_t.
 *
 * @param target_list Pointer to the target list.
 * @param range The CIDR block string (e.g., "192.168.1.0/24").
 * @param port The port number to use for all targets.
 */
void btkg_target_list_append_range(btkg_target_list_t *target_list,
				   const char *range, uint16_t port)
{
	char *netmask_s = strchr(range, '/');

	if (netmask_s == NULL) {
		btkg_target_t *target = malloc(sizeof(btkg_target_t));
		if (target == NULL) {
			perror("malloc");
			exit(EXIT_FAILURE);
		}
		target->host = strdup(range);
		if (target->host == NULL) {
			perror("strdup");
			free(target);
			exit(EXIT_FAILURE);
		}
		target->port = port;
		btkg_target_list_append(target_list, target);
		free(target);
		return;
	}

	struct in_addr in;
	in_addr_t lo, hi;
	network_addr_t netaddr = str_to_netaddr(range);
	lo = netaddr.addr;
	hi = broadcast(netaddr.addr, netaddr.pfx);

	for (in_addr_t x = lo; x < hi; x++) {
		in.s_addr = htonl(x);
		btkg_target_t *new_target = malloc(sizeof(btkg_target_t));
		if (new_target == NULL) {
			perror("malloc");
			exit(EXIT_FAILURE);
		}
		new_target->host = strdup(inet_ntoa(in));
		if (new_target->host == NULL) {
			perror("strdup");
			free(new_target);
			exit(EXIT_FAILURE);
		}
		new_target->port = port;
		btkg_target_list_append(target_list, new_target);
		free(new_target);
	}
}

/**
 * @brief Load targets from a given file and append them into the
 *        given btkg_target_list_t.
 *
 * @param target_list Pointer to the target list.
 * @param filename The path to the file containing target definitions.
 */
void btkg_target_list_load(btkg_target_list_t *target_list,
			   const char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		perror("fopen");
		log_error("Error opening file. (%s)", filename);
		exit(EXIT_FAILURE);
	}

	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int lines = 0;

	while ((read = getline(&line, &len, fp)) != -1) {
		strtok(line, "\n\r");
		btkg_target_t *ret = btkg_target_parse(line);

		if (ret == NULL) {
			log_error(
				"WARNING: An error occurred parsing '%s' on line #%d",
				filename, lines);
		} else {
			btkg_target_list_append_range(target_list, ret->host,
						      ret->port);
		}

		lines++;
	}

	free(line);
	fclose(fp);
}
