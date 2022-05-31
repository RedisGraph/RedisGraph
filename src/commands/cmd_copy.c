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

	bool               readOnly      =  true;
	bool               shouldCreate  =  false;
	RedisModuleString  *srcGraphID   =  argv[1];
	RedisModuleString  *destGraphID  =  argv[2];

	// retrieve source graph
	GraphContext *src = GraphContext_Retrieve(ctx, srcGraphID, readOnly,
			shouldCreate);

	if(src == NULL) {
		// source graph doesn't exists
		goto cleanup;
	}

	// make sure destination graph doesn't already exists
	GraphContext *dest = GraphContext_Retrieve(ctx, destGraphID, readOnly,
			shouldCreate);

	if(dest != NULL) {
		// destination graph already exists
		goto cleanup;
	}

	// create pipe
	Pipe *p = Pipe_Create();
	
	//--------------------------------------------------------------------------
	// encode to pipe
	//--------------------------------------------------------------------------

	PipeSaveGraph(p, src);

	//--------------------------------------------------------------------------
	// decode from pipe
	//--------------------------------------------------------------------------

	GraphContext *clone = PipeLoadGraph (p);
	// save graph to key space
	
	Pipe_Free(p);

cleanup:
	if(src) {
		GraphContext_DecreaseRefCount(src);
	}

	return REDISMODULE_OK;
}

