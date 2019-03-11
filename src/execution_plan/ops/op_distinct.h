/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
<<<<<<< HEAD
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
=======
* This file is available under the Redis Labs Source Available License Agreement
>>>>>>> origin/master
*/

#pragma once

#include "op.h"
#include "resultset/resultset.h"
#include "util/triemap/triemap.h"

typedef struct {
    OpBase op;
    TrieMap *trie;
} OpDistinct;

OpBase* NewDistinctOp();
Record DistinctConsume(OpBase *opBase);
OpResult DistinctReset(OpBase *ctx);
void DistinctFree(OpBase *ctx);
