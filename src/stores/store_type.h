/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef STORE_TYPE_H
#define STORE_TYPE_H

#include "../redismodule.h"

void* StoreType_RdbLoad(RedisModuleIO *rdb, int encver);
void StoreType_RdbSave(RedisModuleIO *rdb, void *value);

#endif
