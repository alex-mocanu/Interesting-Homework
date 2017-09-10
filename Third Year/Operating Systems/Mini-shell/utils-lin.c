/**
 * Operating Systems 2013-2017 - Assignment 2
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

/**
 * Concatenate parts of the word to obtain the command
 */
char *get_word(word_t *s)
{
	int string_length = 0;
	int substring_length = 0;

	char *string = NULL;
	char *substring = NULL;

	while (s != NULL) {
		substring = strdup(s->string);

		if (substring == NULL)
			return NULL;

		if (s->expand == true) {
			char *aux = substring;

			substring = getenv(substring);

			/* prevents strlen from failing */
			if (substring == NULL)
				substring = "";

			free(aux);
		}

		substring_length = strlen(substring);

		string = realloc(string, string_length + substring_length + 1);
		if (string == NULL) {
			if (s->expand == false)
				free(substring);
			return NULL;
		}

		memset(string + string_length, 0, substring_length + 1);

		strcat(string, substring);
		string_length += substring_length;

		if (s->expand == false)
			free(substring);

		s = s->next_part;
	}

	return string;
}

/**
 * Concatenate command arguments in a NULL terminated list in order to pass
 * them directly to execv.
 */
char **get_argv(simple_command_t *command, int *size)
{
	char **argv;
	word_t *param;

	int argc = 0;

	argv = calloc(argc + 1, sizeof(char *));
	DIE(argv == NULL, "Error allocating argv.");

	argv[argc] = get_word(command->verb);
	DIE(argv[argc] == NULL, "Error retrieving word.");

	argc++;

	param = command->params;
	while (param != NULL) {
		argv = realloc(argv, (argc + 1) * sizeof(char *));
		DIE(argv == NULL, "Error reallocating argv.");

		argv[argc] = get_word(param);
		DIE(argv[argc] == NULL, "Error retrieving word.");

		param = param->next_word;
		argc++;
	}

	argv = realloc(argv, (argc + 1) * sizeof(char *));
	DIE(argv == NULL, "Error reallocating argv.");

	argv[argc] = NULL;
	*size = argc;

	return argv;
}

/**
 * Store the state of standard input, output and error
 */
int store_std(int *save_stdin, int *save_stdout, int *save_stderr)
{
	*save_stdin = dup(STDIN_FILENO);
	if (*save_stdin < 0)
		return -1;
	*save_stdout = dup(STDIN_FILENO);
	if (*save_stdout < 0) {
		close(*save_stdin);
		return -1;
	}
	*save_stderr = dup(STDIN_FILENO);
	if (*save_stderr < 0) {
		close(*save_stdin);
		close(*save_stdout);
		return -1;
	}
	return 0;
}

/**
 * Restore the state of standard input, output and error
 */
int restore_std(int save_stdin, int save_stdout, int save_stderr)
{
	if (dup2(save_stdin, STDIN_FILENO) < 0 ||
	dup2(save_stdout, STDOUT_FILENO) < 0 ||
	dup2(save_stderr, STDERR_FILENO) < 0) {
		close(save_stdin);
		close(save_stdout);
		close(save_stderr);
		return -1;
	}
	close(save_stdin);
	close(save_stdout);
	close(save_stderr);
	return 0;
}

/**
 * Treat redirections
 */
int treat_redirect(simple_command_t *s)
{
	int fd;

	if (s->err) {
		/* Open file in regular or append mode */
		if (s->io_flags == IO_REGULAR)
			fd = open(get_word(s->err), O_WRONLY | O_TRUNC |
				O_CREAT, 0644);
		else
			fd = open(get_word(s->err), O_WRONLY | O_APPEND |
				O_CREAT, 0644);

		if (fd < 0) {
			fprintf(stderr, "Could not open file for redirect\n");
			return -1;
		}

		if (dup2(fd, STDERR_FILENO) < 0) {
			fprintf(stderr, "Could not redirect stderr\n");
			close(fd);
			return -1;
		}

		/* If stdout is redirected in the same file */
		if (s->err == s->out) {
			if (dup2(fd, STDOUT_FILENO) < 0) {
				fprintf(stderr, "Could not redirect stdout\n");
				close(fd);
				return -1;
			}
		}
		close(fd);
	}
	if (s->out && s->err != s->out) {
		if (s->io_flags == IO_REGULAR)
			fd = open(get_word(s->out), O_WRONLY | O_TRUNC |
				O_CREAT, 0644);
		else
			fd = open(get_word(s->out), O_WRONLY | O_APPEND |
				O_CREAT, 0644);

		if (fd < 0) {
			fprintf(stderr, "Could not open file for redirect\n");
			return -1;
		}

		if (dup2(fd, STDOUT_FILENO) < 0) {
			fprintf(stderr, "Could not redirect stdout\n");
			close(fd);
			return -1;
		}
		close(fd);
	}
	if (s->in) {
		fd = open(get_word(s->in), O_RDONLY);
		if (fd < 0) {
			fprintf(stderr,
				"bash: %s: No such file or directory\n",
				s->in->string);
			return -1;
		}

		if (dup2(fd, STDIN_FILENO) < 0) {
			fprintf(stderr, "Could not redirect stdin\n");
			close(fd);
			return -1;
		}
		close(fd);
	}

	return 0;
}
