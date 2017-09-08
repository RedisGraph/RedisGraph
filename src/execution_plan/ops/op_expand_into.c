#include "op_expand_into.h"

void NewExpandIntoOp(RedisModuleCtx *ctx, Graph *g, const char *graph_name, Node **src_node,
                     Edge **relation, Node **dest_node, OpBase **op) {
    *op = (OpBase *)NewExpandInto(ctx, g, graph_name, src_node, relation, dest_node);
}

ExpandInto* NewExpandInto(RedisModuleCtx *ctx, Graph *g, const char *graph_name, Node **src_node,
                          Edge **relation, Node **dest_node) {
    ExpandInto *expandInto = calloc(1, sizeof(ExpandInto));

    expandInto->src_node = src_node;
    expandInto->dest_node = dest_node;
    expandInto->relation = relation;
    expandInto->_relation = *relation;
    expandInto->refreshAfterPass = 0;
    expandInto->ctx = ctx;
    expandInto->str_triplet = sdsempty();
    expandInto->hexastore = GetHexaStore(ctx, graph_name);
    expandInto->iter = HexaStore_Search(expandInto->hexastore, "");

    // Set our Op operations
    expandInto->op.name = "Expand Into";
    expandInto->op.type = OPType_EXPAND_INTO;
    expandInto->op.consume = ExpandIntoConsume;
    expandInto->op.reset = ExpandIntoReset;
    expandInto->op.free = ExpandIntoFree;
    expandInto->op.modifies = NewVector(char*, 1);
    
    Vector_Push(expandInto->op.modifies, Graph_GetEdgeAlias(g, *relation));

    return expandInto;
}

OpResult ExpandIntoConsume(OpBase *opBase, Graph* graph) {
    ExpandInto *op = (ExpandInto*)opBase;
    
    /* Both src and dest nodes must have an ID. */
    if( (*(op->src_node))->id == INVALID_ENTITY_ID ||
        (*(op->dest_node))->id == INVALID_ENTITY_ID) return OP_REFRESH;

    /* Request data refresh. */
    if(op->refreshAfterPass) {
        op->refreshAfterPass = 0;
        *op->relation = op->_relation;
        return OP_REFRESH;
    }
    
    Triplet t = {.subject = *(op->src_node),
                 .predicate = *(op->relation),
                 .object = *(op->dest_node)};

    t.kind = SO;

    /* Overrides current value with triplet string representation,
     * If string buffer is large enough, there will be no allocation. */
    TripletToString(&t, &op->str_triplet);

    /* Search hexastore, reuse iterator. */
    HexaStore_Search_Iterator(op->hexastore, op->str_triplet, op->iter);
    
    /* No connection between src and dest. */
    Triplet *triplet = NULL;
    if(!TripletIterator_Next(op->iter, &triplet)) return OP_REFRESH;

    /* Update graph. */
    *op->relation = triplet->predicate;
    
    /* Connection found,
     * Require a data refresh next time we're called */
    op->refreshAfterPass = 1;
    return OP_OK;
}

/* Simply returns OP_OK */
OpResult ExpandIntoReset(OpBase *ctx) {
    ExpandInto *op = (ExpandInto*)ctx;
    /* Restore original relation. */
    *op->relation = op->_relation;
    return OP_OK;
}

/* Frees ExpandInto*/
void ExpandIntoFree(OpBase *ctx) {
    ExpandInto *op = (ExpandInto*)ctx;
    free(op);
}
