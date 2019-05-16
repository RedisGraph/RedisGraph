#include "xxhash_query_hash.h"
#include "../../util/rmalloc.h"
#include "xxhash/xxhash.h"
#include "../result_set_cache_utils.h"



void hashQuery(const char* query, size_t queryLength, char* buffer){
    unsigned long long const hash = XXH64(query, queryLength, 0);
    memcpy(buffer, &hash, HASH_KEY_LENGTH);
}

void XXHashQueryHash_Free(QueryHash *queryHash){
    rm_free((XXHashQueryHash *)queryHash);
}

QueryHash *initXXHashQueryHash(XXHashQueryHash *xxHashQueryHash)
{

    QueryHash *queryHash = (QueryHash *)xxHashQueryHash;
    queryHash->hashQuery = &hashQuery;
    queryHash->queryHash_Free = &XXHashQueryHash_Free;
    return queryHash;
}

QueryHash *XXHashQueryHash_New(){
    XXHashQueryHash *xxHashQueryHash = rm_malloc(sizeof(XXHashQueryHash));
    return initXXHashQueryHash(xxHashQueryHash);
}

