#ifndef GROUP_H_
#define GROUP_H_

#include "../rmutil/vector.h"
#include "../aggregate/agg_ctx.h"

typedef struct {
    int key_count;
    SIValue* keys;
    Vector* aggregationFunctions;   /* Vector of AR_ExpNode*, where the root is an aggregation function. */
} Group;

/* Creates a new group */
Group* NewGroup(int key_count, SIValue* keys, Vector* funcs);

void FreeGroup(Group* group);

#endif