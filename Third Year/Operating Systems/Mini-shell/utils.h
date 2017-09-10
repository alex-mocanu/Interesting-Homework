/**
 * Operating Systems 2013-2017 - Assignment 2
 *
 */

#ifndef _UTILS_H
#define _UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <unistd.h>

#include "parser.h"


/* useful macro for handling error codes */
#define DIE(assertion, call_description) \
	do { \
		if (assertion) { \
			fprintf(stderr, "(%s, %s, %d): ", \
				__FILE__, __FUNCTION__, __LINE__); \
			perror(call_description); \
			exit(EXIT_FAILURE); \
		} \
	} while (0)

/**
 * Concatenate parts of the word to obtain the command
 */
char *get_word(word_t *s);

/**
 * Concatenate command arguments in a NULL terminated list in order to pass
 * them directly to execv.
 */
char **get_argv(simple_command_t *command, int *size);

/**
 * Store the state of standard input, output and error
 */
int store_std(int *save_stdin, int *save_stdout, int *save_stderr);

/**
 * Restore the state of standard input, output and error
 */
int restore_std(int save_stdin, int save_stdout, int save_stderr);

/**
 * Treat redirections
 */
int treat_redirect(simple_command_t *s);

#endif /* _UTILS_H */
