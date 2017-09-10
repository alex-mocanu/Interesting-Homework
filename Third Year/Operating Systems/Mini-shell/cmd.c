/**
 * Operating Systems 2013-2017 - Assignment 2
 *
 * Mocanu Alexandru
 * 331CB
 *
 */
#include "cmd.h"
#include "utils.h"

#define READ		0
#define WRITE		1

/**
 * Internal change-directory command.
 */
static bool shell_cd(word_t *dir)
{
	/* execute cd */
	if (chdir(dir->string) == 0)
		return true;
	return false;
}

/**
 * Internal exit/quit command.
 */
static int shell_exit(void)
{
	return SHELL_EXIT;
}

/**
 * Parse a simple command (internal, environment variable assignment,
 * external command).
 */
static int parse_simple(simple_command_t *s, int level, command_t *father)
{
	int pid, size, status;
	int save_stdin, save_stdout, save_stderr;
	char **args;

	/* builtin command */
	/* Exit the shell */
	if (strcmp(s->verb->string, "exit") == 0 ||
	strcmp(s->verb->string, "quit") == 0) {
		if (store_std(&save_stdin, &save_stdout, &save_stderr) < 0)
			return EXIT_FAILURE;
		if (treat_redirect(s) < 0) {
			restore_std(save_stdin, save_stdout, save_stderr);
			return EXIT_FAILURE;
		}
		if (restore_std(save_stdin, save_stdout, save_stderr) < 0)
			return EXIT_FAILURE;
		return shell_exit();
	}

	/* In case we change directory */
	if (strcmp(s->verb->string, "cd") == 0 && s->params) {
		if (store_std(&save_stdin, &save_stdout, &save_stderr) < 0)
			return EXIT_FAILURE;
		if (treat_redirect(s) < 0) {
			restore_std(save_stdin, save_stdout, save_stderr);
			return EXIT_FAILURE;
		}

		if (!shell_cd(s->params)) {
			fprintf(stderr,
			"bash: cd: %s: No such file or directory\n",
			s->params->string);
			restore_std(save_stdin, save_stdout, save_stderr);
			return EXIT_FAILURE;
		}
		if (restore_std(save_stdin, save_stdout, save_stderr) < 0)
			return EXIT_FAILURE;
		return EXIT_SUCCESS;
	}

	/* variable assignment */
	if (s->verb->next_part &&
	strcmp(s->verb->next_part->string, "=") == 0) {
		if (setenv(s->verb->string,
			get_word(s->verb->next_part->next_part), 1) < 0)
			return EXIT_FAILURE;
		return EXIT_SUCCESS;
	}

	/* external command */
	pid = fork();
	/* Could not fork */
	if (pid == -1) {
		fprintf(stderr, "Could not fork\n");
		return EXIT_FAILURE;
	}
	/* Execute command in child */
	if (pid == 0) {
		args = get_argv(s, &size);
		if (store_std(&save_stdin, &save_stdout, &save_stderr) < 0)
			exit(EXIT_FAILURE);
		if (treat_redirect(s) < 0) {
			restore_std(save_stdin, save_stdout, save_stderr);
			exit(EXIT_FAILURE);
		}
		if (execvp(s->verb->string, args) < 0) {
			fprintf(stderr, "Execution failed for '%s'\n",
				s->verb->string);
			restore_std(save_stdin, save_stdout, save_stderr);
			exit(EXIT_FAILURE);
		}
		if (restore_std(save_stdin, save_stdout, save_stderr) < 0)
			exit(EXIT_FAILURE);
	}

	waitpid(pid, &status, 0);
	return WEXITSTATUS(status);
}

/**
 * Process two commands in parallel, by creating two children.
 */
