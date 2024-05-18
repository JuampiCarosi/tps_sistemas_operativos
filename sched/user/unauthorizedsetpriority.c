// check that we can read the priority of the current environment
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	envid_t parent_envid = sys_getenvid();

	envid_t fork_id = fork();

	if (fork_id == 0) {
		int res = sys_set_priority(parent_envid, 3);
		cprintf("Response: %d\n", res);
	}
}
