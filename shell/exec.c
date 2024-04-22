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

static int
check_syscall(int syscall_result, char *message)
{
	if (syscall_result < 0) {
		perror_debug(message);
		exit(-1);
	}
	return syscall_result;
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

	int fd = check_syscall(open(file, flags, extra_flags),
	                       "Error opening file\n");

	return fd;
}

static void
redirect_stdin(char *in_file)
{
	if (strlen(in_file) > 0) {
		int in_fd = open_redir_fd(in_file, O_RDONLY | O_CLOEXEC);

		int result = dup2(in_fd, STDIN_FILENO);

		close(in_fd);

		check_syscall(result, "Error redirecting stdin\n");
	}
}

static void
redirect_stdout(char *out_file)
{
	if (strlen(out_file) > 0) {
		int out_fd =
		        open_redir_fd(out_file,
		                      O_WRONLY | O_CREAT | O_CLOEXEC | O_TRUNC);

		int result = dup2(out_fd, STDOUT_FILENO);

		close(out_fd);

		check_syscall(result, "Error redirecting stdout\n");
	}
}

static void
redirect_stderr(char *err_file)
{
	if (strlen(err_file) > 0) {
		if (strcmp(err_file, "&1") == 0) {
			int result = dup2(STDOUT_FILENO, STDERR_FILENO);

			check_syscall(result,
			              "Error redirecting stderr to stdout\n");
		} else {
			int err_fd = open_redir_fd(err_file,
			                           O_WRONLY | O_CREAT |
			                                   O_CLOEXEC | O_TRUNC);

			int result = dup2(err_fd, STDERR_FILENO);

			close(err_fd);
			check_syscall(result, "Error redirecting stderr\n");
		}
	}
}

static void
run_exec(struct execcmd *e)
{
	set_environ_vars(e->eargv, e->eargc);

	if (e->argv[0] == NULL) {
		return;
	}

	check_syscall(execvp(e->argv[0], e->argv), "Error executing execvp\n");
}

static void
run_pipe(struct pipecmd *p)
{
	int fildes[2];
	check_syscall(pipe(fildes), "Error creating a pipe\n");

	pid_t left_pid = check_syscall(fork(), "Error creating a new process\n");

	if (left_pid == 0) {
		close(fildes[READ]);

		int dup2_res = dup2(fildes[WRITE], STDOUT_FILENO);

		close(fildes[WRITE]);

		check_syscall(dup2_res,
		              "Error duplicating an existing object "
		              "descriptor\n");

		exec_cmd(p->leftcmd);
	}

	pid_t right_pid = fork();

	check_syscall(right_pid, "Error creating a new process\n");

	if (right_pid == 0) {
		close(fildes[WRITE]);

		int dup2_res = dup2(fildes[READ], STDIN_FILENO);

		close(fildes[READ]);

		check_syscall(dup2_res,
		              "Error duplicating an existing object "
		              "descriptor\n");

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
		free_command(parsed_pipe);

		break;
	}
	}

	exit(0);
}
