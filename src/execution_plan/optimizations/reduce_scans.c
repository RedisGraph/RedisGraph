/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./reduce_scans.h"
#include "../../util/arr.h"
#include "../ops/op_traverse.h"
#include "../ops/op_conditional_traverse.h"
#include <assert.h>

// Locates all SCAN operations within execution plan.
void _collectScanOpes(OpBase *root, OpBase ***scans) {
    if(root == NULL) return;
    
    if(root->type == OPType_ALL_NODE_SCAN ||
       root->type == OPType_NODE_BY_LABEL_SCAN ||
       root->type == OPType_INDEX_SCAN) {
           *scans = array_append(*scans, root);
    }
    
    for(int i = 0; i < root->childCount; i++) {
        _collectScanOpes(root->children[i], scans);
    }
}

void _reduceScans(ExecutionPlan *plan, OpBase *scan) {
    if(!scan->children || scan->childCount == 0) return;
    
    /* Find taps above scan.
     * a tap is an operation which is a source of data
     * for example SCAN, UNWIND, PROCEDURE_CALL operation. */
    OpBase **taps = array_new(OpBase*, 0);
    ExecutionPlan_LocateTaps(scan->children[0], &taps);
    
    // No taps.
    int taps_count = array_len(taps);
    if(taps_count == 0) {
        array_free(taps);
        return;
    }

    /* See if one of the taps modifies the same entity as
     * current scan operation. */
    char *scanned_entity;
    assert(Vector_Size(scan->modifies) == 1);
    Vector_Get(scan->modifies, 0, &scanned_entity);

    for(int i = 0; i < taps_count; i++) {
        int j = 0;
        OpBase *tap = taps[i];
        for(; j < Vector_Size(tap->modifies); j++) {
            char *set_entity;
            Vector_Get(tap->modifies, j, &set_entity);
            if(strcmp(set_entity, scanned_entity) == 0) {
                // Entity already set remove SCAN.
                ExecutionPlan_RemoveOp(plan, scan);
                break;
            }
        }
        if(j < Vector_Size(tap->modifies)) break;
    }

    array_free(taps);
}

void reduceScans(ExecutionPlan *plan) {
    OpBase **scans = array_new(OpBase*, 0);
    _collectScanOpes(plan->root, &scans);

    int scan_ops_count = array_len(scans);

    if(scan_ops_count) {
        for(int i = 0; i < scan_ops_count; i++) {
            OpBase *scan = scans[i];
            _reduceScans(plan, scan);
        }        
    }

    array_free(scans);
}
