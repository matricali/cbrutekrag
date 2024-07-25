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

#ifndef BRUTEFORCE_SSH_H
#define BRUTEFORCE_SSH_H

#include <stdint.h>

#include "cbrutekrag.h"

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
			 const char *password);

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
			     const char *password);

/**
 * Start the brute-force SSH login process.
 *
 * This function initializes and starts the brute-force process, including setting
 * up necessary threads and handling the brute-force attack.
 *
 * @param context   The context containing options and configurations for brute-force.
 */
void btkg_bruteforce_start(btkg_context_t *context);

#endif // BRUTEFORCE_SSH_H
