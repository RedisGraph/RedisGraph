/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "encode_io.h"
#include "v11/encode_v11.h"

static inline void _SaveGraph
(
	IOEncoder *encoder,
	void *value
) {
	RdbSaveGraph_v11(encoder, value);
}

// graph encoder used by Redis to produce RDB
void RdbSaveGraph
(
	RedisModuleIO *rdb,  // RDB io to write graph binary representation to
	void *value          // graph to encode
) {
	IOEncoder *encoder = IOEncoder_New(IOEncoderType_RDB, rdb);
	_SaveGraph(encoder, value);
	IOEncoder_Free(encoder);
}

// graph encoder used by the GRAPH.COPY command to clone a graph
void PipeSaveGraph
(
	Pipe *pipe,  // pipe to write graph binary representation to
	void *value  // graph to encode
) {
	IOEncoder *encoder = IOEncoder_New(IOEncoderType_Pipe, pipe);
	_SaveGraph(encoder, value);
	IOEncoder_Free(encoder);
}

