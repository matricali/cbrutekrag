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

#include "cbrutekrag.h"

/**
 * @brief Initializes the options structure with default values.
 *
 * This function sets the default values for the options used in the brute
 * force tool.
 *
 * @param options Pointer to the options structure to be initialized.
 */
void btkg_options_init(btkg_options_t *options)
{
	if (options == NULL)
		return;

	options->timeout = 3;
	options->max_threads = 1;
	options->progress_bar = 0;
	options->verbose = 0;
	options->dry_run = 0;
	options->perform_scan = 0;
	options->non_openssh = 0;
	options->allow_honeypots = 0;
}

/**
 * @brief Initializes the context structure with default values.
 *
 * This function initializes the context structure, including its embedded
 * options, credentials list, and target list, with default values.
 *
 * @param context Pointer to the context structure to be initialized.
 */
void btkg_context_init(btkg_context_t *context)
{
	if (context == NULL) {
		return;
	}

	btkg_options_init(&context->options);

	context->output = NULL;
	context->count = 0;
	context->successful = 0;
	context->total = 0;
	context->credentials_idx = 0;
	context->targets_idx = 0;

	btkg_credentials_list_init(&context->credentials);
	btkg_target_list_init(&context->targets);
}
