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

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <libssh/libssh.h>
#include <pthread.h>

#include "cbrutekrag.h"
#include "log.h"
#include "progressbar.h"

static pthread_mutex_t bflock = PTHREAD_MUTEX_INITIALIZER;

/**
 * Attempt to brute-force SSH login using the provided credentials.
 *
 * This function tries to log in to the SSH server using the specified hostname,
 * port, username, and password. It performs the brute-force attempt and handles
 * any related errors.
 *
 * @param context   The context containing options and configurations for brute-force.
 * @param hostname  The hostname of the SSH server.
 * @param port      The port of the SSH server.
 * @param username  The username to use for the login attempt.
 * @param password  The password to use for the login attempt.
 *
 * @return 0 on success, non-zero on failure.
 */
int bruteforce_ssh_login(btkg_context_t *context, const char *hostname,
			 uint16_t port, const char *username,
			 const char *password)
{
	ssh_session my_ssh_session;
	int verbosity = 0;

	if (context->options.verbose & CBRUTEKRAG_VERBOSE_SSHLIB) {
		verbosity = SSH_LOG_PROTOCOL;
	} else {
		verbosity = SSH_LOG_NOLOG;
	}

	my_ssh_session = ssh_new();

	if (my_ssh_session == NULL) {
		log_error("Cant create SSH session.");
		return -1;
	}

	ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, hostname);
	ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
	ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &(int){port});
#if LIBSSH_VERSION_MAYOR > 0 ||                                                \
	(LIBSSH_VERSION_MAYOR == 0 && LIBSSH_VERSION_MINOR >= 6)
	ssh_options_set(my_ssh_session, SSH_OPTIONS_KEY_EXCHANGE, "none");
	ssh_options_set(my_ssh_session, SSH_OPTIONS_HOSTKEYS, "none");
#endif
	ssh_options_set(my_ssh_session, SSH_OPTIONS_TIMEOUT,
			&context->options.timeout);
	ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, username);

	int r;
	r = ssh_connect(my_ssh_session);
	if (r != SSH_OK) {
		if (context->options.verbose & CBRUTEKRAG_VERBOSE_MODE) {
			log_error("[!] Error connecting to %s:%d %s.", hostname,
				  port, ssh_get_error(my_ssh_session));
		}
		ssh_free(my_ssh_session);
		return -1;
	}

	r = ssh_userauth_none(my_ssh_session, NULL);

	if (r == SSH_AUTH_SUCCESS) {
		log_debug("[!] %s:%d - Server without authentication.",
			  hostname, port);
		ssh_disconnect(my_ssh_session);
		ssh_free(my_ssh_session);

		return -1;
	}

	if (r == SSH_AUTH_ERROR) {
		log_debug(
			"[!] %s:%d - ssh_userauth_none(): A serious error happened.",
			hostname, port);
		ssh_disconnect(my_ssh_session);
		ssh_free(my_ssh_session);

		return -1;
	}

	int method = 0;

	method = ssh_userauth_list(my_ssh_session, NULL);

	if (method & (int)SSH_AUTH_METHOD_PASSWORD) {
		r = ssh_userauth_password(my_ssh_session, NULL, password);
		if (r == SSH_AUTH_SUCCESS) {
			ssh_disconnect(my_ssh_session);
			ssh_free(my_ssh_session);

			return 0;
		}
	}

	ssh_disconnect(my_ssh_session);
	ssh_free(my_ssh_session);
	return -1;
}

/**
 * Try to log in to an SSH server with the specified credentials.
 *
 * This function tries a single login attempt using the provided credentials and
 * returns the result of the attempt.
 *
 * @param context   The context containing options and configurations for brute-force.
 * @param hostname  The hostname of the SSH server.
 * @param port      The port of the SSH server.
 * @param username  The username to use for the login attempt.
 * @param password  The password to use for the login attempt.
 *
 * @return 0 on success, non-zero on failure.
 */
