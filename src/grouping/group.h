#ifndef GROUP_H_
#define GROUP_H_

#include "../rmutil/vector.h"
#include "../aggregate/agg_ctx.h"

typedef struct {
    Vector* keys;                   /* Vector of SIValue* */
    Vector* aggregationFunctions;   /* Vector of AggCtx* */
} Group;

/* Creates a new group */
Group* NewGroup(Vector* keys, Vector* funcs);

void FreeGroup(Group* group);

#endif