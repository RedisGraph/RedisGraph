#ifndef RESULTSET_CACHE_H
#define RESULTSET_CACHE_H
#include "../redismodule.h"
#include "../resultset/resultset.h"

int getResultSet(const char* query, ResultSet* resultSet);
int storeResultSet(const char* query, ResultSet* resultSet);


#endif