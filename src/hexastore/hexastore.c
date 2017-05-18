#include "hexastore.h"
#include "../util/triemap/triemap_type.h"

HexaStore *_NewHexaStore() {
	return NewTrieMap();
}

HexaStore *GetHexaStore(RedisModuleCtx *ctx, RedisModuleString *id) {
	HexaStore *hexaStore = NULL;
	
	RedisModuleKey *key = RedisModule_OpenKey(ctx, id, REDISMODULE_WRITE);
	int type = RedisModule_KeyType(key);

	if (type == REDISMODULE_KEYTYPE_EMPTY) {
		hexaStore = _NewHexaStore();
		RedisModule_ModuleTypeSetValue(key, TrieRedisModuleType, hexaStore);
	}

	hexaStore = RedisModule_ModuleTypeGetValue(key);
	RedisModule_CloseKey(key);
	return hexaStore;
}

/* Stores all 6 permutations of given triplet */
void HexaStore_InsertTriplet(HexaStore* hexaStore, const Triplet *triplet) {
	return HexaStore_InsertAllPerm(hexaStore, triplet->subject, triplet->predicate, triplet->object, triplet);
}

/* Create all 6 triplets from given subject, predicate and object */
void HexaStore_InsertAllPerm(HexaStore* hexaStore, const char *subject, const char *predicate, const char *object, void *value) {
	size_t sLen = strlen(subject);
    size_t pLen = strlen(predicate);
    size_t oLen = strlen(object);

    size_t bufferSize = sizeof(char) * (4 + sLen + 1 + pLen + 1 + oLen + 1);
	char *triplet = malloc(sizeof(char) * bufferSize);
	
	snprintf(triplet, bufferSize, "SPO:%s:%s:%s", subject, predicate, object);
	TrieMap_Add(hexaStore, triplet, bufferSize, value, NULL);

	snprintf(triplet, bufferSize, "SOP:%s:%s:%s", subject, object, predicate);
	TrieMap_Add(hexaStore, triplet, bufferSize, value, NULL);

	snprintf(triplet, bufferSize, "PSO:%s:%s:%s", predicate, subject, object);
	TrieMap_Add(hexaStore, triplet, bufferSize, value, NULL);

	snprintf(triplet, bufferSize, "POS:%s:%s:%s", predicate, object, subject);
	TrieMap_Add(hexaStore, triplet, bufferSize, value, NULL);

	snprintf(triplet, bufferSize, "OSP:%s:%s:%s", object, subject, predicate);
	TrieMap_Add(hexaStore, triplet, bufferSize, value, NULL);

	snprintf(triplet, bufferSize, "OPS:%s:%s:%s", object, predicate, subject);
	TrieMap_Add(hexaStore, triplet, bufferSize, value, NULL);

	free(triplet);
}

void HexaStore_RemoveAllPerm(HexaStore *hexaStore, const char *subject, const char *predicate, const char *object) {
	size_t sLen = strlen(subject);
    size_t pLen = strlen(predicate);
    size_t oLen = strlen(object);

    size_t bufferSize = sizeof(char) * (4 + sLen + 1 + pLen + 1 + oLen + 1);
	char *triplet = malloc(sizeof(char) * bufferSize);
    
	snprintf(triplet, bufferSize, "SPO:%s:%s:%s", subject, predicate, object);
	TrieMap_Delete(hexaStore, triplet, bufferSize, NULL);

	snprintf(triplet, bufferSize, "SOP:%s:%s:%s", subject, object, predicate);
	TrieMap_Delete(hexaStore, triplet, bufferSize, NULL);

	snprintf(triplet, bufferSize, "PSO:%s:%s:%s", predicate, subject, object);
	TrieMap_Delete(hexaStore, triplet, bufferSize, NULL);

	snprintf(triplet, bufferSize, "POS:%s:%s:%s", predicate, object, subject);
	TrieMap_Delete(hexaStore, triplet, bufferSize, NULL);

	snprintf(triplet, bufferSize, "OSP:%s:%s:%s", object, subject, predicate);
	TrieMap_Delete(hexaStore, triplet, bufferSize, NULL);

	snprintf(triplet, bufferSize, "OPS:%s:%s:%s", object, predicate, subject);
	TrieMap_Delete(hexaStore, triplet, bufferSize, NULL);

	free(triplet);
}

// TODO: return HexaStoreIterator.
TripletIterator *HexaStore_Search(HexaStore* hexaStore, const char *prefix) {
	// TODO: free prefix_dup.
	char* prefix_dup = strdup(prefix);
	return TrieMap_Iterate(hexaStore, prefix_dup, strlen(prefix_dup));
}

TripletIterator* HexaStore_QueryTriplet(HexaStore *hexaStore, const Triplet* triplet) {
	char *prefix = TripletToString(triplet);
    TripletIterator *iter = HexaStore_Search(hexaStore, prefix);
    free(prefix);
    return iter;
}
