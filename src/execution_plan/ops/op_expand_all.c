#include "op_expand_all.h"
#include "../../hexastore/hexastore.h"

OpBase* NewExpandAllOp(RedisModuleCtx *ctx, RedisModuleString *graphId, Node *srcNode, Edge *relation, Node *destNode) {
    return (OpBase*)NewExpandAll(ctx, graphId, srcNode, relation, destNode);
}

ExpandAll* NewExpandAll(RedisModuleCtx *ctx, RedisModuleString *graphId, Node *srcNode, Edge *relation, Node *destNode) {
    ExpandAll *expandAll = malloc(sizeof(ExpandAll));

    expandAll->ctx = ctx;
    expandAll->srcNode = srcNode;
    expandAll->destNode = destNode;
    expandAll->relation = relation;
    expandAll->hexaStore = GetHexaStore(ctx, graphId);
    expandAll->iter = NULL;

    // Set our Op operations
    expandAll->op.name = "Expand All";
    expandAll->op.type = OPType_EXPAND_ALL;
    expandAll->op.next = ExpandAllConsume;
    expandAll->op.reset = ExpandAllReset;
    expandAll->op.free = ExpandAllFree;
    expandAll->op.modifies = NewVector(char*, 3);
    
    Vector_Push(expandAll->op.modifies, srcNode->alias);
    Vector_Push(expandAll->op.modifies, relation->alias);
    Vector_Push(expandAll->op.modifies, destNode->alias);

    return expandAll;
}

/* ExpandAllConsume next operation 
 * each call will update the graph
 * returns OP_DEPLETED when no additional updates are available */
OpResult ExpandAllConsume(OpBase *opBase, Graph* graph) {
    ExpandAll *op = (ExpandAll*)opBase;
    
    if(op->srcNode->id == NULL && op->destNode->id == NULL && op->relation->id == NULL) {
        return OP_DEPLETED;
    }

    if(op->iter == NULL) {
        Triplet *t = TripletFromEdge(op->relation);
        op->iter = HexaStore_QueryTriplet(op->hexaStore, t);
        FreeTriplet(t);
    }
    
    Triplet *triplet = NULL;
    if(!TripletIterator_Next(op->iter, &triplet)) {
        return OP_DEPLETED;
    }

    /* TODO: Make sure retrieved id are indeed
     * labeled under nodes / edge lables. */

    // clear ids before update
    if(op->srcNode->id != NULL) {
        free(op->srcNode->id);
    }
    if(op->relation->id != NULL) {
        free(op->relation->id);
    }
    if(op->destNode->id != NULL) {
        free(op->destNode->id);
    }

    // Update graph
    op->srcNode->id = strdup(triplet->subject);
    op->relation->id = strdup(triplet->predicate);
    op->destNode->id = strdup(triplet->object);
    
    return OP_OK;
}

OpResult ExpandAllReset(OpBase *ctx) {
    ExpandAll *op = ctx;

    if(op->srcNode->id != NULL) {
        free(op->srcNode->id);
        op->srcNode->id = NULL;
    }

    if(op->destNode->id != NULL) {
        free(op->destNode->id);
        op->destNode->id = NULL;
    }

    if(op->relation->id != NULL) {
        free(op->relation->id);
        op->relation->id = NULL;
    }
    
    if(op->iter != NULL) {
        TripletIterator_Free(op->iter);
        op->iter = NULL;
    }
    
    return OP_OK;
}

/* Frees ExpandAll */
void ExpandAllFree(OpBase *ctx) {
    ExpandAll *op = ctx;
    if(op->iter != NULL) {
        TripletIterator_Free(op->iter);
    }
    free(op);
}