static bool do_in_parallel(command_t *cmd1, command_t *cmd2, int level,
		command_t *father)
{
	/* execute cmd1 and cmd2 simultaneously */
	int pid1, pid2;
	int status1, status2;

	pid1 = fork();
	/* Could not fork */
	if (pid1 == -1) {
		fprintf(stderr, "Could not fork\n");
		return EXIT_FAILURE;
	}

	if (pid1 == 0)
		exit(parse_command(cmd1, level, father));
	else {
		pid2 = fork();
		if (pid1 == -1) {
			waitpid(pid1, &status1, 0);
			fprintf(stderr, "Could not fork\n");
			return EXIT_FAILURE;
		}

		if (pid2 == 0)
			exit(parse_command(cmd2, level, father));
		else {
			waitpid(pid1, &status1, 0);
			waitpid(pid2, &status2, 0);
		}
	}

	if (status1 || status2)
		return false;
	return true;
}

/**
 * Run commands by creating an anonymous pipe (cmd1 | cmd2)
 */
static bool do_on_pipe(command_t *cmd1, command_t *cmd2, int level,
		command_t *father)
{
	/* redirect the output of cmd1 to the input of cmd2 */
	int pid;
	int filedes[2], save_stdin;

	if (pipe(filedes) < 0) {
		fprintf(stderr, "Failed to create pipe\n");
		return false;
	}

	pid = fork();
	/* Could not fork */
	if (pid == -1) {
		fprintf(stderr, "Could not fork\n");
		return EXIT_FAILURE;
	}
	if (pid == 0) {
		close(filedes[0]);
		/* Redirect stdout */
		if (dup2(filedes[1], STDOUT_FILENO) < 0) {
			close(filedes[1]);
			exit(EXIT_FAILURE);
		}
		close(filedes[1]);
		exit(parse_command(cmd1, level, father));
	}

	/* Redirect stdin */
	close(filedes[1]);
	save_stdin = dup(STDIN_FILENO);
	if (save_stdin < 0)
		return false;
	if (dup2(filedes[0], STDIN_FILENO) < 0) {
		if (dup2(save_stdin, STDIN_FILENO) < 0)
			return false;
		close(save_stdin);
		close(filedes[0]);
		return false;
	}
	close(filedes[0]);
	if (parse_command(cmd2, level, father) == EXIT_FAILURE) {
		if (dup2(save_stdin, STDIN_FILENO) < 0)
			return false;
		close(save_stdin);
		waitpid(pid, NULL, 0);
		return false;
	}
	if (dup2(save_stdin, STDIN_FILENO) < 0)
		return false;
	close(save_stdin);

	waitpid(pid, NULL, 0);
	return true;
}

/**
 * Parse and execute a command.
 */
int parse_command(command_t *c, int level, command_t *father)
{
	if (c->op == OP_NONE) {
		/* execute a simple command */
		return parse_simple(c->scmd, level, father);
	}

	switch (c->op) {
	case OP_SEQUENTIAL:
		/* execute the commands one after the other */
		parse_command(c->cmd1, level + 1, c);
		if (parse_command(c->cmd2, level + 1, c) == EXIT_FAILURE)
			return EXIT_FAILURE;
		return EXIT_SUCCESS;

	case OP_PARALLEL:
		/* execute the commands simultaneously */
		if (do_in_parallel(c->cmd1, c->cmd2, level + 1, c))
			return EXIT_SUCCESS;
		return EXIT_FAILURE;

	case OP_CONDITIONAL_NZERO:
		/* execute the second command only if the first one
		 * returns non zero
		 */
		if (parse_command(c->cmd1, level + 1, c) != 0)
			return parse_command(c->cmd2, level + 1, c);
		return EXIT_SUCCESS;

	case OP_CONDITIONAL_ZERO:
		/* execute the second command only if the first one
		 * returns zero
		 */
		if (parse_command(c->cmd1, level + 1, c) == 0)
			return parse_command(c->cmd2, level + 1, c);
		return EXIT_FAILURE;

	case OP_PIPE:
		/* redirect the output of the first command to the
		 * input of the second
		 */
		if (do_on_pipe(c->cmd1, c->cmd2, level + 1, c))
			return EXIT_SUCCESS;
		return EXIT_FAILURE;

	default:
		return SHELL_EXIT;
	}

	return EXIT_SUCCESS;
}
