/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graphmetacontext.h"
#include "../util/rmalloc.h"

GraphMetaContext *GraphMetaContext_New(GraphContext *gc, const char *meta_key_name) {
	GraphMetaContext *meta = rm_malloc(sizeof(GraphMetaContext));
	meta->gc = gc;
	meta->meta_key_name = rm_strdup(meta_key_name);
	return meta;
}

void GraphMetaContext_Free(GraphMetaContext *ctx) {
	if(!ctx) return;
	if(ctx->meta_key_name) {
		rm_free(ctx->meta_key_name);
		ctx->meta_key_name = NULL;
	}
	rm_free(ctx);
}
