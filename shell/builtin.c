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
	if (strstr(cmd, "cd")) {
		char *directory = split_line(cmd, ' ');

		if (directory[0] != '\0') {
			if (chdir(directory) < 0) {
				printf_debug("Error changing to %s\n", directory);
			} else {
				char *buffer = getcwd(NULL, 0);
				snprintf(prompt, sizeof prompt, "(%s)", buffer);
				free(buffer);
			}
		} else {
			char *home = getenv(HOME);
			if (chdir(home) < 0) {
				printf_debug("Error changing to %s\n", home);
			} else {
				snprintf(prompt, sizeof prompt, "(%s)", home);
			}
		}
		return TRUE;
	}
	return FALSE;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (strcmp("pwd", cmd) == 0) {
		char *buffer = getcwd(NULL, 0);
		if (!buffer) {
			printf_debug("Error getting current directory\n");
			return TRUE;
		}
		printf_debug("%s\n", buffer);
		free(buffer);
		return TRUE;
	}
	return FALSE;
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
