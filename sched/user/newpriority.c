// check that we can read the priority of the current environment
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	envid_t envid = sys_getenvid();
	sys_set_priority(envid, 3);

	int child = fork();
	if (child == 0) {
		envid_t envid = sys_getenvid();
		int priority = sys_get_priority(envid);
		cprintf("Child priority: %d\n", priority);
	} else {
		envid_t envid = sys_getenvid();
		int priority = sys_get_priority(envid);
		cprintf("Parent priority: %d\n", priority);
	}
}
