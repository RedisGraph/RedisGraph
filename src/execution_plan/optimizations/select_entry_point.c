/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./select_entry_point.h"
#include "../../util/arr.h"

void selectEntryPoint(AlgebraicExpression *ae, const FT_FilterNode *tree) {
    uint *modifies = FilterTree_CollectModified(tree);
    uint modifiesCount = array_len(modifies);
    uint dest_idx = ae->dest_node_idx;
    uint src_idx = ae->src_node_idx;

    bool srcFiltered = false;
    bool destFiltered = false;
    bool srcLabeled = ae->src_node->label != NULL;
    bool destLabeled = ae->dest_node->label != NULL;

    // See if either source or destination nodes are filtered.
    for(uint i = 0; i < modifiesCount; i++) {
        uint id = modifies[i];
        srcFiltered = (id == src_idx);
        if(srcFiltered) goto cleanup;

        // See if dest is filtered.
        if(!destFiltered) destFiltered = (id == dest_idx);
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
    array_free(modifies);
}
