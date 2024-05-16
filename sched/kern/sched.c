#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>
#include <kern/sched.h>

#define MAX_MLFQ_EXECUTIONS 10

void sched_halt(void);

static int sched_runs;

struct MLFQ_sched mlfq_sched = {
	.q0 = { .last = 0, .beginning = 0 },
	.q1 = { .last = 0, .beginning = 0 },
	.q2 = { .last = 0, .beginning = 0 },
	.q3 = { .last = 0, .beginning = 0 },
	.total_executions = 0,
};

void
sched_push_env(envid_t env_id, int queue)
{
	struct MLFQ_queue *q = NULL;
	switch (queue) {
	case 0:
		q = &mlfq_sched.q0;
		break;
	case 1:
		q = &mlfq_sched.q1;
		break;
	case 2:
		q = &mlfq_sched.q2;
		break;
	default:
		q = &mlfq_sched.q3;
		break;
	}

	int index = q->last % NENV;
	envs[ENVX(env_id)].current_queue = queue > 3 ? 3 : queue;
	q->envs[index] = env_id;
	q->last++;
}

void
sched_remove_env(struct MLFQ_queue *queue, int position)
{
	if (position == queue->beginning) {
		queue->beginning++;
		return;
	}

	envid_t first = queue->envs[queue->beginning % NENV];
	envid_t target = queue->envs[position];

	envid_t aux = first;
	queue->envs[queue->beginning % NENV] = target;
	queue->envs[position] = aux;

	queue->beginning++;
}

void
sched_destroy_env(envid_t env_id)
{
	struct Env *env = &envs[ENVX(env_id)];
	int queue = env->current_queue;
	struct MLFQ_queue *q = NULL;

	switch (queue) {
	case 0:
		q = &mlfq_sched.q0;
		break;
	case 1:
		q = &mlfq_sched.q1;
		break;
	case 2:
		q = &mlfq_sched.q2;
		break;
	default:
		q = &mlfq_sched.q3;
		break;
	}

	for (int i = q->beginning; i < q->last; i++) {
		if (q->envs[i % NENV] == env_id) {
			sched_remove_env(q, i % NENV);
			break;
		}
	}
}

void
clean_queues()
{
	mlfq_sched.q0.beginning = 0;
	mlfq_sched.q0.last = 0;
	mlfq_sched.q1.beginning = 0;
	mlfq_sched.q1.last = 0;
	mlfq_sched.q2.beginning = 0;
	mlfq_sched.q2.last = 0;
	mlfq_sched.q3.beginning = 0;
	mlfq_sched.q3.last = 0;
}

void
boost_envs()
{
	clean_queues();
	for (int i = 0; i < NENV; i++) {
		envs[i].current_queue = 0;
		sched_push_env(envs[i].env_id, 0);
	}
}

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

bool
is_empty(struct MLFQ_queue queue)
{
	return (queue.beginning % NENV) == (queue.last % NENV);
}

int
get_best_priority(struct MLFQ_queue **q)
{
	// if (!is_empty(mlfq_sched.q0)) {
	// 	*q = &mlfq_sched.q0;
	// 	return 0;
	// }

	// if (!is_empty(mlfq_sched.q1)) {
	// 	*q = &mlfq_sched.q1;
	// 	return 1;
	// }

	// if (!is_empty(mlfq_sched.q2)) {
	// 	*q = &mlfq_sched.q2;
	// 	return 2;
	// }

	// if (!is_empty(mlfq_sched.q3)) {
	// 	*q = &mlfq_sched.q3;
	// 	return 3;
	// }

	// return -1;


	for (int i = mlfq_sched.q0.beginning; i < mlfq_sched.q0.last; i++) {
		if (envs[ENVX(mlfq_sched.q0.envs[i % NENV])].env_status ==
		    ENV_RUNNABLE) {
			*q = &mlfq_sched.q0;
			return 0;
		}
	}

	for (int i = mlfq_sched.q1.beginning; i < mlfq_sched.q1.last; i++) {
		if (envs[ENVX(mlfq_sched.q1.envs[i % NENV])].env_status ==
		    ENV_RUNNABLE) {
			*q = &mlfq_sched.q1;
			return 1;
		}
	}

	for (int i = mlfq_sched.q2.beginning; i < mlfq_sched.q2.last; i++) {
		if (envs[ENVX(mlfq_sched.q2.envs[i % NENV])].env_status ==
		    ENV_RUNNABLE) {
			*q = &mlfq_sched.q2;
			return 2;
		}
	}

	for (int i = mlfq_sched.q3.beginning; i < mlfq_sched.q3.last; i++) {
		if (envs[ENVX(mlfq_sched.q3.envs[i % NENV])].env_status ==
		    ENV_RUNNABLE) {
			*q = &mlfq_sched.q3;
			return 3;
		}
	}

	return -1;
}

void
priority_MLFQ()
{
	struct Env *curr_env = curenv;
	struct MLFQ_queue *best_priority_queue = NULL;
	int queue_number = get_best_priority(&best_priority_queue);

	if (!best_priority_queue) {
		if (curenv && curenv->env_status == ENV_RUNNING) {
			env_run(curenv);
		} else {
			sched_halt();
		}
	}

	int start_queue = best_priority_queue->beginning;
	int last_queue = best_priority_queue->last;

	for (int i = start_queue; i < last_queue; i++) {
		envid_t next_env_id = best_priority_queue->envs[i % NENV];
		struct Env *next_env = &envs[ENVX(next_env_id)];

		if (next_env->env_status == ENV_RUNNABLE) {
			sched_remove_env(best_priority_queue, i % NENV);
			sched_push_env(next_env_id, queue_number + 1);
			env_run(next_env);
		}
	}
}

void
sched_yield(void)
{
	sched_runs++;
	if (mlfq_sched.total_executions >= MAX_MLFQ_EXECUTIONS) {
		boost_envs();
		mlfq_sched.total_executions = 0;
	}
	// printea las colas
//	cprintf("Q0: %d, Q1: %d, Q2: %d, Q3: %d\n",
//	        mlfq_sched.q0.last - mlfq_sched.q0.beginning,
//	        mlfq_sched.q1.last - mlfq_sched.q1.beginning,
//	        mlfq_sched.q2.last - mlfq_sched.q2.beginning,
//	        mlfq_sched.q3.last - mlfq_sched.q3.beginning);

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
	priority_MLFQ();
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
