// check that we can read the priority of the current environment
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	envid_t envid = sys_getenvid();
	int priority = sys_get_priority(envid);
	cprintf("priority: %d\n", priority);
}
