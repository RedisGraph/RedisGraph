#ifndef RESULTSET_CACHE_H
#define RESULTSET_CACHE_H

#include "result_set_cache_includes.h"

int getResultSet(const char* query, ResultSet* resultSet);
int storeResultSet(const char* query, ResultSet* resultSet);


#endif