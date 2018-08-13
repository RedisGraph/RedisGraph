/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "delete_graph.h"

#include <stdlib.h>
#include <assert.h>

DeleteGraphContext *DeleteGraphContext_new(RedisModuleString *graphID, RedisModuleBlockedClient *bc) {
    DeleteGraphContext *ctx = malloc(sizeof(DeleteGraphContext));
    ctx->bc = bc;
    ctx->graphID = graphID;
    return ctx;
}

void DeleteGraphContext_free(DeleteGraphContext *ctx) {
    assert(ctx);
    free(ctx);
}
