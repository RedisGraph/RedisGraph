#include "triplet.h"
#include "hexastore.h"
#include "../util/triemap/triemap_type.h"

void FakeFree(void* element) {

}

HexaStore *_NewHexaStore() {
	return NewTrieMap();
}

HexaStore *GetHexaStore(RedisModuleCtx *ctx, const char *id) {
	HexaStore *hexaStore = NULL;
	
	RedisModuleString *rmId = RedisModule_CreateString(ctx, id, strlen(id));
	RedisModuleKey *key = RedisModule_OpenKey(ctx, rmId, REDISMODULE_WRITE);
	RedisModule_FreeString(ctx, rmId);
	
	int type = RedisModule_KeyType(key);

	if (type == REDISMODULE_KEYTYPE_EMPTY) {
		hexaStore = _NewHexaStore();
		RedisModule_ModuleTypeSetValue(key, TrieRedisModuleType, hexaStore);
	}

	hexaStore = RedisModule_ModuleTypeGetValue(key);
	RedisModule_CloseKey(key);
	return hexaStore;
}

void HexaStore_InsertAllPerm(HexaStore* hexaStore, Triplet *t) {
	char triplet[128] 	= {0};
	char subject[32] 	= {0};
	char predicate[64] 	= {0};
	char object[32] 	= {0};
	size_t tripletLength;

	snprintf(subject, 32, "%ld", t->subject->id);
	snprintf(predicate, 64, "%s%s%ld", t->predicate->relationship, TRIPLET_PREDICATE_DELIMITER, t->predicate->id);
	snprintf(object, 32, "%ld", t->object->id);

	tripletLength = snprintf(triplet, 128, "SPO:%s:%s:%s", subject, predicate, object);
	TrieMap_Add(hexaStore, triplet, tripletLength, (void*)t, NULL);

	tripletLength = snprintf(triplet, 128, "SOP:%s:%s:%s", subject, object, predicate);
	TrieMap_Add(hexaStore, triplet, tripletLength, (void*)t, NULL);

	tripletLength = snprintf(triplet, 128, "PSO:%s:%s:%s", predicate, subject, object);
	TrieMap_Add(hexaStore, triplet, tripletLength, (void*)t, NULL);

	tripletLength = snprintf(triplet, 128, "POS:%s:%s:%s", predicate, object, subject);
	TrieMap_Add(hexaStore, triplet, tripletLength, (void*)t, NULL);

	tripletLength = snprintf(triplet, 128, "OSP:%s:%s:%s", object, subject, predicate);
	TrieMap_Add(hexaStore, triplet, tripletLength, (void*)t, NULL);

	tripletLength = snprintf(triplet, 128, "OPS:%s:%s:%s", object, predicate, subject);
	TrieMap_Add(hexaStore, triplet, tripletLength, (void*)t, NULL);
}

void HexaStore_RemoveAllPerm(HexaStore *hexaStore, const Triplet *t) {
	char triplet[128] 	= {0};
	char subject[32] 	= {0};
	char predicate[64] 	= {0};
	char object[32] 	= {0};
	size_t tripletLength;

	snprintf(subject, 32, "%ld", t->subject->id);
	snprintf(predicate, 64, "%s%s%ld", t->predicate->relationship, TRIPLET_PREDICATE_DELIMITER, t->predicate->id);
	snprintf(object, 32, "%ld", t->object->id);
    
	tripletLength = snprintf(triplet, 128, "SPO:%s:%s:%s", subject, predicate, object);
	TrieMap_Delete(hexaStore, triplet, tripletLength, FakeFree);

	tripletLength = snprintf(triplet, 128, "SOP:%s:%s:%s", subject, object, predicate);
	TrieMap_Delete(hexaStore, triplet, tripletLength, FakeFree);

	tripletLength = snprintf(triplet, 128, "PSO:%s:%s:%s", predicate, subject, object);
	TrieMap_Delete(hexaStore, triplet, tripletLength, FakeFree);

	tripletLength = snprintf(triplet, 128, "POS:%s:%s:%s", predicate, object, subject);
	TrieMap_Delete(hexaStore, triplet, tripletLength, FakeFree);

	tripletLength = snprintf(triplet, 128, "OSP:%s:%s:%s", object, subject, predicate);
	TrieMap_Delete(hexaStore, triplet, tripletLength, FakeFree);

	tripletLength = snprintf(triplet, 128, "OPS:%s:%s:%s", object, predicate, subject);
	TrieMap_Delete(hexaStore, triplet, tripletLength, (void (*)(void *))FreeTriplet);
}

// TODO: return HexaStoreIterator.
TripletIterator *HexaStore_Search(HexaStore* hexaStore, const char *prefix) {
	// TODO: free prefix_dup.
	char* prefix_dup = strdup(prefix);
	return TrieMap_Iterate(hexaStore, prefix_dup, strlen(prefix_dup));
}

void HexaStore_Search_Iterator(HexaStore* hexastore, sds prefix, TripletIterator *it) {
	TrieMapIterator_Reset(it, hexastore, prefix, sdslen(prefix));
}
