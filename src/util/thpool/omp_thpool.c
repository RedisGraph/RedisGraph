#include "omp_thpool.h"
#include <pthread.h>
#if defined(__linux__)
#include <sys/prctl.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <stdatomic.h>
#include <unistd.h>
#include "jobqueue.h"
#include <stdbool.h>

/* Thread */
typedef struct thread {
    pthread_t pthread;        /* pointer to actual thread  */
    struct omp_thpool_ *pool; /* access to thpool          */
} thread;

/* Threadpool */
typedef struct omp_thpool_ {
    thread *tasks_thread;   /* pointer to the jobqueue thread   */
    volatile int keepalive; /* keep pool alive           */
    jobqueue jobqueue;      /* job queue                 */
    int threads_num;        /* number of threads         */
    bsem *wait_bsem;

} omp_thpool_;

static int tasks_thread_init(omp_thpool_ *pool);
static void *tasks_thread_do(struct thread *thread_p);

static void *tasks_thread_do(struct thread *thread_p) {

    /* Set thread name for profiling and debuging */
    char *thread_name = "tasks_thread";

#if defined(__linux__)
    /* Use prctl instead to prevent using _GNU_SOURCE flag and implicit declaration */
    prctl(PR_SET_NAME, thread_name);
#elif defined(__APPLE__) && defined(__MACH__)
    pthread_setname_np(thread_name);
#else
    err("thread_do(): pthread_setname_np is not supported on this system");
#endif

    /* Assure all threads have been created before starting serving */
    omp_thpool_ *pool = thread_p->pool;
    omp_set_num_threads(pool->threads_num);
    omp_set_dynamic(0);
    omp_set_nested(true);

#pragma omp parallel num_threads(pool->threads_num)
    {
#pragma omp master
        {
            // printf("omp thread num: %d\n", omp_get_num_threads());
            // printf("thread num: %d\n", omp_get_thread_num());
            while (pool->keepalive) {
                bsem_wait(pool->jobqueue.has_jobs);

                if (pool->keepalive) {

                    /* Read job from queue and execute it */
                    void (*func_buff)(void *);
                    void *arg_buff;
                    job *job_p = jobqueue_pull(&pool->jobqueue);
                    if (job_p) {
                        func_buff = job_p->task;
                        arg_buff = job_p->arg;
                        job_type type = job_p->type;
                        // func_buff(arg_buff);
                        if (type == TIED) {
#pragma omp task
                            { func_buff(arg_buff); }
                        } else if (type == UNTIED) {
#pragma omp task untied
                            { func_buff(arg_buff); }
                        } else {
#pragma omp taskwait
                            bsem_post(pool->wait_bsem);
                            // atomic_store(&pool->wait, 0);
                        }
                        free(job_p);
                    }
                }
            }
        }
    }

    return NULL;
}

static int tasks_thread_init(omp_thpool_ *pool) {

    thread *thread_p = (struct thread *)malloc(sizeof(struct thread));
    // if (thread_p == NULL) {
    //     err("thread_init(): Could not allocate memory for thread\n");
    //     return -1;
    // }

    thread_p->pool = pool;
    pool->tasks_thread = thread_p;

    pthread_create(&(thread_p)->pthread, NULL, (void *)tasks_thread_do, thread_p);
    pthread_detach((thread_p)->pthread);
    return 0;
}
static void thread_destroy(thread *thread_p) { free(thread_p); }

/* Initialise thread pool */
struct omp_thpool_ *omp_thpool_init(int num_threads) {

    /* Make new thread pool */
    omp_thpool_ *pool = (struct omp_thpool_ *)malloc(sizeof(struct omp_thpool_));
    // if (pool == NULL) {
    //     err("thpool_init(): Could not allocate memory for thread pool\n");
    //     return NULL;
    // }
    pool->keepalive = 1;
    pool->threads_num = num_threads;

    /* Job queue thread init */

    jobqueue_init(&pool->jobqueue);
    tasks_thread_init(pool);
    pool->wait_bsem = (struct bsem *)malloc(sizeof(struct bsem));
    bsem_init(pool->wait_bsem, 0);
    return pool;
}

void omp_thpool_add_work(omp_thpool_ *pool, async_task task, void *context) {
    job *newjob;

    newjob = (struct job *)malloc(sizeof(struct job));
    // if (newjob == NULL) {
    //     err("thpool_add_work(): Could not allocate memory for new job\n");
    //     return -1;
    // }

    /* add function and argument */
    newjob->task = task;
    newjob->arg = context;
    newjob->type = TIED;

    /* add job to queue */
    jobqueue_push(&pool->jobqueue, newjob);
}

void omp_thpool_add_work_untied(omp_thpool_ *pool, async_task task, void *context) {
    job *newjob;

    newjob = (struct job *)malloc(sizeof(struct job));
    // if (newjob == NULL) {
    //     err("thpool_add_work(): Could not allocate memory for new job\n");
    //     return -1;
    // }

    /* add function and argument */
    newjob->task = task;
    newjob->arg = context;
    newjob->type = UNTIED;

    /* add job to queue */
    jobqueue_push(&pool->jobqueue, newjob);
}

void omp_thpool_wait(omp_threadpool pool) {
    job *newjob;

    newjob = (struct job *)malloc(sizeof(struct job));
    // if (newjob == NULL) {
    //     err("thpool_add_work(): Could not allocate memory for new job\n");
    //     return -1;
    // }

    /* add function and argument */
    newjob->task = NULL;
    newjob->arg = NULL;
    newjob->type = WAIT;

    jobqueue_push(&pool->jobqueue, newjob);

    bsem_wait(pool->wait_bsem);
}

void omp_thpool_destroy(omp_threadpool pool) {
    /* Give one second to kill idle threads */
    // double TIMEOUT = 1.0;
    // time_t start, end;
    // double tpassed = 0.0;
    // time(&start);
    // while (tpassed < TIMEOUT) {
    //     bsem_post_all(pool->jobqueue.has_jobs);
    //     time(&end);
    //     tpassed = difftime(end, start);
    // }

    /* Job queue cleanup */
    jobqueue_destroy(&pool->jobqueue);
    // /* Deallocs */
    thread_destroy(pool->tasks_thread);
    free(pool);
}
