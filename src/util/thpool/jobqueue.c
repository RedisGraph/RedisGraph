#include "jobqueue.h"

/* ============================ JOB QUEUE =========================== */

/* Initialize queue */
int jobqueue_init(jobqueue *jobqueue_p) {
    jobqueue_p->len = 0;
    jobqueue_p->front = NULL;
    jobqueue_p->rear = NULL;

    jobqueue_p->has_jobs = (struct bsem *)malloc(sizeof(struct bsem));
    if (jobqueue_p->has_jobs == NULL) {
        return -1;
    }

    pthread_mutex_init(&(jobqueue_p->rwmutex), NULL);
    bsem_init(jobqueue_p->has_jobs, 0);

    return 0;
}

/* Clear the queue */
void jobqueue_clear(jobqueue *jobqueue_p) {

    while (jobqueue_p->len) {
        free(jobqueue_pull(jobqueue_p));
    }

    jobqueue_p->front = NULL;
    jobqueue_p->rear = NULL;
    bsem_reset(jobqueue_p->has_jobs);
    jobqueue_p->len = 0;
}

/* Add (allocated) job to queue
 */
void jobqueue_push(jobqueue *jobqueue_p, struct job *newjob) {

    pthread_mutex_lock(&jobqueue_p->rwmutex);
    newjob->prev = NULL;

    switch (jobqueue_p->len) {

    case 0: /* if no jobs in queue */
        jobqueue_p->front = newjob;
        jobqueue_p->rear = newjob;
        break;

    default: /* if jobs in queue */
        jobqueue_p->rear->prev = newjob;
        jobqueue_p->rear = newjob;
    }
    jobqueue_p->len++;

    bsem_post(jobqueue_p->has_jobs);
    pthread_mutex_unlock(&jobqueue_p->rwmutex);
}

/* Get first job from queue(removes it from queue)
 *
 * Notice: Caller MUST hold a mutex
 */
struct job *jobqueue_pull(jobqueue *jobqueue_p) {

    pthread_mutex_lock(&jobqueue_p->rwmutex);
    job *job_p = jobqueue_p->front;

    switch (jobqueue_p->len) {

    case 0: /* if no jobs in queue */
        break;

    case 1: /* if one job in queue */
        jobqueue_p->front = NULL;
        jobqueue_p->rear = NULL;
        jobqueue_p->len = 0;
        break;

    default: /* if >1 jobs in queue */
        jobqueue_p->front = job_p->prev;
        jobqueue_p->len--;
        /* more than one job in queue -> post it */
        bsem_post(jobqueue_p->has_jobs);
    }

    pthread_mutex_unlock(&jobqueue_p->rwmutex);
    return job_p;
}

/* Free all queue resources back to the system */
void jobqueue_destroy(jobqueue *jobqueue_p) {
    jobqueue_clear(jobqueue_p);
    free(jobqueue_p->has_jobs);
}
