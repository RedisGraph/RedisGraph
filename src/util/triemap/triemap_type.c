#include "triemap.h"
#include "triemap_type.h"

/* declaration of the type for redis registration. */
RedisModuleType *TrieRedisModuleType;

void *TrieMapType_RdbLoad(RedisModuleIO *rdb, int encver) {
  if (encver != 0) {
    return NULL;
  }

  // Determin how many elements are in the trie
  uint64_t elements = RedisModule_LoadUnsigned(rdb);
  TrieMap *trie = NewTrieMap();

  while (elements--) {
    size_t len;
    char *key = RedisModule_LoadStringBuffer(rdb, &len);
    TrieMap_Add(trie, key, len, NULL, NULL);
  }
  return trie;
}

void TrieMapType_RdbSave(RedisModuleIO *rdb, void *value) {
  TrieMap *trie = (TrieMap *)value;
  int count = trie->cardinality;

  RedisModule_SaveUnsigned(rdb, count);

  // Scan entire trie.
  TrieMapIterator *it = TrieMap_Iterate(trie, "", 0);
  
  char *key;
  tm_len_t len;
  void *v;
  
  while (count > 0 && TrieMapIterator_Next(it, &key, &len, &v) != 0) {
    RedisModule_SaveStringBuffer(rdb, key, len);
    count--;
  }
  TrieMapIterator_Free(it);
}

void TrieMapType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
  // TODO: implement.
}

void TrieMapType_Free(void *value) {
  TrieMap *trie = value;
  TrieMap_Free(trie, NULL);
}

int TrieMapType_Register(RedisModuleCtx *ctx) {
  RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
                               .rdb_load = TrieMapType_RdbLoad,
                               .rdb_save = TrieMapType_RdbSave,
                               .aof_rewrite = TrieMapType_AofRewrite,
                               .free = TrieMapType_Free};
  
  
  TrieRedisModuleType = RedisModule_CreateDataType(ctx, "trietype1", TRIEMAP_TYPE_ENCODING_VERSION, &tm);
  if (TrieRedisModuleType == NULL) {
    return REDISMODULE_ERR;
  }
  return REDISMODULE_OK;
}