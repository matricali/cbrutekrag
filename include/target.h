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

#ifndef TARGET_H
#define TARGET_H

typedef struct {
	char *host;
	uint16_t port;
} btkg_target_t;

typedef struct {
	size_t length;
	btkg_target_t **targets;
} btkg_target_list_t;

int btkg_target_port_is_valid(const long port);

void btkg_target_list_init(btkg_target_list_t *target_list);

void btkg_target_list_append(btkg_target_list_t *target_list,
			     btkg_target_t *target);

void btkg_target_list_append_range(btkg_target_list_t *target_list,
				   const char *range, uint16_t port);

void btkg_target_list_load(btkg_target_list_t *target_list, char *filename);

btkg_target_t *target_parse(char *line);

/**
 * Allocate btkg_target_t
 */
btkg_target_t *btkg_target_create(void);

/**
 * Destroy btkg_target_t
 */
void btkg_target_destroy(btkg_target_t *target);

#endif /* TARGET_H */
