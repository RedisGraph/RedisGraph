#ifndef XXHASH_QUERY_HASH_H
#define XXHASH_QUERY_HASH_H

#include "../query_hash.h"

typedef struct XXHashQueryHash{
    struct QueryHash queryHash;
} XXHashQueryHash;

QueryHash *XXHashQueryHash_New();

#endif