#include "op_expand_into.h"

void NewExpandIntoOp(RedisModuleCtx *ctx, RedisModuleString *graphId, Node *srcNode, Edge *relation, Node *destNode, OpBase **op) {
    *op = (OpBase *)NewExpandInto(ctx, graphId, srcNode, relation, destNode);
}

ExpandInto* NewExpandInto(RedisModuleCtx *ctx, RedisModuleString *graphId, Node *srcNode, Edge *relation, Node *destNode) {
    ExpandInto *expandInto = calloc(1, sizeof(ExpandInto));

    expandInto->srcNode = srcNode;
    expandInto->destNode = destNode;
    expandInto->relation = relation;
    expandInto->refreshAfterPass = 0;
    expandInto->ctx = ctx;
    expandInto->hexaStore = GetHexaStore(ctx, graphId);

    // Set our Op operations
    expandInto->op.name = "Expand Into";
    expandInto->op.next = ExpandIntoConsume;
    expandInto->op.reset = ExpandIntoReset;
    expandInto->op.free = ExpandIntoFree;

    return expandInto;
}

OpResult ExpandIntoConsume(OpBase *opBase, Graph* graph) {
    ExpandInto *op = opBase;
    
    // both src and dest nodes must have an ID.
    if(op->srcNode->id == NULL || op->destNode->id == NULL) {
        return OP_DEPLETED;
    }

    // should we require a data refresh
    if(op->refreshAfterPass) {
        op->refreshAfterPass = 0;
        return OP_REFRESH;
    }

    /* check to see if there's a connection between src and dest nodes.
     * assuming there could be only a single edge of type X
     * connecting src to dest nodes */
    // Triplet *triplet = TripletFromEdge(op->relation);

    char *prefix;
    asprintf(&prefix, "SOP:%s:%s:%s", op->srcNode->id, op->destNode->id, op->relation->relationship);

    TripletIterator *it = HexaStore_Search(op->hexaStore, prefix);
    // TripletIterator *it = HexaStore_QueryTriplet(op->hexaStore, triplet);
    // FreeTriplet(triplet);

    Triplet *res = TripletIterator_Next(it);
    TripletIterator_Free(it);

    // no connection between src and dest.
    if(res == NULL) {
        return OP_REFRESH;
    }
    
    /* connection found,
     * Require a data refresh next time we're called */
    op->refreshAfterPass = 1;
    return OP_OK;
}

/* Simply returns OP_OK */
OpResult ExpandIntoReset(OpBase *ctx) {
    return OP_OK;
}

/* Frees ExpandInto*/
void ExpandIntoFree(OpBase *ctx) {
    ExpandInto *op = ctx;
    free(op);
}