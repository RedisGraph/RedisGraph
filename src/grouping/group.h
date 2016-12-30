#ifndef GROUP_H_
#define GROUP_H_

#include "../rmutil/vector.h"
#include "../aggregate/agg_ctx.h"

// RETURN actor.age, actor.country, count(*), max(movie.views)
typedef struct {
    Vector* keys;                   // Vector of RedisModuleString*
    Vector* aggregationFunctions;   // Vector of AggCtx*
} Group;

// Creates a new group
Group* NewGroup(Vector* keys, Vector* funcs);

void FreeGroup(Group* group);

#endif