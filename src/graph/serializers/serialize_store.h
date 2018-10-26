/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Apache License, Version 2.0,
 * modified with the Commons Clause restriction.
 */


#ifndef SERIALIZE_STORE_H
#define SERIALIZE_STORE_H

#include "../../redismodule.h"

void* RdbLoadStore(RedisModuleIO *rdb);
void RdbSaveStore(RedisModuleIO *rdb, void *value);

#endif
