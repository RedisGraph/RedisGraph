/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __DELETE_GRAPH_H__
#define __DELETE_GRAPH_H__

#include "redismodule.h"


/* DeleteGraphContext is used to hold all information required 
 * in order to delete a graph. */
typedef struct {
    RedisModuleString *graphID;     /* Graph ID been deleted. */
    RedisModuleBlockedClient *bc;   /* Redis blocked client. */
} DeleteGraphContext;

DeleteGraphContext *DeleteGraphContext_new(RedisModuleString *graphID, RedisModuleBlockedClient *bc);
void DeleteGraphContext_free(DeleteGraphContext *ctx);

#endif
