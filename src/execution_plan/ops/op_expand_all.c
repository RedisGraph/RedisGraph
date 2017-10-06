#include "op_expand_all.h"
#include "../../hexastore/hexastore.h"

OpBase* NewExpandAllOp(RedisModuleCtx *ctx, Graph *g, const char *graph_name,
                       Node **src_node, Edge **relation, Node **dest_node) {
    
    return (OpBase*)NewExpandAll(ctx, g, graph_name, src_node, relation, dest_node);
}

ExpandAll* NewExpandAll(RedisModuleCtx *ctx, Graph *g, const char *graph_name,
                        Node **src_node, Edge **relation, Node **dest_node) {
    
    ExpandAll *expand_all = calloc(1, sizeof(ExpandAll));

    expand_all->ctx = ctx;
    expand_all->src_node = src_node;
    expand_all->_src_node = *src_node;
    expand_all->dest_node = dest_node;
    expand_all->_dest_node = *dest_node;
    expand_all->relation = relation;
    expand_all->_relation = *relation;
    expand_all->hexastore = GetHexaStore(ctx, graph_name);
    expand_all->triplet = NewTriplet(NULL, NULL, NULL);
    expand_all->str_triplet = sdsempty();
    expand_all->iter = HexaStore_Search(expand_all->hexastore, "");
    expand_all->state = ExpandAllUninitialized;

    // Set our Op operations
    expand_all->op.name = "Expand All";
    expand_all->op.type = OPType_EXPAND_ALL;
    expand_all->op.consume = ExpandAllConsume;
    expand_all->op.reset = ExpandAllReset;
    expand_all->op.free = ExpandAllFree;
    expand_all->op.modifies = NewVector(char*, 3);
        
    char *modified;
    /* Not completely true, but at this stage we don't know which entity
     * is given to us by previous ops, at the moment there's no harm 
     * in assuming we're modifiying all three entities, this can only effect 
     * filter operations, which is already aware of previously modified entities. */
    modified = Graph_GetNodeAlias(g, *src_node);
    Vector_Push(expand_all->op.modifies, modified);
    modified = Graph_GetEdgeAlias(g, *relation);
    Vector_Push(expand_all->op.modifies, modified);
    modified = Graph_GetNodeAlias(g, *dest_node);
    Vector_Push(expand_all->op.modifies, modified);

    return expand_all;
}

/* ExpandAllConsume next operation 
 * each call will update the graph
 * returns OP_DEPLETED when no additional updates are available */
OpResult ExpandAllConsume(OpBase *opBase, Graph* graph) {
    ExpandAll *op = (ExpandAll*)opBase;
    
    if(op->state == ExpandAllUninitialized) {
        return OP_REFRESH;
    }

    /* State reseted. */
    if(op->state == ExpandAllResetted) {
        op->triplet->subject = *(op->src_node);
        op->triplet->predicate = *(op->relation);
        op->triplet->object = *(op->dest_node);
        if(op->triplet->kind == UNKNOW) op->triplet->kind = TripletGetKind(op->triplet);

        if(op->modifies.kind == UNKNOW) {
            int s = (op->triplet->subject->id == INVALID_ENTITY_ID);
            int o = (op->triplet->object->id == INVALID_ENTITY_ID);
            int p = (op->triplet->predicate->id == INVALID_ENTITY_ID);
            op->modifies.kind = (s > 0) << 2 | (o > 0) << 1 | (p > 0);
        }

        /* Overrides current value with triplet string representation,
        * if string buffer is large enough, there will be no allocation. */
        TripletToString(op->triplet, &op->str_triplet);
        
        /* Search hexastore, reuse iterator. */
        HexaStore_Search_Iterator(op->hexastore, op->str_triplet, op->iter);

        op->state = ExpandAllConsuming;
    }
    
    Triplet *triplet = NULL;
    if(!TripletIterator_Next(op->iter, &triplet)) {
        return OP_REFRESH;
    }

    /* TODO: Make sure retrieved id are indeed
     * labeled under nodes / edge lables. */
         
    /* Update graph. */
    if(op->modifies.kind & S) {
        *op->src_node = triplet->subject;
    }
    if(op->modifies.kind & P) {
        *op->relation = triplet->predicate;
    }
    if(op->modifies.kind & O) {
        *op->dest_node = triplet->object;
    }
    return OP_OK;
}

OpResult ExpandAllReset(OpBase *ctx) {
    ExpandAll *op = (ExpandAll*)ctx;
    
    /* Reset triplet string representation. */
    op->str_triplet[0] = '\0';
	sdsupdatelen(op->str_triplet);

    if(op->modifies.kind & S) {
        *op->src_node = op->_src_node;
    }
    if(op->modifies.kind & O) {
        *op->dest_node = op->_dest_node;
    }
    if(op->modifies.kind & P) {
        *op->relation = op->_relation;
    }

    op->state = ExpandAllResetted;   /* Mark reset. */
    return OP_OK;
}

/* Frees ExpandAll */
void ExpandAllFree(OpBase *ctx) {
    ExpandAll *op = (ExpandAll*)ctx;
    if(op->iter != NULL) {
        TripletIterator_Free(op->iter);
    }
    sdsfree(op->str_triplet);
    free(op);
}
