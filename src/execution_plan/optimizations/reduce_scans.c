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

// void _reduceScans(ExecutionPlan *plan, OpBase *scan) {
    // if(!scan->children || scan->childCount == 0) return;
    
    /* Find taps above scan.
     * a tap is an operation which is a source of data
     * for example SCAN, UNWIND, PROCEDURE_CALL operation. */
    // OpBase **taps = array_new(OpBase*, 0);
    // ExecutionPlan_LocateTaps(scan->children[0], &taps);
    
    // // No taps.
    // int taps_count = array_len(taps);
    // if(taps_count == 0) {
        // array_free(taps);
        // return;
    // }

    /* See if one of the taps modifies the same entity as
     * current scan operation. */
    // assert(array_len(scan->modifies) == 1);
    // uint scanned_entity = scan->modifies[0];

    // for(int i = 0; i < taps_count; i++) {
        // int j = 0;
        // OpBase *tap = taps[i];
        // for(; j < array_len(tap->modifies); j++) {
            // uint set_entity = tap->modifies[j];
            // if(set_entity == scanned_entity) {
                // // Entity already set remove SCAN.
                // ExecutionPlan_RemoveOp(plan, scan);
                // break;
            // }
        // }
        // if(j < array_len(tap->modifies)) break;
    // }

    // array_free(taps);
// }

void reduceScans(ExecutionPlan *plan) {
    // OpBase **scans = array_new(OpBase*, 0);
    // _collectScanOpes(plan->root, &scans);

    // int scan_ops_count = array_len(scans);

    // if(scan_ops_count) {
        // for(int i = 0; i < scan_ops_count; i++) {
            // OpBase *scan = scans[i];
            // _reduceScans(plan, scan);
        // }        
    // }

    // array_free(scans);
}
