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

#include "cbrutekrag.h"
#include <string.h>
#include <time.h>

#include "log.h"
#include "progress.h"
#include "progressbar.h"

#define NANO_PER_SEC 1000000000L

/**
 * @brief Formats time in seconds into hours, minutes, and seconds.
 *
 * This function takes a time value in seconds and breaks it down into
 * hours, minutes, and seconds.
 *
 * @param seconds Time value in seconds to format.
 * @param hours Pointer to store the computed hours.
 * @param minutes Pointer to store the computed minutes.
 * @param secs Pointer to store the computed seconds.
 */
static inline void format_time(double seconds, int *hours, int *minutes,
			       int *secs)
{
	*hours = (int)seconds / 3600;
	*minutes = ((int)seconds % 3600) / 60;
	*secs = (int)seconds % 60;
}

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
double btkg_elapsed_time(struct timespec *start)
{
	struct timespec end;
	clock_gettime(CLOCK_MONOTONIC, &end);
	return (double)(end.tv_sec - start->tv_sec) +
	       ((double)(end.tv_nsec - start->tv_nsec) / NANO_PER_SEC);
}

/**
 * @brief Progress monitoring worker thread function.
 *
 * This function is executed by a separate thread to monitor the progress
 * of a task. It calculates and displays the estimated time remaining (ETR)
 * and the processing rate (items per second). The progress is updated every
 * second, and optionally, a progress bar can be displayed.
 *
 * @param ptr Pointer to the `btkg_progress_worker_args_t` structure containing
 *        context information.
 * @return `NULL` upon completion.
 */
void *progress_worker(void *ptr)
{
	btkg_progress_worker_args_t *args = (btkg_progress_worker_args_t *)ptr;
	btkg_context_t *context = args->context;
	btkg_options_t *options = &context->options;
	time_t start_time = time(NULL);
	time_t last_update_time = 0;
	size_t stats_len = 40;
	char stats[stats_len];

	memset(&stats, 0, stats_len);

	struct timespec last_update = { 0 };
	int suffix_len = 0;

	while (context->count < context->total) {
		time_t current_time = time(NULL);

		if (btkg_elapsed_time(&last_update) < 0.2)
			continue;

		if (difftime(current_time, last_update_time) >=
		    1.0) { // Update ETR every 1 sec
			double elapsed_time =
				difftime(current_time, start_time);

			if (context->count > 0 && elapsed_time > 0) {
				double rate =
					(double)context->count / elapsed_time;
				double remaining_time =
					((double)(context->total -
						  context->count)) /
					rate;

				int hours, minutes, seconds;
				format_time(remaining_time, &hours, &minutes,
					    &seconds);

				suffix_len =
					snprintf(
						stats, stats_len,
						"ETR: %02d:%02d:%02d Rate: %.0f/sec",
						hours, minutes, seconds, rate) +
					1;
			}

			// Actualizar el tiempo de la última llamada
			last_update_time = current_time;

			if (!options->progress_bar && stats[0] != '\0')
				log_info("%s", stats);
		}

		clock_gettime(CLOCK_MONOTONIC, &last_update);

		if (options->progress_bar)
			progressbar_render(context->count, context->total,
					   stats, (size_t)suffix_len);
	}

	free(args);
	return NULL;
}

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
void btkg_progress_watcher_start(btkg_context_t *context, pthread_t *thread)
{
	int ret;
	btkg_progress_worker_args_t *args =
		malloc(sizeof(btkg_progress_worker_args_t));

	if (args == NULL) {
		log_error("Cannot allocate progress watcher thread");
		return;
	}

	args->context = context;

	if ((ret = pthread_create(thread, NULL, progress_worker,
				  (void *)args))) {
		log_error("Thread creation failed: %d\n", ret);
		free(args);
	}
}

/**
 * @brief Waits for the progress monitoring thread to complete.
 *
 * This function blocks until the progress monitoring thread has completed
 * its execution.
 *
 * @param thread Pointer to the `pthread_t` structure holding the thread
 *        identifier.
 */
void btkg_progress_watcher_wait(pthread_t *thread)
{
	int ret = pthread_join(*thread, NULL);
	if (ret != 0) {
		log_error("Cannot join thread no: %d\n", ret);
	}
}
