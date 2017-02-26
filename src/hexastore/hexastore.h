#ifndef __HEXASTORE_H__
#define __HEXASTORE_H__

#include "../redismodule.h"
#include "../util/triemap/triemap.h"
#include "../triplet.h"

// TODO: find a suiteable place to store hexastores.
typedef TrieMap HexaStore;

HexaStore *GetHexaStore(RedisModuleCtx *ctx, RedisModuleString *id);

/* Create all 6 triplets from given subject, predicate and object.
 * Assuming triplets is an array of six char */
void HexaStore_InsertAllPerm(HexaStore *hexaStore, const char *subject, const char *predicate, const char *object);

void HexaStore_RemoveAllPerm(HexaStore *hexaStore, const char *subject, const char *predicate, const char *object);

// TODO: return HexaStoreIterator.
TripletIterator *HexaStore_Search(HexaStore* hexaStore, const char *prefix);

#endif