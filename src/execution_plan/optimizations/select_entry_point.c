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

    bool srcFiltered = false;
    bool destFiltered = false;
    bool srcLabeled = (*ae->src_node)->label != NULL;
    bool destLabeled = (*ae->dest_node)->label != NULL;

    // See if either source or destination nodes are filtered.
    for(int i = 0; i < Vector_Size(aliases); i++) {
        char *alias;
        Vector_Get(aliases, i, &alias);

        srcFiltered = (strcmp(alias, srcAlias) == 0);
        if(srcFiltered) goto cleanup;

        // See if dest is filtered.
        if(!destFiltered) destFiltered = (strcmp(alias, destAlias) == 0);
    }

    /* Prefer filter over label 
     * if no filters are applied prefer labeled entity. */
     
    /* TODO: when additional statistics are available 
     * do not use label scan if for every node N such that 
     * (N)-[relation]->(T) N is of the same type T, and type of 
     * either source or destination node is T. */
    if(destFiltered) {
        AlgebraicExpression_Transpose(ae);
    } else if(srcLabeled) {
        goto cleanup;
    } else if(destLabeled) {
        AlgebraicExpression_Transpose(ae);
    }

cleanup:
    for(int i = 0; i < Vector_Size(aliases); i++) {
        char *alias;
        Vector_Get(aliases, i, &alias);
        free(alias);
    }
    Vector_Free(aliases);
}
