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

#ifndef CBRUTEKRAG_H
#define CBRUTEKRAG_H

#include <stdio.h>

#include "credentials.h"
#include "target.h"

#define CBRUTEKRAG_VERBOSE_MODE 0x1
#define CBRUTEKRAG_VERBOSE_SSHLIB 0x2

typedef struct {
	int timeout;
	size_t max_threads;
	int progress_bar;
	int verbose;
	int dry_run;
	int perform_scan;
	int non_openssh;
	int allow_honeypots;
	char *check_http;
} btkg_options_t;

typedef struct {
	btkg_options_t options;
	btkg_credentials_list_t credentials;
	btkg_target_list_t targets;
	size_t count;
	size_t successful;
	size_t total;
	size_t credentials_idx;
	size_t targets_idx;
	FILE *output;
	FILE *scan_output;
} btkg_context_t;

/**
 * @brief Initializes the options structure with default values.
 *
 * This function sets the default values for the options used in the brute
 * force tool.
 *
 * @param options Pointer to the options structure to be initialized.
 */
void btkg_options_init(btkg_options_t *options);

/**
 * @brief Initializes the context structure with default values.
 *
 * This function initializes the context structure, including its embedded
 * options, credentials list, and target list, with default values.
 *
 * @param context Pointer to the context structure to be initialized.
 */
void btkg_context_init(btkg_context_t *context);

#endif /* CBRUTEKRAG_H */
