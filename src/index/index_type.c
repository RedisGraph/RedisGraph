#include "index_type.h"

/* declaration of the type for redis registration. */
RedisModuleType *IndexRedisModuleType;

void saveSIValue(RedisModuleIO *rdb, SIValue *v) {
  // Elem 1: key type
  RedisModule_SaveUnsigned(rdb, v->type);

  // Elem 2: key val
  if (v->type == T_STRING) {
      RedisModule_SaveStringBuffer(rdb, v->stringval, strlen(v->stringval) + 1);
  } else if (v->type && SI_NUMERIC) {
    RedisModule_SaveDouble(rdb, v->doubleval);
    /*
       TODO we compare numeric keys exclusively as doubles -
       verify that this assumption is valid. Else, something like below routine?
       RedisModule_* functions always operate on 64-bit values
    */
    /*
    double converted;
    if (SIValue_To_Double(v, &converted)) {
      RedisModule_SaveDouble(rdb, converted);
    } else {
      // Conversion to double failed
    }
    */
  }
}

SIValue loadSIValue(RedisModuleIO *rdb) {
  // Elem 1: key type
  SIValue v;
  // TODO assumes 64 bit
  v.type = RedisModule_LoadUnsigned(rdb);

  // Elem 2: key val
  if (v.type == T_STRING) {
    // TODO safe?
    v.stringval = RedisModule_LoadStringBuffer(rdb, NULL);
  } else {
    v.doubleval = RedisModule_LoadDouble(rdb);
  }

  return v;
}

void serializeSkiplistNode(RedisModuleIO *rdb, skiplistNode *sl_node) {
  // Elem 1: SIValue key
  saveSIValue(rdb, sl_node->key);

  // Elem 2: value (Node ID) count
  RedisModule_SaveUnsigned(rdb, (uint64_t)sl_node->numVals);

  // Elem 3-x: buffer of Node IDs
  GrB_Index node_id;

  for (int i = 0; i < sl_node->numVals; i ++) {
    node_id = (GrB_Index)sl_node->vals[i];
    // graph_node->id is of type `long int`
    RedisModule_SaveUnsigned(rdb, (uint64_t)node_id);
  }
}

void serializeSkiplist(RedisModuleIO *rdb, skiplist *sl) {
  // Elem: skiplist length
  RedisModule_SaveUnsigned(rdb, (uint64_t)sl->length);

  skiplistNode *node = sl->header->level[0].forward;
  while (node) {
    // Need to serialize key-val pairs; levels don't matter (and should in fact be restructured on skiplist load)
    serializeSkiplistNode(rdb, node);
    // Could free each node at this point
    node = node->level[0].forward;
  }
}

void serializeIndex(RedisModuleIO *rdb, Index *index) {
  RedisModule_SaveStringBuffer(rdb, index->label, strlen(index->label) + 1);
  RedisModule_SaveStringBuffer(rdb, index->property, strlen(index->property) + 1);

  serializeSkiplist(rdb, index->string_sl);
  serializeSkiplist(rdb, index->numeric_sl);
}

void loadSkiplist(RedisModuleIO *rdb, skiplist *sl) {
  uint64_t sl_len = RedisModule_LoadUnsigned(rdb);
  uint64_t node_num_vals;

  for (int i = 0; i < sl_len; i ++) {
    // Loop over key-val pairs
    SIValue key = loadSIValue(rdb);

    node_num_vals = RedisModule_LoadUnsigned(rdb);
    for (int j = 0; j < node_num_vals; j ++) {
      uint64_t node_id = RedisModule_LoadUnsigned(rdb);
      // printf("ID: %lu\n", node_id);
      // skiplistInsert(sl, key, node);
    }
  }
}

void *IndexType_RdbLoad(RedisModuleIO *rdb, int encver) {
  if (encver != 0) {
    return NULL;
  }

  const char *label = RedisModule_LoadStringBuffer(rdb, NULL);
  const char *property = RedisModule_LoadStringBuffer(rdb, NULL);

  Index *index = malloc(sizeof(Index));
  index->label = strdup(label);
  index->property = strdup(property);

  loadSkiplist(rdb, index->string_sl);
  loadSkiplist(rdb, index->numeric_sl);

  return index;
}

void IndexType_RdbSave(RedisModuleIO *rdb, void *value) {
  Index *index = value;

  serializeIndex(rdb, index);
}

void IndexType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
  // TODO: implement.
}

void IndexType_Free(void *value) {
  Index_Free((Index*)value);
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
