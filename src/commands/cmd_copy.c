/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../util/pipe.h"
#include "../redismodule.h"
#include "../graph/graphcontext.h"
#include "../serializers/encoder/encode_graph.h"
#include "../serializers/decoders/decode_graph.h"
#include <stdbool.h>
#include <pthread.h>

// graph copy context
typedef struct {
	RedisModuleCtx *rmCtx;         // redis module context
	Pipe *pipe;                    // encode / decode pipe
	GraphContext *srcGraph;        // graph to clone
	RedisModuleString *destID;     // cloned graph ID
	RedisModuleBlockedClient *bc;  // block client
} CopyCtx;

// creates a new graph copy context
CopyCtx *CopyCtx_New
(
	RedisModuleBlockedClient *bc,  // blocked client
	RedisModuleString *srcID,      // origin graph ID
	RedisModuleString *destID      // cloned graph ID
) {
	Pipe           *pipe     = Pipe_Create();
	RedisModuleCtx *rmCtx    = RedisModule_GetThreadSafeContext(bc);
	GraphContext   *srcGraph = GraphContext_Retrieve(rmCtx, srcID, true, false);

	CopyCtx *ctx = rm_malloc(sizeof(CopyCtx));

	ctx->bc       = bc;
	ctx->pipe     = pipe;
	ctx->rmCtx    = rmCtx;
	ctx->destID   = destID;
	ctx->srcGraph = srcGraph;

	return ctx;
}

// free graph copy context
void CopyCtx_Free
(
	CopyCtx *ctx
) {
	ASSERT(ctx != NULL);

	// counter part of GraphContext_Retrieve
	GraphContext_DecreaseRefCount(ctx->srcGraph);

	// free blocked client thread safe redis context
	RedisModule_FreeThreadSafeContext(ctx->rmCtx);

	// release blocked client
	RedisModule_UnblockClient(ctx->bc, NULL);

	// close and free pipe
	Pipe_Free(ctx->pipe);

	rm_free(ctx);
}

// returns true if `id` is in keyspace
static bool _key_exists
(
	RedisModuleCtx *ctx,
	RedisModuleString *id
) {
	RedisModuleKey *key = RedisModule_OpenKey(ctx, id,
			REDISMODULE_READ | REDISMODULE_OPEN_KEY_NOTOUCH);
	bool exists = (RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_EMPTY);
	RedisModule_CloseKey(key);
	return exists;
}

// performs graph decoding
static GraphContext *_Graph_Clone_Decode
(
	Pipe *p
) {
	// decode graph from pipe
	return PipeLoadGraph(p);
}

// encode graph to pipe
static void _Graph_Clone_Encode
(
	CopyCtx *ctx
) {
	// operating in a fork process
	Pipe              *p      = ctx->pipe;
	RedisModuleCtx    *rmCtx  = ctx->rmCtx;
	GraphContext      *src    = ctx->srcGraph;
	RedisModuleString *destID = ctx->destID;

	//--------------------------------------------------------------------------
	// adjust graph name
	//--------------------------------------------------------------------------

	// rename from srcGraph to destID
	const char *cloneName = RedisModule_StringPtrLen(destID, NULL);
	src->graph_name = rm_strdup(cloneName);

	// encode to pipe
	PipeSaveGraph(p, (void*)src);

	// child all done, terminate
	RedisModule_ExitFromChild(0);
}

