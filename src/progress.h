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

#ifndef PROGRESS_H
#define PROGRESS_H

#include "cbrutekrag.h"

typedef struct {
	btkg_context_t *context;
} btkg_progress_worker_args_t;

/**
* @brief Calculates the elapsed time since a given start time.
*
* This function calculates the time elapsed since a given start time
* using the `CLOCK_MONOTONIC` clock, which is unaffected by system time
* changes.
*
* @param start Pointer to the starting time.
* @return The elapsed time in seconds.
*/
double btkg_elapsed_time(struct timespec *start);

/**
 * @brief Starts the progress monitoring thread.
 *
 * This function creates and starts a new thread that runs the
 * `progress_worker` function to monitor the progress of a task.
 *
 * @param context Pointer to the `btkg_context_t` structure containing the
 *        context information.
 * @param thread Pointer to a `pthread_t` structure that will hold the thread
 *        identifier.
 */
void btkg_progress_watcher_start(btkg_context_t *context, pthread_t *thread);

/**
 * @brief Waits for the progress monitoring thread to complete.
 *
 * This function blocks until the progress monitoring thread has completed
 * its execution.
 *
 * @param thread Pointer to the `pthread_t` structure holding the thread
 *        identifier.
 */
void btkg_progress_watcher_wait(pthread_t *thread);

#endif // PROGRESS_H
