#pragma once
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#if defined(__linux__)
#include <sys/prctl.h>
#endif

/* Binary semaphore */
typedef struct bsem {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int v;
} bsem;

void bsem_init(struct bsem *bsem_p, int value);
void bsem_reset(struct bsem *bsem_p);
void bsem_post(struct bsem *bsem_p);
void bsem_post_all(struct bsem *bsem_p);
void bsem_wait(struct bsem *bsem_p);