// clone graph
static void *_Graph_Clone
(
	void *arg
) {
	// note: executing in a dedicated thread

	ASSERT(arg != NULL);

	CopyCtx *ctx = (CopyCtx*)arg;

	Pipe              *pipe     = ctx->pipe;
	RedisModuleCtx    *rmCtx    = ctx->rmCtx;
	RedisModuleString *destID   = ctx->destID;
	GraphContext      *srcGraph = ctx->srcGraph;

	// PARENT
	bool key_exists = false;    // dest key already exists
	bool gil_acquired = false;  // did we acquired GIL ?

	// decode from pipe
	GraphContext *clone = _Graph_Clone_Decode(pipe);
	if(clone == NULL) {
		RedisModule_ReplyWithError(rmCtx, "failed to copy graph");
		goto cleanup;
	}

	// make sure destination graph doesn't already exists
	RedisModule_ThreadSafeContextLock(rmCtx); // lock GIL
	gil_acquired = true;

	key_exists = _key_exists(rmCtx, destID);
	if(key_exists) {
		// destination key already exists
		RedisModule_ReplyWithError(rmCtx, "destination key already exists");
		goto cleanup;
	}

	// register cloned graph in keyspace
	GraphContext_RegisterInKeyspace(rmCtx, clone);

	// reply to caller
	RedisModule_ReplyWithSimpleString(rmCtx, "OK");

cleanup:
	if(gil_acquired) {
		RedisModule_ThreadSafeContextUnlock(rmCtx); // unlock GIL
	}

	if(key_exists) {
		// unable to set clone graph in keyspace, free it
		GraphContext_DecreaseRefCount(clone);
	}

	CopyCtx_Free(ctx);

	return NULL;
}

// copy graph
// argv[1] source graph ID
// argv[2] destination graph ID
//
// graph copy is performed asynchronously
// this is necessery as we don't want to block Redis main thread
// in addition the source graph queryable throughout the entire cloning process
//
// the cloning process builds ontop of our encoding/decoding mechanism:
// a forked child process provides for a quick&cheap snapshot of the src graph
// it is the child process which encodes the graph onto a pipe
// collaborating with this fork is a dedicated worker thread which reads from
// the pipe and decodes the graph, once the graph is fully decoded it is set
// within Redis keyspace
int Graph_Copy
(
	RedisModuleCtx *ctx,
	RedisModuleString **argv,
	int argc
) {
	//--------------------------------------------------------------------------
	// validations
	//--------------------------------------------------------------------------

	if(argc != 3) {
		return RedisModule_WrongArity(ctx);
	}

	RedisModuleString *srcID   = argv[1];
	RedisModuleString *destID  = argv[2];
	CopyCtx           *copyCtx = NULL;

	// make sure source graph exists
	if(!GraphContext_Exists(ctx, srcID)) {
		// source graph doesn't exists
		RedisModule_ReplyWithError(ctx, "source graph doesn't exists");
		return REDISMODULE_OK;
	}

	// make sure destination graph doesn't already exists
	if(_key_exists(ctx, destID)) {
		// destination graph already exists
		RedisModule_ReplyWithError(ctx, "destination graph already exists");
		return REDISMODULE_OK;
	}

	//--------------------------------------------------------------------------
	// create graph copy context
	//--------------------------------------------------------------------------

	RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL,
			NULL, 0);

	copyCtx = CopyCtx_New(bc, srcID, destID);

	// create a fork
	// graph encoding will be done by child process
	int pid = RedisModule_Fork(NULL, NULL);
	if(pid == -1) {
		// failed to create fork
		RedisModule_ReplyWithError(ctx, "failed to create fork process");
		goto error;
	} else if(pid == 0) {
		// CHILD
		// _Graph_Clone_Encode terminates child process execution
		_Graph_Clone_Encode(copyCtx);
		ASSERT(false);  // we shouldn't get here
	}

	// PARENT

	//--------------------------------------------------------------------------
	// clone graph on a dedicated thread
	//--------------------------------------------------------------------------

	pthread_t pthread;
	if(pthread_create(&pthread, NULL, _Graph_Clone, copyCtx) != 0) {
		RedisModule_ReplyWithError(ctx, "failed to create graph clone thread");
		goto error;
	}
	pthread_detach(pthread);

	return REDISMODULE_OK;

error:
	// an error occurred
	// either fork or pthread creation failed
	CopyCtx_Free(copyCtx);
	if(pid != -1) {
		RedisModule_KillForkChild(pid);
	}

	return REDISMODULE_OK;
}

