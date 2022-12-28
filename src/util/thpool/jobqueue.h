#pragma once
#include "bsem.h"

typedef void (*async_task)(void *);
typedef enum { TIED, UNTIED, WAIT } job_type;

/* Job */
typedef struct job {
    struct job *prev; /* pointer to previous job   */
    async_task task;  /* function pointer          */
    void *arg;        /* function's argument       */
    job_type type;
} job;

/* Job queue */
typedef struct jobqueue {
    pthread_mutex_t rwmutex; /* used for queue r/w access */
    job *front;              /* pointer to front of queue */
    job *rear;               /* pointer to rear  of queue */
    bsem *has_jobs;          /* flag as binary semaphore  */
    int len;                 /* number of jobs in queue   */
} jobqueue;

int jobqueue_init(jobqueue *jobqueue_p);
void jobqueue_clear(jobqueue *jobqueue_p);
void jobqueue_push(jobqueue *jobqueue_p, struct job *newjob_p);
struct job *jobqueue_pull(jobqueue *jobqueue_p);
void jobqueue_destroy(jobqueue *jobqueue_p);
