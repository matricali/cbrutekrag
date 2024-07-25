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

#ifndef TARGET_H
#define TARGET_H

#include <stdint.h>

/**
 * @brief Represents a network target with a hostname or IP address
 *        and a port number.
 */
typedef struct {
	char *host;
	uint16_t port;
} btkg_target_t;

/**
 * @brief Represents a list of network targets.
 */
typedef struct {
	size_t length;
	btkg_target_t *targets;
} btkg_target_list_t;

/**
 * @brief Check if a given port number is valid.
 *
 * @param port The port number to check.
 *
 * @return 1 if the port is valid (between 1 and 65535), otherwise 0.
 */
int btkg_target_port_is_valid(const long port);

/**
 * @brief Allocate and initialize a new btkg_target_list_t structure.
 *
 * @return Pointer to the newly allocated btkg_target_list_t structure,
 *         or NULL if the allocation fails.
 */
btkg_target_list_t *btkg_target_list_create(void);

/**
 * @brief Destroy and free a btkg_target_list_t structure.
 *
 * @param target_list Pointer to the target list to destroy.
 */
void btkg_target_list_destroy(btkg_target_list_t *target_list);

/**
 * @brief Initialize a btkg_target_list_t structure.
 *
 * @param target_list Pointer to the target list to initialize.
 */
void btkg_target_list_init(btkg_target_list_t *target_list);

/**
 * @brief Append a btkg_target_t structure to a btkg_target_list_t.
 *
 * @param target_list Pointer to the target list.
 * @param target Pointer to the target to append.
 */
void btkg_target_list_append(btkg_target_list_t *target_list,
			     btkg_target_t *target);

/**
 * @brief Parse a CIDR block and append all addresses in the range as
 *        btkg_target_t structures to the given btkg_target_list_t.
 *
 * @param target_list Pointer to the target list.
 * @param range The CIDR block string (e.g., "192.168.1.0/24").
 * @param port The port number to use for all targets.
 */
void btkg_target_list_append_range(btkg_target_list_t *target_list,
				   const char *range, uint16_t port);

/**
 * @brief Load targets from a given file and append them into the
 *        given btkg_target_list_t.
 *
 * @param target_list Pointer to the target list.
 * @param filename The path to the file containing target definitions.
 */
void btkg_target_list_load(btkg_target_list_t *target_list,
			   const char *filename);

/**
 * @brief Parse a target string and create a btkg_target_t structure.
 *
 * @param line The target string (e.g., "192.168.1.1:8080").
 *
 * @return Pointer to the newly created btkg_target_t structure,
 *         or NULL if the string is invalid.
 */
btkg_target_t *btkg_target_parse(char *line);

#endif // TARGET_H
