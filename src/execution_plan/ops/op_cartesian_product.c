/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_cartesian_product.h"
#include "../../parser/newast.h"

OpBase* NewCartesianProductOp(int record_len) {
    CartesianProduct *cp = malloc(sizeof(CartesianProduct));
    cp->init = true;

    NEWAST *ast = NEWAST_GetFromLTS();
    cp->r = Record_New(NEWAST_AliasCount(ast));

    // Set our Op operations
    OpBase_Init(&cp->op);
    cp->op.name = "Cartesian Product";
    cp->op.type = OPType_CARTESIAN_PRODUCT;
    cp->op.consume = CartesianProductConsume;
    cp->op.reset = CartesianProductReset;
    cp->op.free = CartesianProductFree;

    return (OpBase*)cp;
}

static void _ResetStreams(CartesianProduct *cp, int streamIdx) {
    // Reset each child stream, Reset propagates upwards.
    for(int i = 0; i < streamIdx; i++) OpBase_Reset(cp->op.children[i]);    
}

static int _PullFromStreams(CartesianProduct *op) {
    for(int i = 1; i < op->op.childCount; i++) {
        OpBase *child = op->op.children[i];
        Record childRecord = child->consume(child);

        if(childRecord) {
            Record_Merge(&op->r, childRecord);
            Record_Free(childRecord);
            /* Managed to get new data
             * Reset streams [0-i] */
            _ResetStreams(op, i);

            // Pull from resetted streams.
            for(int j = 0; j < i; j++) {
                child = op->op.children[j];
                childRecord = child->consume(child);
                if(childRecord) {
                    Record_Merge(&op->r, childRecord);
                    Record_Free(childRecord);                    
                } else {
                    return 0;
                }
            }
            // Ready to continue.
            return 1;
        }
    }

    /* If we're here, then we didn't manged to get new data.
     * Last stream depleted. */
    return 0;
}

Record CartesianProductConsume(OpBase *opBase) {
    CartesianProduct *op = (CartesianProduct*)opBase;
    OpResult res;
    OpBase *child;
    Record childRecord;

    if(op->init) {
        op->init = false;

        for(int i = 0; i < op->op.childCount; i++) {
            child = op->op.children[i];
            childRecord = child->consume(child);
            if(!childRecord) {
                // TODO: leak childRecord.
                return NULL;
            }
            Record_Merge(&op->r, childRecord);
            Record_Free(childRecord);
        }
        return Record_Clone(op->r);
    }

    // Pull from first stream.
    child = op->op.children[0];
    childRecord = child->consume(child);
        
    if(childRecord) {
        // Managed to get data from first stream.
        Record_Merge(&op->r, childRecord);
        Record_Free(childRecord);
    } else {
        // Failed to get data from first stream,
        // try pulling other streams for data.
        if(!_PullFromStreams(op)) return NULL;
    }

    // Pass down a clone of record.
    return Record_Clone(op->r);
}

OpResult CartesianProductReset(OpBase *opBase) {
    CartesianProduct *op = (CartesianProduct*)opBase;
    op->init = true;
    return OP_OK;
}

void CartesianProductFree(OpBase *opBase) {
    CartesianProduct *op = (CartesianProduct*)opBase;
    Record_Free(op->r);
}
