#ifndef __HEXASTORE_H__
#define __HEXASTORE_H__

#include "../rmutil/sds.h"
#include "../redismodule.h"
#include "../util/triemap/triemap.h"
#include "triplet.h"

// TODO: find a suiteable place to store hexastores.
typedef TrieMap HexaStore;

HexaStore *_NewHexaStore();

HexaStore *GetHexaStore(RedisModuleCtx *ctx, const char *id);

/* Create all 6 triplets from given triplet. */
void HexaStore_InsertAllPerm(HexaStore* hexaStore, Triplet *t);

void HexaStore_RemoveAllPerm(HexaStore *hexaStore, const Triplet *t);

TripletIterator *HexaStore_Search(HexaStore* hexaStore, const char *prefix);

void HexaStore_Search_Iterator(HexaStore* hexastore, sds prefix, TripletIterator *it);

#endif