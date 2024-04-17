#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"

char prompt[PRMTLEN] = { 0 };


void
sigchild_handler()
{
	pid_t pid;
	int status;

	pid = waitpid(0, &status, WNOHANG);

	if (pid > 0)
		printf("Process %d exited with status %d\n", pid, status);
}

void
setup_sigchild()
{
	struct sigaction sigchild_action = { .sa_handler = sigchild_handler,
		                             .sa_flags = SA_NOCLDSTOP |
		                                         SA_RESTART };

	stack_t alternative_stack = { .ss_sp = malloc(SIGSTKSZ),
		                      .ss_size = SIGSTKSZ,
		                      .ss_flags = 0 };

	if (alternative_stack.ss_sp == NULL) {
		perror("malloc");
		exit(-1);
	}

	if (sigaltstack(&alternative_stack, 0) < 0)
		perror("sigaltstack");

	if (sigaction(SIGCHLD, &sigchild_action, NULL) < 0)
		perror("sigaction");
}

// runs a shell command
static void
run_shell()
{
	char *cmd;

	while ((cmd = read_line(prompt)) != NULL)
		if (run_cmd(cmd) == EXIT_SHELL)
			return;
}

// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}

	setpgid(0, 0);
	setup_sigchild();
}

int
main(void)
{
	init_shell();

	run_shell();

	return 0;
}
