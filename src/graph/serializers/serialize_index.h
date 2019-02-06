/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Apache License, Version 2.0,
 * modified with the Commons Clause restriction.
 */


#ifndef SERIALIZE_INDEX_H
#define SERIALIZE_INDEX_H

#include "../../redismodule.h"
#include "../../index/index.h"
#include "../graphcontext.h"

void RdbLoadIndex(RedisModuleIO *rdb, GraphContext *gc);
void RdbSaveIndex(RedisModuleIO *rdb, void *value);

#endif
