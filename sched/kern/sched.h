/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_SCHED_H
#define JOS_KERN_SCHED_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

extern struct MLFQ_sched mlfq_sched;

struct MLFQ_queue {
	envid_t envs[NENV];
	int last;
	int beginning;
};
struct MLFQ_sched {
	struct MLFQ_queue q0;
	struct MLFQ_queue q1;
	struct MLFQ_queue q2;
	struct MLFQ_queue q3;
	int total_executions;
};

// This function does not return.
void sched_destroy_env(envid_t env_id);
void sched_yield(void);
void sched_push_env(envid_t env_id, int queue);

#endif  // !JOS_KERN_SCHED_H
