/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "decode_io.h"
#include "current/v12/decode_v12.h"

static inline GraphContext *_LoadGraph
(
	IODecoder *decoder
) {
	return RdbLoadGraphContext_v12(decoder);
}

// load graph from pipe
GraphContext *PipeLoadGraph
(
	Pipe *p
) { 
	IODecoder *decoder = IODecoder_New(IODecoderType_Pipe, p);
	GraphContext *gc = _LoadGraph(decoder);
	IODecoder_Free(decoder);
	return gc;
}

// load graph from RDB
GraphContext *RdbLoadGraph
(
	RedisModuleIO *rdb
) {
	IODecoder *decoder = IODecoder_New(IODecoderType_RDB, rdb);
	GraphContext *gc = _LoadGraph(decoder);
	IODecoder_Free(decoder);
	return gc;
}

