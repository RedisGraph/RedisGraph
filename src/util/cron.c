#include "cron.h"
#include "heap.h"
#include "rmalloc.h"
#include "../RG.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX(a,b) ((a) >= (b)) ? (a) : (b)

//------------------------------------------------------------------------------
// Data structures
//------------------------------------------------------------------------------

// CRON task
typedef struct {
	uint64_t due;	// absolute time for when task should run
	CronTaskCB cb;  // callback to call when task is due
	void *pdata;    // [optional] private data passed to callback
} CRON_TASK;

// CRON object
typedef struct {
	bool alive;             // indicates cron is active
	heap_t *tasks;          // min heap of cron tasks
	pthread_mutex_t mutex;  // mutex control access to tasks
	pthread_cond_t condv;   // conditional variable, indicates main loop runs
	pthread_t thread;       // thread running cron main loop
} CRON;

// single static CRON instance, initialized at CRON_Start
static CRON *cron = NULL;

// minimum heap sort function
static int CRON_JobCmp(const void *a, const void *b, const void *udata) {
	CRON_TASK *_a = (CRON_TASK*)a;
	CRON_TASK *_b = (CRON_TASK*)b;
	return (_b->due - _a->due);
}

//------------------------------------------------------------------------------
// Utility functions
//------------------------------------------------------------------------------

// compute current time in ms
static uint64_t _get_time_ms(void) {
	struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
	uint64_t time_in_ms = (spec.tv_sec) * 1000 + (spec.tv_nsec) / 1000;
	return time_in_ms;
}

static void CRON_WakeUp(void) {
	// Send single to wake up CRON
	pthread_kill(cron->thread, SIGUSR1);
}

// determines if task is due
static bool CRON_TaskDue(const CRON_TASK *t) {
	// ASSERT(t);
	return(_get_time_ms() >= t->due);
}

static CRON_TASK *CRON_Peek() {
	pthread_mutex_lock(&cron->mutex);
	CRON_TASK *task = heap_peek(cron->tasks);
	pthread_mutex_unlock(&cron->mutex);
	return task;
}

static CRON_TASK *CRON_RemoveTask(void) {
	pthread_mutex_lock(&cron->mutex);
	CRON_TASK *task = (CRON_TASK*)heap_poll(cron->tasks);
	pthread_mutex_unlock(&cron->mutex);
	return task;
}

static void CRON_InsertTask(CRON_TASK *t) {
	pthread_mutex_lock(&cron->mutex);
	heap_offer(&cron->tasks, t);
	pthread_mutex_unlock(&cron->mutex);

	CRON_WakeUp();
}

static void CRON_PerformTask(CRON_TASK *t) {
	// ASSERT(t);
	t->cb(t->pdata);
}

static void CRON_FreeTask(CRON_TASK *t) {
	// ASSERT(t);
	// if(t->pdata) rm_free(t->pdata);
	rm_free(t);
}

static void clear_tasks() {
	CRON_TASK *task = NULL;
	while((task = CRON_RemoveTask())) {
		CRON_FreeTask(task);
	}
	heap_free(cron->tasks);
}

void _signal_handler(int signum) {
}

//------------------------------------------------------------------------------
// CRON main loop
//------------------------------------------------------------------------------

static void *Cron_Run(void *arg) {
	struct sigaction sigact;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigact.sa_handler = _signal_handler;
	sigaction(SIGUSR1, &sigact, NULL);

	// signal main loop is running
	//pthread_mutex_lock(&cron->mutex);
	//pthread_cond_signal(&cron->condv);
	//pthread_mutex_unlock(&cron->mutex);

	while(cron->alive) {
		// execute due tasks
		CRON_TASK *task = NULL;
		while((task = CRON_Peek()) && CRON_TaskDue(task)) {
			task = CRON_RemoveTask();
			CRON_PerformTask(task);
			CRON_FreeTask(task);
		}

		if(task) {
			// sleep until next task is due
			float d = MAX(0.0, task->due - _get_time_ms()) / 1000.0;
			sleep(d);
		} else {
			// TODO: check for tasks, only pause if there are no tasks
			// TODO: consider switching from pause to sleep(0.1)
			// no more tasks, pause until a new task arraives
			//pause();
			sleep(0.1);
		}
	}

	return NULL;
}

//------------------------------------------------------------------------------
// User facing API
//------------------------------------------------------------------------------

void Cron_Start(void) {
	// ASSERT(cron == NULL);

	cron = rm_malloc(sizeof(CRON));
	cron->alive = true;
	cron->tasks = heap_new(CRON_JobCmp, NULL);
	pthread_cond_init(&cron->condv, NULL);
	pthread_mutex_init(&cron->mutex, NULL);
	pthread_create(&cron->thread, NULL, Cron_Run, NULL);

	// wait for thread to start
	//pthread_mutex_lock(&cron->mutex);
	//pthread_cond_wait(&cron->condv , &cron->mutex);
	//pthread_mutex_unlock(&cron->mutex);
}


void Cron_Stop(void) {
	//ASSERT(cron != NULL);

	// Stop cron main loop
	cron->alive = false;
	CRON_WakeUp();

	// Wait for thread to terminate
	pthread_join(cron->thread, NULL);

	clear_tasks();
	pthread_mutex_destroy(&cron->mutex);
	pthread_cond_destroy(&cron->condv);
	rm_free(cron);
	cron = NULL;
}

void Cron_AddTask(uint when, CronTaskCB cb, void *pdata) {
	// ASSERT(cron != NULL);
	// ASSERT(cb != NULL);

	CRON_TASK *task = rm_malloc(sizeof(CRON_TASK));
	task->cb     =  cb;
	task->pdata  =  pdata;
	task->due    =  _get_time_ms() + when;

	CRON_InsertTask(task);
}