int bruteforce_ssh_try_login(btkg_context_t *context, const char *hostname,
			     const uint16_t port, const char *username,
			     const char *password)
{
	const char *_password =
		strcmp(password, "$TARGET") == 0 ? hostname : password;
	const char *_username =
		strcmp(username, "$TARGET") == 0 ? hostname : username;

	int ret = bruteforce_ssh_login(context, hostname, port, _username,
				       _password);

	if (ret == 0) {
		log_info("\033[32m[+]\033[0m %s:%d %s %s", hostname, port,
			 _username, _password);
		if (context->output != NULL) {
			btkg_log_successfull_login(context->output, hostname,
						   port, _username, _password);
		}
	} else {
		log_debug("\033[38m[-]\033[0m %s:%d %s %s", hostname, port,
			  _username, _password);
	}

	return ret;
}

/**
 * Worker function for brute-force SSH login attempts.
 *
 * This function is executed by each worker thread to perform brute-force SSH login attempts
 * on the specified targets using the provided credentials.
 *
 * @param ptr  Pointer to the context containing targets, credentials, and options.
 *
 * @return NULL when the worker completes its task.
 */
static void *btkg_bruteforce_worker(void *ptr)
{
	btkg_context_t *context = (btkg_context_t *)ptr;
	btkg_target_list_t *targets = &context->targets;
	btkg_credentials_list_t *credentials = &context->credentials;
	btkg_options_t *options = &context->options;

	for (;;) {
		pthread_mutex_lock(&bflock);
		if (context->targets_idx >= targets->length) {
			context->targets_idx = 0;
			context->credentials_idx++;
		}
		if (context->credentials_idx >= credentials->length) {
			// Llegamos al final
			log_debug("No work to do. Stopping thread...");
			pthread_mutex_unlock(&bflock);
			break;
		}

		btkg_target_t *target =
			&targets->targets[context->targets_idx++];
		btkg_credentials_t *combo =
			&credentials->credentials[context->credentials_idx];
		context->count++;

		if (options->progress_bar) {
			char str[40];
			snprintf(str, 40, "[%zu/%zu] %zu OK - %s:%d",
				 context->count, context->total, context->count,
				 target->host, target->port);
			progressbar_render(context->count, context->total, str,
					   0);
		}
		pthread_mutex_unlock(&bflock);

		if (!options->dry_run) {
			int ret = bruteforce_ssh_try_login(
				context, target->host, target->port,
				combo->username, combo->password);
			if (ret == 0) {
				pthread_mutex_lock(&bflock);
				context->successful++;
				pthread_mutex_unlock(&bflock);
			}
		} else {
			log_debug("\033[38m[-]\033[0m %s:%d %s %s",
				  target->host, target->port, combo->username,
				  combo->password);
		}
	}
	pthread_exit(NULL);
	return NULL;
}

/**
 * Start the brute-force SSH login process.
 *
 * This function initializes and starts the brute-force process, including setting
 * up necessary threads and handling the brute-force attack.
 *
 * @param context   The context containing options and configurations for brute-force.
 */
void btkg_bruteforce_start(btkg_context_t *context)
{
	btkg_options_t *options = &context->options;

	pthread_t scan_threads[options->max_threads];
	int ret;

	for (size_t i = 0; i < options->max_threads; i++) {
		log_debug("Creating thread: %ld", i);
		if ((ret = pthread_create(&scan_threads[i], NULL,
					  *btkg_bruteforce_worker,
					  (void *)context))) {
			log_error("Thread creation failed: %d\n", ret);
		}
	}

	for (size_t i = 0; i < options->max_threads; i++) {
		ret = pthread_join(scan_threads[i], NULL);
		if (ret != 0) {
			log_error("Cannot join thread no: %d\n", ret);
		}
	}

	if (options->progress_bar)
		progressbar_render(context->count, context->total, NULL, 0);
}
