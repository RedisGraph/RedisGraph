/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "./select_entry_point.h"

void selectEntryPoint(AlgebraicExpression *ae, const QueryGraph *q, const FT_FilterNode *tree) {
    Vector *aliases = FilterTree_CollectAliases(tree);
    char *srcAlias = QueryGraph_GetNodeAlias(q, *ae->src_node);
    char *destAlias = QueryGraph_GetNodeAlias(q, *ae->dest_node);

    bool destFiltered = false;

    for(int i = 0; i < Vector_Size(aliases); i++) {
        char *alias;
        Vector_Get(aliases, i, &alias);
        // Source is already filtered
        bool equal = !strcmp(alias, srcAlias);
        if(equal) break;
        
        // See if dest is filtered.
        destFiltered |= (strcmp(alias, destAlias) == 0);
    }
    
    /* If we're here that means source is not filtered
     * see if dest has a filter applied to it. */
    if(destFiltered) AlgebraicExpression_Transpose(ae);

    // Clean up.
    for(int i = 0; i < Vector_Size(aliases); i++) {
        char *alias;
        Vector_Get(aliases, i, &alias);
        free(alias);
    }
    Vector_Free(aliases);
}
