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

// index drop context
typedef struct {
	Index idx;         // index to populate
	GraphContext *gc;  // graph holding entities to index
} IndexDropCtx;

// constraint enforce context
typedef struct {
	GraphContext *gc;  // graph object
	Constraint c;      // constraint to enforce
} ConstraintEnforceCtx;

// constraint drop context
typedef struct {
	GraphContext *gc;  // graph object
	Constraint c;      // constraint to enforce
} ConstraintDropCtx;

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

// index populate task handler
static void _indexer_idx_populate
(
	IndexPopulateCtx *ctx
) {
	Index idx = ctx->idx;
	GraphContext *gc = ctx->gc;

	// populate index
	Index_Populate(idx, ctx->gc->g);

	// we're required to hold both GIL and write lock
	// as Schema_ActivateIndex might drop an index
	RedisModuleCtx *rm_ctx = RedisModule_GetThreadSafeContext(NULL);
	RedisModule_ThreadSafeContextLock(rm_ctx);
	Graph_AcquireWriteLock(ctx->gc->g);

	// index populated, try to enable
	Index_Enable(idx);

	if(Index_Enabled(idx)) {
		Schema_ActivateIndex(ctx->s, idx);
	}

	// release locks
	Graph_ReleaseLock(ctx->gc->g);
	RedisModule_ThreadSafeContextUnlock(rm_ctx);
	RedisModule_FreeThreadSafeContext(rm_ctx);

	// decrease graph reference count
	GraphContext_DecreaseRefCount(ctx->gc);

	rm_free(ctx);
}

// index drop task handler
static void _indexer_idx_drop
(
	IndexDropCtx *ctx
) {
	RedisModuleCtx *rm_ctx = RedisModule_GetThreadSafeContext(NULL);
	RedisModule_ThreadSafeContextLock(rm_ctx);

	// TODO: expecting index pending_changes count to be either 0 or 1
	Index_Free(ctx->idx);

	RedisModule_ThreadSafeContextUnlock(rm_ctx);
	RedisModule_FreeThreadSafeContext(rm_ctx);

	// decrease graph reference count
	GraphContext_DecreaseRefCount(ctx->gc);

	rm_free(ctx);
}

// constraint enforce task handler
static void _indexer_enforce_constraint
(
	ConstraintEnforceCtx *ctx
) {
	Constraint c = ctx->c;
	GraphContext *gc = ctx->gc;
	Graph *g = GraphContext_GetGraph(gc);

//	if(Constraint_GetType(c) == CT_UNIQUE) {
//		// get constraint's supporting index
//		// make sure it is eanbled
//		Index c_idx = Constraint_GetPrivateData(c);
//		ASSERT(Index_Enabled(c_idx));
//
//		// get schema's active index
//		// make sure schema's active index is the same as the constraint's
//		int s_id = Constraint_GetSchemaID(c);
//		SchemaType t = SCHEMA_NODE;
//		if(Constraint_GetEntityType(c) == GETYPE_EDGE) t = SCHEMA_EDGE;
//
//		Schema *s = GraphContext_GetSchemaByID(gc, s_id, t);
//
//		Index s_idx = Schema_GetIndex(s, NULL, 0, IDX_EXACT_MATCH, false);
//		ASSERT(c_idx == s_idx);
//	}

	if(Constraint_GetEntityType(c) == GETYPE_NODE) {
		Constraint_EnforceNodes(c, g);
	} else {
		Constraint_EnforceEdges(c, g);
	}

	// decrease number of pending changes
	Constraint_DecPendingChanges(c);

	// decrease graph reference count
	GraphContext_DecreaseRefCount(gc);

	rm_free(ctx);
}

// constraint drop task handler
static void _indexer_drop_constraint
(
	ConstraintDropCtx *ctx
) {
	Constraint_Free(&ctx->c);

	// decrease graph reference count
	GraphContext_DecreaseRefCount(ctx->gc);

	rm_free(ctx);
}

// populate index
// this function executes on the indexer's worker thread
static void *_indexer_run
(
	void *arg
) {
	while(true) {
		// pop an item from queue
		// if queue is empty thread will be put to sleep
		IndexerTask ctx;
		_indexer_PopTask(&ctx);

		switch(ctx.op) {
			case INDEXER_IDX_POPULATE:
			{
				//RedisModule_Log(NULL, "notice", "INDEXER_IDX_POPULATE");
				IndexPopulateCtx *pdata = (IndexPopulateCtx*)ctx.pdata;
				_indexer_idx_populate(pdata);
				break;
			}
			case INDEXER_IDX_DROP:
			{
				//RedisModule_Log(NULL, "notice", "INDEXER_IDX_DROP");
				IndexDropCtx *pdata = (IndexDropCtx*)ctx.pdata;
				_indexer_idx_drop(pdata);
				break;
			}
			case INDEXER_CONSTRAINT_ENFORCE:
			{
				//RedisModule_Log(NULL, "notice", "INDEXER_CONSTRAINT_ENFORCE");
				ConstraintEnforceCtx *pdata = (ConstraintEnforceCtx*)ctx.pdata;
				_indexer_enforce_constraint(pdata);
				break;
			}
			case INDEXER_CONSTRAINT_DROP:
			{
				//RedisModule_Log(NULL, "notice", "INDEXER_CONSTRAINT_DROP");
				ConstraintDropCtx *pdata = (ConstraintDropCtx*)ctx.pdata;
				_indexer_drop_constraint(pdata);
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

	t_res = pthread_create(&indexer->t, &attr, _indexer_run, NULL);
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
	//RedisModule_Log(NULL, "notice", "Indexer_PopulateIndex");
	ASSERT(s       != NULL);
	ASSERT(gc      != NULL);
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
	Index idx,        // index to drop
	GraphContext *gc  // graph context
) {
	//RedisModule_Log(NULL, "notice", "Indexer_DropIndex");
	ASSERT(idx     != NULL);
	ASSERT(indexer != NULL);

	// create work item
	IndexDropCtx *ctx = rm_malloc(sizeof(IndexDropCtx));
	ctx->gc  = gc;
	ctx->idx = idx;

	// increase graph reference count
	// count will be reduced once this task is perfomed
	// this is done to handle the case where a graph has pending index
	// population tasks and it is being asked to be deleted
	GraphContext_IncreaseRefCount(gc);

	// place task into queue
	_indexer_AddTask(INDEXER_IDX_DROP, ctx);
}

// enforces constraint
// adds the task for enforcing the given constraint to the indexer
void Indexer_EnforceConstraint
(
	Constraint c,     // constraint to enforce
	GraphContext *gc  // graph context
) {
	//RedisModule_Log(NULL, "notice", "Indexer_EnforceConstraint");
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
	Constraint c,     // constraint to drop
	GraphContext *gc  // graph context
) {
	//RedisModule_Log(NULL, "notice", "Indexer_DropConstraint");
	ASSERT(c       != NULL);
	ASSERT(indexer != NULL);

	ConstraintDropCtx *ctx = rm_malloc(sizeof(ConstraintDropCtx));
	ctx->c  = c;
	ctx->gc = gc;

	// increase graph reference count
	// count will be reduced once this task is perfomed
	// this is done to handle the case where a graph has pending index
	// population tasks and it is being asked to be deleted
	GraphContext_IncreaseRefCount(gc);

	// place task into queue
	_indexer_AddTask(INDEXER_CONSTRAINT_DROP, ctx);
}

