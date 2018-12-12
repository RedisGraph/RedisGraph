/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_cartesian_product.h"

OpBase* NewCartesianProductOp() {
    CartesianProduct *cp = malloc(sizeof(CartesianProduct));
    cp->init = true;
    // Set our Op operations
    OpBase_Init(&cp->op);
    cp->op.name = "Cartesian Product";
    cp->op.type = OPType_CARTESIAN_PRODUCT;
    cp->op.consume = CartesianProductConsume;
    cp->op.reset = CartesianProductReset;
    cp->op.free = CartesianProductFree;

    return (OpBase*)cp;
}

void _ResetStreams(CartesianProduct *cp, int streamIdx) {
    // Reset each child stream, Reset propagates upwards.
    for(int i = 0; i < streamIdx; i++) OpBase_Reset(cp->op.children[i]);    
}

OpResult _PullFromStreams(CartesianProduct *cp, Record r) {
    OpResult res;
    for(int i = 1; i < cp->op.childCount; i++) {
        OpBase *child = cp->op.children[i];
        res = child->consume(child, r);
        if(res == OP_OK) {
            /* Managed to get new data
             * Reset stream (0-i) */
            _ResetStreams(cp, i);

            // Pull from resetted streams.
            for(int j = 0; j < i; j++) {
                child = cp->op.children[j];
                res = child->consume(child, r);
                if(res != OP_OK) return res;
            }
            // Ready to continue.
            return OP_OK;
        } else if(res == OP_ERR) {
            return res;
        }
    }

    /* If we're here, then we didn't manged to get new data.
     * Last stream depleted. */
    return OP_DEPLETED;
}

OpResult CartesianProductConsume(OpBase *opBase, Record r) {
    CartesianProduct *cp = (CartesianProduct*)opBase;
    OpResult res;
    OpBase *child;

    if(cp->init) {
        cp->init = false;
        for(int i = 0; i < cp->op.childCount; i++) {
            child = cp->op.children[i];
            OpResult res = child->consume(child, r);
            if(res != OP_OK) return res;
        }
        return OP_OK;
    }

    // Pull from first stream.
    child = cp->op.children[0];
    res = child->consume(child, r);

    if(res == OP_ERR) {
        return res;
    } else if(res == OP_DEPLETED) {
        res = _PullFromStreams(cp, r);
        if(res != OP_OK) return res;
    }

    return OP_OK;
}

OpResult CartesianProductReset(OpBase *opBase) {
    CartesianProduct *cp = (CartesianProduct*)opBase;
    cp->init = true;
    return OP_OK;
}

void CartesianProductFree(OpBase *opBase) {
}
