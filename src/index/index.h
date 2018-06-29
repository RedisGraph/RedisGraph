#ifndef __INDEX_H__
#define __INDEX_H__

#include <assert.h>
#include "../redismodule.h"
#include "../graph/graph_entity.h"
#include "../stores/store.h"
#include "../util/skiplist.h"
#include "../parser/ast.h"
#include "../graph/node.h"
#include "../filter_tree/filter_tree.h"
#include "../parser/grammar.h" // required for the definition of filter operations (LT, GT, etc)
#include "../arithmetic/tuples_iter.h"

#define INDEX_PREFIX "redis_graph_INDEX"

typedef TuplesIter IndexIterator; // TODO unused
typedef skiplistIterator IndexCreateIter;

typedef struct {
  IndexTarget target;
  skiplist *string_sl;
  skiplist *numeric_sl;
} Index;

void Index_Delete(RedisModuleCtx *ctx, const char *graphName, const char *label, const char *prop);
void Index_Create(RedisModuleCtx *ctx, const char *graphName, Graph *g, AST_IndexNode *indexOp);

/* Select an Index and range based on filters associated with Node */
IndexCreateIter* Index_IntersectFilters(RedisModuleCtx *ctx, const char *graphName, Vector *filters, const char *label);

char* Index_OpPrint(AST_IndexNode *indexNode);

void* IndexCreateIter_Next(IndexCreateIter *iter);

/*
void* IndexIterator_Next(IndexIterator *iter);
void IndexIterator_Reset(IndexIterator *iter);
void IndexIterator_Free(IndexIterator *iter);
*/

#endif
