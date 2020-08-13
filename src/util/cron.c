#include "cron.h"
#include "heap.h"
#include "rmalloc.h"
#include "../RG.h"
#include <time.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX(a,b) ((a) >= (b)) ? (a) : (b)

//------------------------------------------------------------------------------
// Data structures
//------------------------------------------------------------------------------

// CRON task
typedef struct {
	struct timespec due;    // absolute time for when task should run
	CronTaskCB cb;          // callback to call when task is due
	void *pdata;            // [optional] private data passed to callback
} CRON_TASK;

// CRON object
typedef struct {
	bool alive;                   // indicates cron is active
	heap_t *tasks;                // min heap of cron tasks
	pthread_mutex_t mutex;        // mutex control access to tasks
	pthread_mutex_t condv_mutex;  // mutex control access to condv
	pthread_cond_t condv;         // conditional variable
	pthread_t thread;             // thread running cron main loop
} CRON;

// single static CRON instance, initialized at CRON_Start
static CRON *cron = NULL;

// compares two time objects
static int cmp_timespec(struct timespec a, struct timespec b) {
	if(a.tv_sec == b.tv_sec) {
		return a.tv_nsec - b.tv_nsec;
	} else {
		return a.tv_sec - b.tv_sec;
	}
}

// minimum heap sort function
static int CRON_JobCmp(const void *a, const void *b, const void *udata) {
	CRON_TASK *_a = (CRON_TASK*)a;
	CRON_TASK *_b = (CRON_TASK*)b;
	return cmp_timespec(_b->due, _a->due);
}

//------------------------------------------------------------------------------
// Utility functions
//------------------------------------------------------------------------------

// compute now + ms
static struct timespec due_in_ms(uint ms) {
	struct timespec due;
    clock_gettime(CLOCK_REALTIME, &due);

	due.tv_sec += ms / 1000;
    due.tv_nsec += (ms % 1000) * 1000000;

	return due;
}

static void CRON_WakeUp(void) {
	// Set conditional variable to wake up CRON thread
	pthread_mutex_lock(&cron->condv_mutex);
	pthread_cond_signal(&cron->condv);
	pthread_mutex_unlock(&cron->condv_mutex);
}

// determines if task is due
static bool CRON_TaskDue(const CRON_TASK *t) {
	ASSERT(t);

	struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
	return cmp_timespec(now, t->due) >= 0;
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
	ASSERT(t);
	t->cb(t->pdata);
}

static void CRON_FreeTask(CRON_TASK *t) {
	ASSERT(t);
	rm_free(t);
}

static void clear_tasks() {
	CRON_TASK *task = NULL;
	while((task = CRON_RemoveTask())) {
		CRON_FreeTask(task);
	}
}

//------------------------------------------------------------------------------
// CRON main loop
//------------------------------------------------------------------------------

static void *Cron_Run(void *arg) {
	while(cron->alive) {
		// execute due tasks
		CRON_TASK *task = NULL;
		while((task = CRON_Peek()) && CRON_TaskDue(task)) {
			task = CRON_RemoveTask();
			CRON_PerformTask(task);
			CRON_FreeTask(task);
		}

		struct timespec timeout = (task) ? task->due : due_in_ms(1000);
		pthread_mutex_lock(&cron->condv_mutex);
		pthread_cond_timedwait(&cron->condv, &cron->condv_mutex, &timeout);
		pthread_mutex_unlock(&cron->condv_mutex);
	}

	return NULL;
}

//------------------------------------------------------------------------------
// User facing API
//------------------------------------------------------------------------------

void Cron_Start(void) {
	ASSERT(cron == NULL);

	cron = rm_malloc(sizeof(CRON));
	cron->alive = true;
	cron->tasks = heap_new(CRON_JobCmp, NULL);
	pthread_cond_init(&cron->condv, NULL);
	pthread_mutex_init(&cron->mutex, NULL);
	pthread_mutex_init(&cron->condv_mutex, NULL);
	pthread_create(&cron->thread, NULL, Cron_Run, NULL);
}

void Cron_Stop(void) {
	ASSERT(cron != NULL);

	// Stop cron main loop
	cron->alive = false;
	CRON_WakeUp();

	// Wait for thread to terminate
	pthread_join(cron->thread, NULL);

	clear_tasks();

	heap_free(cron->tasks);
	pthread_mutex_destroy(&cron->mutex);
	pthread_mutex_destroy(&cron->condv_mutex);
	pthread_cond_destroy(&cron->condv);
	rm_free(cron);
	cron = NULL;
}

void Cron_AddTask(uint when, CronTaskCB cb, void *pdata) {
	ASSERT(cron != NULL);
	ASSERT(cb != NULL);

	CRON_TASK *task = rm_malloc(sizeof(CRON_TASK));
	task->cb     =  cb;
	task->pdata  =  pdata;
	task->due    =  due_in_ms(when);

	CRON_InsertTask(task);
}

