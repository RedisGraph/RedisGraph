/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "index.h"
#include "index_type.h"

RedisModuleKey* _index_LookupKey(RedisModuleCtx *ctx, const char *graph, const char *label, const char *property, bool write_access) {
  char *strKey;
  int keylen = asprintf(&strKey, "%s_%s_%s_%s", INDEX_PREFIX, graph, label, property);

  RedisModuleString *rmIndexId = RedisModule_CreateString(ctx, strKey, keylen);
  free(strKey);

  int mode = write_access ? REDISMODULE_WRITE : REDISMODULE_READ;
  RedisModuleKey *key = RedisModule_OpenKey(ctx, rmIndexId, mode);
  RedisModule_FreeString(ctx, rmIndexId);

  return key;
}

/* Memory management and comparator functions that get attached to
 * string and numeric skiplists as function pointers. */
int compareNodes(NodeID a, NodeID b) {
  return a - b;
}

int compareStrings(SIValue *a, SIValue *b) {
  return strcmp(a->stringval, b->stringval);
}

int compareNumerics(SIValue *a, SIValue *b) {
  double diff = a->doubleval - b->doubleval;
  return COMPARE_RETVAL(diff);
}

/* The index must maintain its own copy of the indexed SIValue
 * so that it becomes outdated but not broken by updates to the property. */
SIValue* cloneKey(SIValue *property) {
  SIValue *clone = malloc(sizeof(SIValue));
  *clone = SI_Clone(*property);
  return clone;
}

void freeKey(SIValue *key) {
  SIValue_Free(key);
  free(key);
}

// TODO Currently not serializing indices in keyspace - see graphcontext_type comment
/* Construct key and retrieve index from Redis keyspace */
/*
 * Index* Index_Get(RedisModuleCtx *ctx, const char *graph, const char *label, const char *property) {
 *   Index *idx = NULL;
 *   // Open key with read-only access
 *   RedisModuleKey *key = _index_LookupKey(ctx, graph, label, property, false);
 *
 *   if (RedisModule_ModuleTypeGetType(key) == IndexRedisModuleType) {
 *     idx = RedisModule_ModuleTypeGetValue(key);
 *   }
 *
 *   RedisModule_CloseKey(key);
 *
 *   return idx;
 * }
 */

// TODO non-functional until serialization resolved
int Index_Delete(RedisModuleCtx *ctx, const char *graphName, const char *label, const char *prop) {
  // Open key with write access
  RedisModuleKey *key = _index_LookupKey(ctx, graphName, label, prop, true);
  if (RedisModule_ModuleTypeGetType(key) != IndexRedisModuleType) {
    // Reply with error if this key does not exist or does not correspond to an index object
    RedisModule_CloseKey(key);
    return INDEX_FAIL;
  }

  Index *idx = RedisModule_ModuleTypeGetValue(key);

  // The DeleteKey routine will free the index and skiplists
  RedisModule_DeleteKey(key);
  return INDEX_OK;
}

void initializeSkiplists(Index *index) {
  index->string_sl = skiplistCreate(compareStrings, compareNodes, cloneKey, freeKey);
  index->numeric_sl = skiplistCreate(compareNumerics, compareNodes, cloneKey, freeKey);
}

/* Index_Create allocates an Index object and populates it with a label-property pair
 * by accessing DataBlock elements referred to by a TuplesIter over a label matrix. */
Index* Index_Create(DataBlock *entities, TuplesIter *it, const char *label, const char *prop_str) {
  Index *index = malloc(sizeof(Index));

  index->label = strdup(label);
  index->property = strdup(prop_str);

  initializeSkiplists(index);

  EntityProperty *prop;

  skiplist *sl;
  EntityID entity_id;
  GraphEntity *entity;

  int found;
  int prop_index = 0;
  while(TuplesIter_next(it, NULL, &entity_id) != TuplesIter_DEPLETED) {
    entity = DataBlock_GetItem(entities, entity_id);
    // If the sought property is at a different offset than it occupied in the previous node,
    // then seek and update
    if (strcmp(prop_str, entity->properties[prop_index].name)) {
      found = 0;
      for (int i = 0; i < entity->prop_count; i ++) {
        prop = entity->properties + i;
        if (!strcmp(prop_str, prop->name)) {
          prop_index = i;
          found = 1;
          break;
        }
      }
    } else {
      found = 1;
    }
    // The targeted property does not exist on this node
    if (!found) continue;

    prop = entity->properties + prop_index;
    // This value will be cloned within the skiplistInsert routine if necessary
    SIValue *key = &prop->value;

    assert(key->type == T_STRING || key->type & SI_NUMERIC);
    sl = (key->type & SI_NUMERIC) ? index->numeric_sl: index->string_sl;
    skiplistInsert(sl, key, entity_id);
  }

  return index;
}

/* Generate an iterator with no lower or upper bound. */
IndexIter* IndexIter_Create(Index *idx, SIType type) {
  skiplist *sl = type == T_STRING ? idx->string_sl : idx->numeric_sl;
  return skiplistIterateAll(sl);
}

/* Apply a filter to an iterator, modifying the appropriate bound if
 * it narrows the iterator range.
 * Returns 1 if the filter was a comparison type that can be translated into a bound
 * (effectively, any type but '!='), which indicates that it is now redundant. */
bool IndexIter_ApplyBound(IndexIter *iter, SIValue *bound, int op) {
  return skiplistIter_UpdateBound(iter, bound, op);
}

NodeID* IndexIter_Next(IndexIter *iter) {
  return skiplistIterator_Next(iter);
}

void IndexIter_Reset(IndexIter *iter) {
  skiplistIterate_Reset(iter);
}

void IndexIter_Free(IndexIter *iter) {
  skiplistIterate_Free(iter);
}

void Index_Free(Index *idx) {
  skiplistFree(idx->string_sl);
  skiplistFree(idx->numeric_sl);
  free(idx->label);
  free(idx->property);
  free(idx);
}

