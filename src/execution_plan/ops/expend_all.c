#include "expend_all.h"

OpResult ExpendAllReset(OpBase *ctx) {
    ExpendAll *expendAll = (ExpendAll*)ctx;
    
    // Clear graph
    if(expendAll->srcNode->id != NULL) {
        free(expendAll->srcNode->id);
    }
    if(expendAll->destNode->id != NULL) {
        free(expendAll->destNode->id);
    }

    // Set new iterator
    expendAll->iter = HexaStore_QueryTriplet(expendAll->hexastore, expendAll->triplet);
    if(expendAll->iter == NULL) {
        return OP_ERR;
    }
    return OP_OK;
}

OpBase* NewExpendAllOp(RedisModuleCtx *ctx, RedisModuleString *graphId, const Graph *graph) {
    return (OpBase*)NewExpendAll(ctx, graphId,graph);
}

ExpendAll* NewExpendAll(RedisModuleCtx *ctx, RedisModuleString *graphId, const Graph *graph) {
    // Assuming graph contains a single edge.
    // TODO: validate assumption
    ExpendAll *expendAll = malloc(sizeof(ExpendAll));
    
    // Get triplet from edge
    Vector_Get(graph->nodes, 0, &expendAll->srcNode);
    Vector_Get(expendAll->srcNode->outgoingEdges, 0, &expendAll->edge);
    expendAll->destNode = expendAll->edge->dest;
    expendAll->triplet = TripletFromEdge(expendAll->edge);

    // Get hexastore iterator for triplet
    expendAll->hexastore = GetHexaStore(ctx, graphId);
    
    // Set our Op operations
    expendAll->op.name = "Expend All";
    expendAll->op.next = ExpendAllConsume;
    expendAll->op.reset = ExpendAllReset;
    expendAll->op.free = ExpendAllFree;

    return expendAll;
}

/* ExpendAllConsume next operation 
 * each call will update the graph
 * returns OP_DEPLETED when no additional updates are available */
OpResult ExpendAllConsume(OpBase *opBase, Graph* graph) {
    ExpendAll *op = (ExpendAll*)opBase;

    if(op->iter == NULL) {
        return OP_DEPLETED;
    }

    Triplet *triplet = TripletIterator_Next(op->iter);
    if(triplet == NULL) {
        return OP_DEPLETED;
    }

    // Update graph.
    if(op->srcNode->id != NULL) {
        free(op->srcNode->id);
    }
    if(op->destNode->id != NULL) {
        free(op->destNode->id);
    }
    op->srcNode->id = strdup(triplet->subject);
	op->destNode->id = strdup(triplet->object);

    FreeTriplet(triplet);
    return OP_OK;
}

/* Frees ExpendAll */
void ExpendAllFree(ExpendAll *ctx) {
    TripletIterator_Free(ctx->triplet);
    free(ctx);
}