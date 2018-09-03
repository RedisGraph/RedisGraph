/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Apache License, Version 2.0,
 * modified with the Commons Clause restriction.
 */

#include "index_type.h"

/* declaration of the type for redis registration. */
RedisModuleType *IndexRedisModuleType;

void _IndexType_SaveSkiplistNode(RedisModuleIO *rdb, skiplistNode *sl_node) {
  /* Format:
   * Skiplist property key (string or double value only; type will be inferred)
   * #values
   *   value X N
   */
  SIValue *v = sl_node->key;
  if (v->type == T_STRING) {
    RedisModule_SaveStringBuffer(rdb, v->stringval, strlen(v->stringval) + 1);
  } else {
    RedisModule_SaveDouble(rdb, v->doubleval);
  }

  // Elem 2: value (Node ID) count
  RedisModule_SaveUnsigned(rdb, (uint64_t)sl_node->numVals);

  // Elem 3-x: buffer of Node IDs
  NodeID node_id;

  for (int i = 0; i < sl_node->numVals; i ++) {
    node_id = sl_node->vals[i];
    RedisModule_SaveUnsigned(rdb, node_id);
  }
}

void _IndexType_SaveSkiplist(RedisModuleIO *rdb, skiplist *sl) {
  /* Format:
   * #skiplist keys N
   *   key N
   *   #nodes with key M
   *     Nodes X M
   */
  RedisModule_SaveUnsigned(rdb, (uint64_t)sl->length);

  skiplistNode *node = sl->header->level[0].forward;
  while (node) {
    // Need to serialize key-val pairs; levels don't matter
    // (and should in fact be restructured on skiplist load)
    _IndexType_SaveSkiplistNode(rdb, node);
    node = node->level[0].forward;
  }
}

/* TODO We're using the default skiplist-building functions right now,
 * but given that we know all of our elements, it would be better to
 * switch to a non-randomized method of choosing how to build levels. */
void _IndexType_LoadSkiplist(RedisModuleIO *rdb, skiplist *sl, SIType valtype) {
  /* Format:
   * Skiplist key value (string or double value only; type will be inferred)
   * #skiplist nodes N
   *   node X N
   */
  uint64_t sl_len = RedisModule_LoadUnsigned(rdb);
  uint64_t node_num_vals;

  for (int i = 0; i < sl_len; i ++) {
    // Loop over key-val pairs
    // TODO ensure this gets freed
    SIValue *key = malloc(sizeof(SIValue));
    if (valtype == T_STRING) {
      *key = SI_StringVal(RedisModule_LoadStringBuffer(rdb, NULL));
    } else {
      *key = SI_DoubleVal(RedisModule_LoadDouble(rdb));
    }

    node_num_vals = RedisModule_LoadUnsigned(rdb);
    for (int j = 0; j < node_num_vals; j ++) {
      NodeID node_id = RedisModule_LoadUnsigned(rdb);
      skiplistInsert(sl, key, node_id);
    }
  }
}

void *IndexType_RdbLoad(RedisModuleIO *rdb, int encver) {
  /* Format:
   * ID
   * label
   * property
   * string skiplist
   * numeric skiplist
   */
  size_t id =  RedisModule_LoadUnsigned(rdb);
  const char *label = RedisModule_LoadStringBuffer(rdb, NULL);
  const char *property = RedisModule_LoadStringBuffer(rdb, NULL);

  Index *idx = malloc(sizeof(Index));
  idx->id = id;
  idx->label = strdup(label);
  idx->property = strdup(property);

  initializeSkiplists(idx);

  _IndexType_LoadSkiplist(rdb, idx->string_sl, T_STRING);
  _IndexType_LoadSkiplist(rdb, idx->numeric_sl, T_DOUBLE);

  return idx;
}

void IndexType_RdbSave(RedisModuleIO *rdb, void *value) {
  /* Format:
   * ID
   * label
   * property
   * string skiplist
   * numeric skiplist
   */
  Index *idx = (Index *)value;

  RedisModule_SaveUnsigned(rdb, idx->id);
  RedisModule_SaveStringBuffer(rdb, idx->label, strlen(idx->label) + 1);
  RedisModule_SaveStringBuffer(rdb, idx->property, strlen(idx->property) + 1);

  _IndexType_SaveSkiplist(rdb, idx->string_sl);
  _IndexType_SaveSkiplist(rdb, idx->numeric_sl);
}

void IndexType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
  // TODO: implement.
}

void IndexType_Free(void *value) {
  if (value) Index_Free((Index*)value);
}

int IndexType_Register(RedisModuleCtx *ctx) {
  RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
                               .rdb_load = IndexType_RdbLoad,
                               .rdb_save = IndexType_RdbSave,
                               .aof_rewrite = IndexType_AofRewrite,
                               .free = IndexType_Free};

  IndexRedisModuleType = RedisModule_CreateDataType(ctx, "indextype", INDEX_TYPE_ENCODING_VERSION, &tm);

  if (IndexRedisModuleType == NULL) {
    return REDISMODULE_ERR;
  }
  return REDISMODULE_OK;
}

