#include "utils.h"
#include <stdarg.h>

// splits a string line in two
// according to the splitter character
char *
split_line(char *buf, char splitter)
{
	int i = 0;

	while (buf[i] != splitter && buf[i] != END_STRING)
		i++;

	buf[i++] = END_STRING;

	while (buf[i] == SPACE)
		i++;

	return &buf[i];
}

// looks in a block for the 'c' character
// and returns the index in which it is, or -1
// in other case
int
block_contains(char *buf, char c)
{
	for (size_t i = 0; i < strlen(buf); i++)
		if (buf[i] == c)
			return i;

	return -1;
}

// Printf wrappers for debug purposes so that they don't
// show when shell is compiled in non-interactive way
int
printf_debug(char *format, ...)
{
#ifndef SHELL_NO_INTERACTIVE
	va_list args;
	va_start(args, format);
	int ret = vprintf(format, args);
	va_end(args);

	return ret;
#else
	return 0;
#endif
}

int
perror_debug(char *msg)
{
#ifndef SHELL_NO_INTERACTIVE
	if (isatty(STDERR_FILENO))
		perror(msg);
#endif

	return 0;
}


int
fprintf_debug(FILE *file, char *format, ...)
{
#ifndef SHELL_NO_INTERACTIVE
	va_list args;
	va_start(args, format);
	int ret = vfprintf(file, format, args);
	va_end(args);

	return ret;
#else
	return 0;
#endif
}

void
restore_default_signal_status(int signal)
{
	stack_t stack;
	sigaltstack(NULL, &stack);

	struct sigaction signal_action;

	memset(&signal_action, 0, sizeof signal_action);
	sigemptyset(&signal_action.sa_mask);

	signal_action.sa_handler = SIG_DFL;
	signal_action.sa_flags = 0;

	if (sigaction(signal, &signal_action, NULL) == -1) {
		perror_debug("restore sigaction failed");
		exit(-1);
	}

	if (stack.ss_sp != NULL)
		free(stack.ss_sp);
}