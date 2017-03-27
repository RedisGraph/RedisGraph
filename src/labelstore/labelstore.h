#ifndef __LABELSTORE_H__
#define __LABELSTORE_H__

#include "../redismodule.h"

/* For faster retrival of entities it is possible
* to mark an entity with multiple labels, for example
* we might have an entity representing San-Francisco and another
* entity representing San-Diego, when querying for all entities representing
* cities, it would be helpful if we labled both San-Francisco and San-Diego
* with the city label.
*/

#define LABEL_PREFIX "redis_graph_label"
#define LABEL_DEFAULT_SCORE 0

typedef struct {
    RedisModuleKey *key;
    int closed;
} LabelStoreIterator;

/* Returns a label iterator, which can be used
 * to scan through entire label-store. */
LabelStoreIterator* LabelGetIter(RedisModuleCtx *ctx, const RedisModuleString *graph, const RedisModuleString *label);

/* Advance iterator and retrieves current item
 * Automaticly closes iterator when end is reached. */
RedisModuleString* LabelIterNext(LabelStoreIterator *cursor);
void LabelIterFree(LabelStoreIterator *iter);

/* Labels given entity */
int LabelEntity(RedisModuleCtx *ctx, const RedisModuleString *graph, const RedisModuleString *label, const RedisModuleString *entityId);

#endif
