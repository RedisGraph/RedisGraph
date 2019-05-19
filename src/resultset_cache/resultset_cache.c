#include "resultset_cache.h"

ResultSet *getResultSet(ResultSetCache *resultSetCache, const char *query)
{
    char hashKey[HASH_KEY_LENGTH];
    hashQuery(query, HASH_KEY_LENGTH, hashKey);
    return getFromCache(resultSetCache->raxCacheStorage, hashKey);
}
void storeResultSet(ResultSetCache *resultSetCache, const char *query, ResultSet *resultSet)
{
    if (isCacheFull(resultSetCache->lruCacheManager)){
        CacheData *evictedCacheData = evictFromCache(resultSetCache->lruCacheManager);
        removeFromCache(resultSetCache->raxCacheStorage, evictedCacheData->hashKey);
        //free RS???
        char hashKey[HASH_KEY_LENGTH];
        hashQuery(query, HASH_KEY_LENGTH, hashKey);
        CacheData *insertedCacheData = addToCache(resultSetCache->lruCacheManager, hashKey, resultSet);
        insertToCache(resultSetCache->raxCacheStorage, hashKey, insertedCacheData);
    }
}