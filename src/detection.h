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

#ifndef DETECTION_H
#define DETECTION_H

#include <stdint.h>

#include "cbrutekrag.h"
#include "target.h"

/**
 * Structure to hold the arguments for detection threads.
 */
typedef struct {
	btkg_context_t *context;
	btkg_target_list_t *target_list;
} btkg_detection_args_t;

/**
 * Detect if the SSH service is running and if it supports
 * password authentication.
 *
 * This function connects to an SSH server and checks its banner and
 * authentication methods to determine if it's a valid SSH service and if
 * it supports password authentication.
 *
 * @param ctx       The context containing options for detection.
 * @param hostname  The hostname of the SSH server.
 * @param port      The port of the SSH server.
 * @param tm        The timeout for the SSH connection.
 *
 * @return 0 if the server is a valid SSH server with password authentication,
 * 				 1 if there is a connection error, -1 if the server does not meet
 *			   the criteria.
 */
int detection_detect_ssh(btkg_context_t *ctx, const char *hostname,
			 uint16_t port, long tm);

/**
 * Thread function to process detection for each target.
 *
 * This function processes targets from the target list, detects SSH services
 * and their authentication methods, and updates the filtered target list.
 *
 * @param ptr A pointer to a btkg_detection_args_t structure containing the
 *            context and target list.
 *
 * @return NULL
 */
void *detection_process(void *ptr);

/**
 * Start the detection process with multiple threads.
 *
 * This function initializes and starts multiple threads to process the target
 * list and detect SSH services. It waits for all threads to complete before
 * updating the target list with the results.
 *
 * @param context  The context containing options for the detection.
 * @param source   The source target list to process.
 * @param target   The target list to store the results.
 * @param max_threads The maximum number of threads to use.
 */
void detection_start(btkg_context_t *context, btkg_target_list_t *source,
		     btkg_target_list_t *target, size_t max_threads);

#endif // DETECTION_H
