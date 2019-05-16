#ifndef QUERY_HASH_H
#define QUERY_HASH_H
#include <stdlib.h>
typedef struct QueryHash QueryHash;

typedef void(*hashQueryFn)(const char *query, size_t queryLength, char* buffer);
typedef void(*queryHash_FreeFn)(QueryHash *QueryHash);

struct QueryHash{
    hashQueryFn hashQuery;
    queryHash_FreeFn queryHash_Free;
};

#endif