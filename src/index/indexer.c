/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "indexer.h"
#include "../redismodule.h"
#include "../util/circular_buffer.h"
#include <assert.h>
#include <pthread.h>

// operations performed by indexer
typedef enum {
	INDEXER_IDX_DROP,            // drop index
	INDEXER_IDX_POPULATE,        // populate index
	INDEXER_CONSTRAINT_DROP,     // drop index
	INDEXER_CONSTRAINT_ENFORCE,  // populate index
} IndexerOp;

// indexer task
typedef struct {
	IndexerOp op;  // type of task
	void *pdata;   // task private data
} IndexerTask;

// index population context
typedef struct {
	Schema *s;         // schema containing the index
	Index idx;         // index to populate
	GraphContext *gc;  // graph holding entities to index
} IndexPopulateCtx;

// constraint enforce context
typedef struct {
	GraphContext *gc;  // graph object
	Constraint c;      // constraint to enforce
} ConstraintEnforceCtx;

typedef struct {
	pthread_t t;         // worker thread handel
	pthread_mutex_t m;   // queue mutex
	pthread_mutex_t cm;  // conditional variable mutex
	pthread_cond_t c;    // conditional variable
	CircularBuffer q;    // task queue
} Indexer;

// forward declarations
static void _indexer_PopTask(IndexerTask *task);

static Indexer *indexer = NULL;

// populate index
// this function executes on the indexer's worker thread
static void *_index_populate
(
	void *arg
) {
	RedisModuleCtx *rm_ctx;
	while(true) {
		// pop an item from queue
		// if queue is empty thread will be put to sleep
		IndexerTask ctx;
		_indexer_PopTask(&ctx);

		switch(ctx.op) {
			case INDEXER_IDX_POPULATE:
			{
				IndexPopulateCtx *pdata = (IndexPopulateCtx*)ctx.pdata;
				Index_Populate(pdata->idx, pdata->gc->g);

				RedisModuleCtx *ctx = RedisModule_GetThreadSafeContext(NULL);
				RedisModule_ThreadSafeContextLock(ctx);
				Graph_AcquireWriteLock(pdata->gc->g);

				Schema_ActivateIndex(pdata->s, Index_Type(pdata->idx));

				Graph_ReleaseLock(pdata->gc->g);
				RedisModule_ThreadSafeContextUnlock(ctx);
				RedisModule_FreeThreadSafeContext(ctx);

				// decrease graph reference count
				GraphContext_DecreaseRefCount(pdata->gc);
				rm_free(pdata);
				break;
			}
			case INDEXER_IDX_DROP:
			{
				Index idx = (Index)ctx.pdata;
				rm_ctx = RedisModule_GetThreadSafeContext(NULL);
				RedisModule_ThreadSafeContextLock(rm_ctx);

				Index_Free(idx);

				RedisModule_ThreadSafeContextUnlock(rm_ctx);
				break;
			}
			case INDEXER_CONSTRAINT_ENFORCE:
			{
				ConstraintEnforceCtx *pdata = (ConstraintEnforceCtx*)ctx.pdata;
				Constraint c = pdata->c;
				GraphContext *gc = pdata->gc;
				Graph *g = GraphContext_GetGraph(gc);
				if(Constraint_GetEntityType(c) == GETYPE_NODE) {
					Constraint_EnforceNodes(c, g);
				} else {
					Constraint_EnforceEdges(c, g);
				}

				// decrease number of pending changes
				Constraint_DecPendingChanges(c);

				// decrease graph reference count
				GraphContext_DecreaseRefCount(gc);

				// free task private data
				rm_free(pdata);

				break;
			}
			case INDEXER_CONSTRAINT_DROP:
			{
				Constraint c = (Constraint)ctx.pdata;
				Constraint_Free(&c);
				break;
			}
			default:
				assert(false && "unknown indexer operation");
				break;
		}
	}

	return NULL;
}

// add task to indexer queue
static void _indexer_AddTask
(
	IndexerOp op,
	void *pdata
) {
	// lock
	int res = pthread_mutex_lock(&indexer->m);
	ASSERT(res == 0);

	// add task to queue
	IndexerTask	task = {.op = op, .pdata = pdata};

	res = CircularBuffer_Add(indexer->q, &task);
	ASSERT(res == 1);

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
static void _indexer_PopTask
(
	IndexerTask *task
) {
	ASSERT(task != NULL);

	// lock queue
	int res = pthread_mutex_lock(&indexer->m);
	ASSERT(res == 0);

	// remove task to queue
	if(CircularBuffer_Empty(indexer->q)) {
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

	res = CircularBuffer_Remove(indexer->q, task);
	ASSERT(res == 1);

	// unlock
	res = pthread_mutex_unlock(&indexer->m);
	ASSERT(res == 0);
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
	indexer->q = CircularBuffer_New(sizeof(IndexerTask), 256);

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
		CircularBuffer_Free(&indexer->q);
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
	Schema *s,        // schema containing the idx
	Index idx         // index to populate
) {
	ASSERT(gc      != NULL);
	ASSERT(s       != NULL);
	ASSERT(idx     != NULL);
	ASSERT(indexer != NULL);
	ASSERT(Index_Enabled(idx) == false);

	// create work item
	IndexPopulateCtx *ctx = rm_malloc(sizeof(IndexPopulateCtx));
	ctx->s   = s;
	ctx->gc  = gc;
	ctx->idx = idx;

	// increase graph reference count
	// count will be reduced once this task is perfomed
	// this is done to handle the case where a graph has pending index
	// population tasks and it is being asked to be deleted
	GraphContext_IncreaseRefCount(gc);

	// place task into queue
	_indexer_AddTask(INDEXER_IDX_POPULATE, ctx);
}

// drops index asynchronously
// this function simply place the drop request onto a queue
// eventually the indexer working thread will pick it up and drop the index
void Indexer_DropIndex
(
	Index idx  // index to drop
) {
	ASSERT(idx     != NULL);
	ASSERT(indexer != NULL);
	ASSERT(Index_Enabled(idx) == false);

	// place task into queue
	_indexer_AddTask(INDEXER_IDX_DROP, idx);
}

// enforces constraint
// adds the task for enforcing the given constraint to the indexer
void Indexer_EnforceConstraint
(
	Constraint c,     // constraint to enforce
	GraphContext *gc  // graph context
) {
	ASSERT(c       != NULL);
	ASSERT(gc      != NULL);
	ASSERT(indexer != NULL);

	ConstraintEnforceCtx *ctx = rm_malloc(sizeof(ConstraintEnforceCtx));
	ctx->c  = c;
	ctx->gc = gc;

	// increase graph reference count
	// count will be reduced once this task is perfomed
	// this is done to handle the case where a graph has pending constraint
	// enforcement tasks and it is being asked to be deleted
	GraphContext_IncreaseRefCount(gc);

	_indexer_AddTask(INDEXER_CONSTRAINT_ENFORCE, ctx);
}

// drops constraint asynchronously
// this function simply place the drop constraint request onto the queue
// eventually the indexer working thread will pick it up and drop the constraint
void Indexer_DropConstraint
(
	Constraint c  // constraint to drop
) {
	ASSERT(c       != NULL);
	ASSERT(indexer != NULL);

	// place task into queue
	_indexer_AddTask(INDEXER_CONSTRAINT_DROP, c);
}

