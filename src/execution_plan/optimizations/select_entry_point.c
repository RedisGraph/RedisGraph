/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./select_entry_point.h"

void selectEntryPoint(AlgebraicExpression *ae, const FT_FilterNode *tree) {
    if (ae->src_node == ae->dest_node) return;

    Vector *aliases = FilterTree_CollectAliases(tree);
    char *srcAlias = ae->src_node->alias;
    char *destAlias = ae->dest_node->alias;

    bool srcFiltered = false;
    bool destFiltered = false;
    bool srcLabeled = ae->src_node->label != NULL;
    bool destLabeled = ae->dest_node->label != NULL;

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
