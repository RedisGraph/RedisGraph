#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct omp_thpool_ *omp_threadpool;

typedef void (*async_task)(void *);

omp_threadpool omp_thpool_init(int num_threads);

void omp_thpool_wait(omp_threadpool);

void omp_thpool_add_work(omp_threadpool, async_task, void *context);

void omp_thpool_add_work_untied(omp_threadpool, async_task, void *context);

void omp_thpool_destroy(omp_threadpool);

#ifdef __cplusplus
}
#endif
