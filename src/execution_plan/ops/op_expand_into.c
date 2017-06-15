#include "op_expand_into.h"

void NewExpandIntoOp(RedisModuleCtx *ctx, const char *graphId, Node *srcNode, Edge *relation, Node *destNode, OpBase **op) {
    *op = (OpBase *)NewExpandInto(ctx, graphId, srcNode, relation, destNode);
}

ExpandInto* NewExpandInto(RedisModuleCtx *ctx, const char *graphId, Node *srcNode, Edge *relation, Node *destNode) {
    ExpandInto *expandInto = calloc(1, sizeof(ExpandInto));

    expandInto->srcNode = srcNode;
    expandInto->destNode = destNode;
    expandInto->relation = relation;
    expandInto->refreshAfterPass = 0;
    expandInto->ctx = ctx;
    expandInto->hexaStore = GetHexaStore(ctx, graphId);

    // Set our Op operations
    expandInto->op.name = "Expand Into";
    expandInto->op.type = OPType_EXPAND_INTO;
    expandInto->op.next = ExpandIntoConsume;
    expandInto->op.reset = ExpandIntoReset;
    expandInto->op.free = ExpandIntoFree;
    expandInto->op.modifies = NewVector(char*, 1);
    
    Vector_Push(expandInto->op.modifies, relation->alias);

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

    // TODO: discard this assumption and consume entire iterator.
    /* check to see if there's a connection between src and dest nodes.
     * assuming there could be only a single edge of type X
     * connecting src to dest nodes */

    char *prefix;
    asprintf(&prefix, "SOP:%s:%s:%s", op->srcNode->id, op->destNode->id, op->relation->relationship);

    TripletIterator *it = HexaStore_Search(op->hexaStore, prefix);
    Triplet *triplet = NULL;
    
    // no connection between src and dest.
    if(!TripletIterator_Next(it, &triplet)) {
        TripletIterator_Free(it);
        return OP_REFRESH;
    }

    if(op->relation->id != NULL) {
        free(op->relation->id);
    }

    // Update graph
    op->relation->id = strdup(triplet->predicate);

    // FreeTriplet(triplet);
    
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