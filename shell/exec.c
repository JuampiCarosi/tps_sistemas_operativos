#include "exec.h"

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != EQUAL_CHAR; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	for (int i = 0; i < eargc; i++) {
		int equal_index =
		        block_contains(eargv[i],
		                       EQUAL_CHAR);  // Get argument '=' index

		if (equal_index >= 0) {
			char arg_key[BUFLEN];
			char arg_value[BUFLEN];

			get_environ_key(eargv[i], arg_key);
			get_environ_value(eargv[i], arg_value, equal_index);
			setenv(arg_key, arg_value, 1);
		}
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	int extra_flags = 0;
	if (flags & O_CREAT) {
		extra_flags = S_IWUSR | S_IRUSR;
	}

	int fd = open(file, flags, extra_flags);

	if (fd < 0) {
		printf_debug("Error opening file %s\n", file);
		exit(-1);
	}

	return fd;
}


void
redirect_stdin(char *in_file)
{
	printf_debug("Redirecting stdin to '%s'\n", in_file);
	if (strlen(in_file) > 0) {
		int in_fd = open_redir_fd(in_file, O_RDONLY | O_CLOEXEC);

		int result = dup2(in_fd, STDIN_FILENO);

		if (result < 0) {
			printf_debug("Error redirecting stdin\n");
			close(in_fd);
			exit(-1);
		}

		close(in_fd);
	}
}

// dup2 wrapper
void
wrapper_dup2(int fd, int flow)
{
	if (fd > 0) {
		if (dup2(fd, flow) < 0) {
			perror("dup2");
			printf_debug("Exiting now...");
			_exit(-1);
		};
		return;
	}
	perror("fd < 0 in dup2");
	printf_debug("Exiting now...");
	_exit(-1);
}

void
redirect_stdout(char *out_file)
{
	if (strlen(out_file) > 0) {
		int out_fd =
		        open_redir_fd(out_file,
		                      O_WRONLY | O_CREAT | O_CLOEXEC | O_TRUNC);

		int result = dup2(out_fd, STDOUT_FILENO);

		if (result < 0) {
			printf_debug("Error redirecting stdout\n");
			close(out_fd);
			exit(-1);
		}
		close(out_fd);
	}
}

void
redirect_stderr(char *err_file)
{
	if (strlen(err_file) > 0) {
		if (strcmp(err_file, "&1") == 0) {
			int result = dup2(STDOUT_FILENO, STDERR_FILENO);

			if (result < 0) {
				printf_debug(
				        "Error redirecting stderr to stdout\n");
				exit(-1);
			}
		} else {
			int err_fd = open_redir_fd(err_file,
			                           O_WRONLY | O_CREAT |
			                                   O_CLOEXEC | O_TRUNC);

			int result = dup2(err_fd, STDERR_FILENO);

			if (result < 0) {
				printf_debug("Error redirecting stderr\n");
				close(err_fd);
				exit(-1);
			}
			close(err_fd);
		}
	}
}

void
run_exec(struct execcmd *e)
{
	set_environ_vars(e->eargv, e->eargc);

	if (e->argv[0] == NULL) {
		return;
	}

	int execvp_result = execvp(e->argv[0], e->argv);

	if (execvp_result < 0) {
		printf_debug("Error executing execvp\n");
		exit(-1);
	}
}

void
run_pipe(struct pipecmd *p)
{
	int fildes[2];
	int pipe_res = pipe(fildes);

	if (pipe_res < 0) {
		printf_debug("Error creating a pipe\n");
		exit(-1);
	}

	pid_t left_pid = fork();

	if (left_pid < 0) {
		printf_debug("Error creating a new process\n");
		exit(-1);
	}

	if (left_pid == 0) {
		close(fildes[READ]);

		int dup2_res = dup2(fildes[WRITE], STDOUT_FILENO);

		close(fildes[WRITE]);

		if (dup2_res < 0) {
			printf_debug("Error duplicating an existing "
			             "object descriptor\n");
			exit(-1);
		}

		exec_cmd(p->leftcmd);
	}

	pid_t right_pid = fork();

	if (right_pid < 0) {
		printf_debug("Error creating a new process\n");
		exit(-1);
	}

	if (right_pid == 0) {
		close(fildes[WRITE]);

		int dup2_res = dup2(fildes[READ], STDIN_FILENO);

		close(fildes[READ]);

		if (dup2_res < 0) {
			printf_debug("Error duplicating an existing "
			             "object descriptor\n");
			exit(-1);
		}

		exec_cmd(p->rightcmd);
	}

	close(fildes[READ]);
	close(fildes[WRITE]);
	waitpid(left_pid, NULL, 0);
	waitpid(right_pid, NULL, 0);
}


// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC:
		e = (struct execcmd *) cmd;
		run_exec(e);

		break;

	case BACK: {
		b = (struct backcmd *) cmd;
		exec_cmd(b->c);

		break;
	}
	case REDIR: {
		r = (struct execcmd *) cmd;

		redirect_stdin(r->in_file);
		redirect_stdout(r->out_file);
		redirect_stderr(r->err_file);

		r->type = EXEC;
		exec_cmd((struct cmd *) r);

		break;
	}

	case PIPE: {
		p = (struct pipecmd *) cmd;
		run_pipe(p);
		// free the memory allocated
		// for the pipe tree structure
		free_command(parsed_pipe);

		break;
	}
	}

	exit(0);
}
