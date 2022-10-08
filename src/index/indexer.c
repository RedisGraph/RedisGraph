/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "indexer.h"
#include <assert.h>
#include <pthread.h>

// index population context
typedef struct {
	Index *idx;        // index to populate
	GraphContext *gc;  // graph holding entities to index
	uint idx_version;  // index version at the time of creation
} IndexPopulateCtx;

typedef struct {
	pthread_t t;          // worker thread handel
	pthread_mutex_t m;    // queue mutex
	pthread_mutex_t cm;   // conditional variable mutex
	pthread_cond_t c;     // conditional variable
	IndexPopulateCtx **q; // task queue
} Indexer;

// forward declarations
static IndexPopulateCtx *_indexer_PopTask(void);

static Indexer *indexer = NULL;

// populate index
// this function executes on the indexer's worker thread
static void *_index_populate
(
	void *arg
) {
	while(true) {
		// pop an item from queue
		// if queue is empty thread will be put to sleep
		IndexPopulateCtx *ctx = _indexer_PopTask();
		ASSERT(ctx != NULL);

		// only populate index if the index version at the time of the request
		// matches the current index version
		//
		// consider the following sequance of requests:
		// 1. CREATE INDEX FOR (n:A) on (n.a)
		// 2. CREATE INDEX FOR (n:Z) on (n.v)
		// 3. CREATE INDEX FOR (n:Z) on (n.y)
		//
		// when this worker thread gets to handle the second request
		// the third request already invalidated it
		// comparing index version at the time of the indexing request against
		// the current index version alows us to detect and avoid situations
		// where we will be index the same data multiple times
		if(ctx->idx_version == Index_Version(ctx->idx)) {
			// populates index in batches
			Index_Populate(ctx->idx, ctx->gc->g);
		}

		// decrease graph reference count
		GraphContext_DecreaseRefCount(ctx->gc);

		// done populating we can discard context
		rm_free(ctx);
	}

	return NULL;
}

// add task to indexer queue
static void _indexer_AddTask
(
	IndexPopulateCtx *task  // task to be added
) {
	// lock
	int res = pthread_mutex_lock(&indexer->m);
	ASSERT(res == 0);

	// add task to queue
	array_append(indexer->q, task);

	// unlock
	res = pthread_mutex_unlock(&indexer->m);
	ASSERT(res == 0);

	// signal conditional variable
	pthread_mutex_lock(&indexer->cm); 
	pthread_cond_signal(&indexer->c);
	pthread_mutex_unlock(&indexer->cm);
}

// pops a task from queue
// if queue is empty caller will be waiting on conditional variable
static IndexPopulateCtx *_indexer_PopTask(void) {
	IndexPopulateCtx *task = NULL;

	// lock queue
	int res = pthread_mutex_lock(&indexer->m);
	ASSERT(res == 0);

	// remove task to queue
	if(array_len(indexer->q) == 0) {
		// waiting for work
		// lock conditional variable mutex
		pthread_mutex_lock(&indexer->cm);

		// unlock queue mutex
		pthread_mutex_unlock(&indexer->m);

		// wait on conditional variable
		pthread_cond_wait(&indexer->c, &indexer->cm);
		pthread_mutex_unlock(&indexer->cm);

		// work been added to queue
		// lock queue
		int res = pthread_mutex_lock(&indexer->m);
		ASSERT(res == 0);
	}

	task = array_pop(indexer->q);

	// unlock
	res = pthread_mutex_unlock(&indexer->m);
	ASSERT(res == 0);

	return task;
}

// initialize indexer
// create indexer worker thread and task queue
bool Indexer_Init(void) {
	ASSERT(indexer == NULL);

	int a_res  = 0;  // attribute create result code
	int c_res  = 0;  // conditional variable create result code
	int t_res  = 0;  // thread create result code
	int m_res  = 0;  // mutex create result code
	int cm_res = 0;  // conditional variable mutex create result code

	indexer = rm_calloc(1, sizeof(Indexer));

	// create queue mutex
	m_res = pthread_mutex_init(&indexer->m, NULL);
	if(m_res != 0) {
		goto cleanup;
	}

	// create conditional variable mutex
	cm_res = pthread_mutex_init(&indexer->cm, NULL);
	if(cm_res != 0) {
		goto cleanup;
	}

	// create conditional var
	c_res = pthread_cond_init(&indexer->c, NULL);
	if(c_res != 0) {
		goto cleanup;
	}

	// create task queue
	indexer->q = array_new(IndexPopulateCtx*, 1);

	// create worker thread
	pthread_attr_t attr;
	a_res = pthread_attr_init(&attr);
	a_res = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if(a_res != 0) {
		goto cleanup;
	}

    t_res = pthread_create(&indexer->t, &attr, _index_populate, NULL);
	if(t_res != 0) {
		goto cleanup;	
	}

	pthread_attr_destroy(&attr);

	return true;

cleanup:
	if(c_res == 0) {
		pthread_cond_destroy(&indexer->c);
	}

	if(m_res == 0) {
		pthread_mutex_destroy(&indexer->m);
	}

	if(cm_res == 0) {
		pthread_mutex_destroy(&indexer->cm);
	}

	if(a_res == 0) {
		pthread_attr_destroy(&attr);
	}

	if(indexer->q != NULL) {
		array_free(indexer->q);
	}

	rm_free(indexer);

	return false;
}

// populates index asynchronously
// this function simply place the population request onto a queue
// eventually the indexer working thread will pick it up and populate the index
void Indexer_PopulateIndex
(
	GraphContext *gc, // graph to operate on
	Index *idx        // index to populate
) {
	ASSERT(indexer != NULL);

	// update index state from under-construction to populating
	assert(Index_UpdateState(idx, IDX_UNDER_CONSTRUCTION, IDX_POPULATING));

	// create work item
	IndexPopulateCtx *ctx = rm_malloc(sizeof(IndexPopulateCtx));
	ctx->gc = gc;
	ctx->idx = idx;
	ctx->idx_version = Index_Version(idx);

	// place task into queue
	_indexer_AddTask(ctx);

	// increase graph reference count
	// count will be reduced once this task is perfomed
	// this is done to handle the case where a graph has pending index
	// population tasks and it is being asked to be deleted
	GraphContext_IncreaseRefCount(gc);
}

