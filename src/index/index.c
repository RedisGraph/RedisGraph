#include "index.h"
#include "index_type.h"

int compareNodes(const void *a, const void *b) {
  return ((Node*)a)->id - ((Node*)b)->id;
}

int compareStrings(void *a, void *b, void *ctx) {
  return strcmp(((SIValue*)a)->stringval, ((SIValue*)b)->stringval);
}

// TODO this seems inefficient
int compareNumerics(void *p1, void *p2, void *ctx) {
  double a, b;
  SIValue_ToDouble(p1, &a);
  SIValue_ToDouble(p2, &b);
  return a - b;
}

/*
 * The index must maintain its own copy of the indexed SIValue
 * so that it becomes outdated but not broken by updates to the property.
 */
void cloneKey(void **property) {
  SIValue *redirect = *property;
  *redirect = SI_Clone(*redirect);
}

void freeKey(void *key) {
  SIValue_Free(key);
}

RedisModuleKey* Index_LookupKey(RedisModuleCtx *ctx, const char *graph, const char *label, const char *property) {
  char *strKey;
  asprintf(&strKey, "%s_%s_%s_%s", INDEX_PREFIX, graph, label, property);

  RedisModuleString *rmIndexId = RedisModule_CreateString(ctx, strKey, strlen(strKey));
  free(strKey);

  RedisModuleKey *key = RedisModule_OpenKey(ctx, rmIndexId, REDISMODULE_WRITE);
  RedisModule_FreeString(ctx, rmIndexId);

  return key;
}

Index* Index_Get(RedisModuleCtx *ctx, const char *graph, const char *label, const char *property) {
  RedisModuleKey *key = Index_LookupKey(ctx, graph, label, property);
  Index *idx = NULL;

  if (RedisModule_ModuleTypeGetType(key) == IndexRedisModuleType) {
    idx = RedisModule_ModuleTypeGetValue(key);
  }
  RedisModule_CloseKey(key);

  return idx;
}

// Create and populate index for specified property
// (This function will create separate string and numeric indices if property has mixed types)
void Index_Create(RedisModuleCtx *ctx, const char *graphName, AST_IndexNode *indexOp) {
  const char *label = indexOp->target.label;
  const char *prop_str = indexOp->target.property;
  LabelStore *store = LabelStore_Get(ctx, STORE_NODE, graphName, label);

  LabelStoreIterator it;
  LabelStore_Scan(store, &it);

  char *nodeId;
  uint16_t nodeIdLen;
  Node *node;
  EntityProperty *prop;

  RedisModuleKey *key = Index_LookupKey(ctx, graphName, label, prop_str);
  // Do nothing if this index already exists
  if (RedisModule_ModuleTypeGetType(key) == IndexRedisModuleType) {
    RedisModule_CloseKey(key);
    return;
  }

  Index *index = malloc(sizeof(Index));
  RedisModule_ModuleTypeSetValue(key, IndexRedisModuleType, index);
  RedisModule_CloseKey(key);

  index->target.label = strdup(label);
  index->target.property = strdup(prop_str);

  index->string_sl = skiplistCreate(compareStrings, NULL, compareNodes, cloneKey, freeKey);
  index->numeric_sl = skiplistCreate(compareNumerics, NULL, compareNodes, cloneKey, freeKey);

  int prop_index = 0;
  while(LabelStoreIterator_Next(&it, &nodeId, &nodeIdLen, (void**)&node)) {
    // If the sought property is at a different offset than it occupied in the previous node,
    // then seek and update
    if (strcmp(prop_str, node->properties[prop_index].name)) {
      for (int i = 0; i < node->prop_count; i ++) {
        prop = node->properties + i;
        if (!strcmp(prop_str, prop->name)) {
          prop_index = i;
          break;
        }
      }
    }
    prop = node->properties + prop_index;
    // This value will be cloned within the skiplistInsert routine if necessary
    SIValue *key = &prop->value;

    if (key->type == T_STRING) {
      skiplistInsert(index->string_sl, key, node);
    } else if (key->type & SI_NUMERIC) {
      skiplistInsert(index->numeric_sl, key, node);
    } else { // This property was neither a string nor numeric value; raise a run-time error.
      assert(0);
    }
  }
}

IndexIterator* IndexIterator_CreateFromFilter(Index *idx, FT_PredicateNode *filter) {
  skiplist *target = filter->constVal.type == T_STRING ? idx->string_sl : idx->numeric_sl;
  SIValue *bound;

  switch(filter->op) {
    case EQ:
      // Only cases that set an upper bound for the skiplist require a cloned value,
      // as the lower bound is only checked within the skiplistIterateRange call here
      bound = malloc(sizeof(SIValue));
      *bound = SI_Clone(filter->constVal);
      return skiplistIterateRange(target, bound, bound, 0, 0);

    case LE:
      bound = malloc(sizeof(SIValue));
      *bound = SI_Clone(filter->constVal);
      return skiplistIterateRange(target, NULL, bound, 0, 0);

    case LT:
      bound = malloc(sizeof(SIValue));
      *bound = SI_Clone(filter->constVal);
      return skiplistIterateRange(target, NULL, bound, 0, 1);

    case GE:
      return skiplistIterateRange(target, &filter->constVal, NULL, 0, 0);

    case GT:
      return skiplistIterateRange(target, &filter->constVal, NULL, 1, 0);
  }

  return NULL;
}

/*
 * Before the ExecutionPlan has been constructed, we can analyze the FilterTree
 * to see if a scan operation can employ an index. This function will return the iterator
 * required for constructing an indexScan operation.
 */
IndexIterator* Index_IntersectFilters(RedisModuleCtx *ctx, const char *graphName, Vector *filters, const char *label) {
  FT_PredicateNode *const_filter;
  Index *idx;
  while (Vector_Size(filters) > 0) {
    Vector_Pop(filters, &const_filter);
    // Look this property up to see if it has been indexed (using the label rather than the node alias)
    if ((idx = Index_Get(ctx, graphName, label, const_filter->Lop.property)) != NULL) {
      // Build an iterator from the first matching index
      return IndexIterator_CreateFromFilter(idx, const_filter);
    }
  }

  return NULL;
}

char* Index_OpPrint(AST_IndexNode *indexNode) {
  switch(indexNode->operation) {
    case CREATE_INDEX:
      return "Create Index";
    default:
      return "Drop Index";
  }
}

void* IndexIterator_Next(IndexIterator *iter) {
  return skiplistIterator_Next(iter);
}

void IndexIterator_Reset(IndexIterator *iter) {
  skiplistIterate_Reset(iter);
}

void IndexIterator_Free(IndexIterator *iter) {
  skiplistIterate_Free(iter);
}
