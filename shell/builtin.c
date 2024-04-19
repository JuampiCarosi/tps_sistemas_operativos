#include "builtin.h"
#define TRUE 1
#define FALSE 0
#define HOME "HOME"

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	if (strcmp(cmd, "exit") == 0)
		return TRUE;

	return FALSE;
}

static bool
is_whitespace(char c)
{
	return c == ' ' || c == '\t' || c == '\n';
}

static char *
trim_whitespaces(char *str)
{
	while (is_whitespace(*str)) {
		str++;
	}

	int n = strlen(str) - 1;

	while (n >= 0 && is_whitespace(str[n])) {
		n--;
	}
	str[n + 1] = '\0';

	return str;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	if (!strstr(cmd, "cd")) {
		return FALSE;
	}

	cmd = trim_whitespaces(cmd);

	if (strcmp("cd", cmd) == 0) {
		char *home = getenv(HOME);
		if (chdir(home) < 0) {
			printf_debug("Error changing to HOME");
			return TRUE;
		}
		snprintf(prompt, sizeof prompt, "(%s)", home);
		return TRUE;
	}

	if (strncmp(cmd, "cd ", 3) != 0) {
		return FALSE;
	}

	strtok(cmd, " ");
	char *directory = strtok(NULL, " ");

	if (directory) {
		if (chdir(directory) < 0) {
			printf_debug("Error changing to %s\n", directory);
			return TRUE;
		}
		char *buffer = getcwd(NULL, 0);
		snprintf(prompt, sizeof prompt, "(%s)", buffer);
		free(buffer);
	}
	return TRUE;
}


// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (strcmp("pwd", cmd) != 0) {
		return FALSE;
	}

	char *buffer = getcwd(NULL, 0);
	if (!buffer) {
		printf_debug("Error getting current directory\n");
		return TRUE;
	}
	printf_debug("%s\n", buffer);
	free(buffer);
	return TRUE;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	// Your code here

	return 0;
}
