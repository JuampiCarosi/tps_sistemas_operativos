#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

#define MAX_MLFQ_EXECUTIONS 10

void sched_halt(void);

// Choose a user environment to run and run it.
static int sched_runs;

struct MLFQ_queue {
	envid_t envs[NENV];
	int size;
	int beginning;
};
struct MLFQ_sched {
	struct MLFQ_queue q0;
	struct MLFQ_queue q1;
	struct MLFQ_queue q2;
	struct MLFQ_queue q3;
	int total_executions;
};

struct MLFQ_sched mlfq_sched;

void
round_robin()
{
	struct Env *next_env = curenv;
	int start_index = next_env ? ENVX(next_env->env_id) + 1 : 0;

	for (int i = 0; i < NENV; i++) {
		int actual_index = (start_index + i) % NENV;
		next_env = &envs[actual_index];

		if (next_env->env_status == ENV_RUNNABLE) {
			env_run(next_env);
		}
	}

	if (curenv && curenv->env_status == ENV_RUNNING) {
		env_run(curenv);
	}
}

void
priority_MLFQ()
{
}

void
sched_yield(void)
{
	sched_runs++;
	if (mlfq_sched.total_executions >= MAX_MLFQ_EXECUTIONS) {
		// boost();
		mlfq_sched.total_executions = 0;
	}

#ifdef SCHED_ROUND_ROBIN
	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running. Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// Your code here
	// Wihtout scheduler, keep runing the last environment while it exists
	round_robin();

#endif

#ifdef SCHED_PRIORITIES
	// Implement simple priorities scheduling.
	//
	// Environments now have a "priority" so
	// it must be consider when the
	// selection is performed.
	//
	// Be careful to not fall in
	// "starvation" such that only one
	// environment is selected and run every
	// time.

	// Your code here - Priorities
	priority();
#endif

	// sched_halt never returns
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the "
		        "system!\n");
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire
	// the big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print
	// statistics on performance. Your code here

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}
