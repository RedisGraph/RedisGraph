/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./op_delete.h"
#include "../../util/arr.h"
#include "../../util/qsort.h"
#include <assert.h>

#define EDGES_ID_ISLT(a, b) (ENTITY_GET_ID((a)) < ENTITY_GET_ID((b)))

void _LocateEntities(OpDelete *op, QueryGraph *qg, AST_DeleteNode *ast_delete_node) {
    AST *ast = op->ast;
    for(int i = 0; i < Vector_Size(ast_delete_node->graphEntities); i++) {
        char *entity_alias;
        Vector_Get(ast_delete_node->graphEntities, i, &entity_alias);

        // Current entity is a node.
        int entityRecIdx = AST_GetAliasID(ast, entity_alias);
        Node *n = QueryGraph_GetNodeByAlias(qg, entity_alias);
        if (n != NULL) {
            op->nodes_to_delete[op->node_count++] = entityRecIdx;
        } else {
            // Current entity is an edge.
            op->edges_to_delete[op->edge_count++] = entityRecIdx;
        }
    }
}

static uint64_t _binarySearch(Edge *array, EdgeID id) {
    uint32_t edgeCount = array_len(array);
    uint32_t left = 0;
    uint32_t right = edgeCount;
    uint32_t pos;
    while(left < right) {
        pos = (right + left) / 2;
        if(ENTITY_GET_ID(array+pos) < id) {
            left = pos + 1;
        } else {
            right = pos;
        }
    }
    return left;
}

void _DeleteEntities(OpDelete *op) {
    /*  */
    Graph *g = op->gc->g;
    // Edge *_temp = array_new(Edge, 0);
    // Edge *multiple_edge = array_new(Edge, 0);
    Edge *inferred_edges = array_new(Edge, 0);

    size_t deletedNodeCount = array_len(op->deleted_nodes);
    for(int i = 0; i < deletedNodeCount; i++) {
        Node *n = op->deleted_nodes + i;

        // size_t current_len = array_len(inferred_edges);
        // TODO If two nodes are connected by multiple edges of the same type,
        // there will be a memory leak on the array (matrix value) used to contain them.
        Graph_GetNodeEdges(g, n, GRAPH_EDGE_DIR_BOTH, GRAPH_NO_RELATION, &inferred_edges);
        // size_t new_len = array_len(inferred_edges);

        // for(int j = current_len; j < new_len; j++) {
        //     Edge *e = inferred_edges + j;
        //     Graph_GetEdgesConnectingNodes(g, Edge_GetSrcNodeID(e), Edge_GetDestNodeID(e), Edge_GetRelationID(e), &_temp);
        //     if(array_len(_temp) > 1) {
        //         // Multiple edges of type r connecting src to dest.
        //         multiple_edge = array_append(multiple_edge, *e);
        //     }
        // }
    }

    // Sort inferred edges, to detect duplicates.
    // QSORT(Edge, multiple_edge, array_len(multiple_edge), EDGES_ID_ISLT);
    QSORT(Edge, inferred_edges, array_len(inferred_edges), EDGES_ID_ISLT);

    // Remove duplicates between inferred_edges and op->deleted_edges.
    size_t inferred_edges_count = array_len(inferred_edges);
    if(inferred_edges_count > 0) {
        for(int i = 0; i < array_len(op->deleted_edges); i++) {
            Edge *e = op->deleted_edges + i;
            uint64_t pos = _binarySearch(inferred_edges, ENTITY_GET_ID(e));
            if(ENTITY_GET_ID(inferred_edges + pos) == ENTITY_GET_ID(e)) {
                // Duplicate, overwrite with last edge.
                if(array_len(op->deleted_edges) > 1) {
                    op->deleted_edges[i] = array_pop(op->deleted_edges);
                } else {
                    // Remove last element.
                    array_pop(op->deleted_edges);
                }
                // Decrement the edge index so that we'll check this edge
                // against the new final edge on the next iteration.
                i--;
            }
        }
    }

    /* Lock everything. */
    Graph_AcquireWriteLock(op->gc->g);
    Graph_SetMatrixPolicy(g, DISABLED);
    
    // size_t multiple_edge_count = array_len(multiple_edge);
    // if(multiple_edge_count > 0) {
    //     EdgeID edgeID;
    //     Edge *e = multiple_edge;

    //     for(int i = 0; i < multiple_edge_count; i++) {
    //         e = multiple_edge + i;
    //         EdgeID edgeID = ENTITY_GET_ID(e);

    //         // Consume duplicates.
    //         while(i < (multiple_edge_count-1) && edgeID == ENTITY_GET_ID(multiple_edge + (i + 1))) {
    //             i++;
    //         }

    //         // TODO: Free array at position SRC/DEST.
    //     }
    // }

    /* We must start with edge deletion as node deletion moves nodes around. 
     * Quickly delete inferred edges, edges which get deleted due to 
     * node deletion, in this case Graph_DeleteEdge doesn't have to perform
     * any of its internal checks (delete_all flag) and simply delete all reference
     * to the deleted inferred edge. */
    if(inferred_edges_count > 0) {
        Edge *e;
        EdgeID edgeID;
        int edges_deleted = 0;

        for(int i = 0; i < inferred_edges_count; i++) {
            e = inferred_edges + i;
            EdgeID edgeID = ENTITY_GET_ID(e);

            // Consume duplicates.
            while(i < (inferred_edges_count-1) && edgeID == ENTITY_GET_ID(inferred_edges + (i + 1))) {
                i++;
            }

            // Force delete, skip checks.
            Graph_DeleteEdge(op->gc->g, e, true);
            // Only track deletions of unique edges.
            edges_deleted++;
        }
        if(op->result_set) op->result_set->stats.relationships_deleted += edges_deleted;
    }

    /* Explicitly delete requested edges, edges in the op->deleted_edges array
     * were explicitly marked for deletion within the query, in this case 
     * Graph_DeleteEdge need to perform additional checks before performing 
     * the deletion e.g. deleting an edge of type R connecting A to B
     * where multiple edges of type R connect A to B. */
    size_t deleted_edge_count = array_len(op->deleted_edges);
    for(int i = 0; i < deleted_edge_count; i++) {
        Edge *e = op->deleted_edges + i;
        if(Graph_DeleteEdge(op->gc->g, e, false))
            if(op->result_set) op->result_set->stats.relationships_deleted++;
    }

    /* Node deletion, 
     * at this point nodes within the op->deleted_nodes array 
     * shouldn't have any incoming nor outgoing edges. */
    for(int i = 0; i < deletedNodeCount; i++) {
        Node *n = op->deleted_nodes + i;
        GraphContext_DeleteNodeFromIndices(op->gc, n);
        // Node's edges should been already deleted.
        Graph_DeleteNode(op->gc->g, n);
        if(op->result_set) op->result_set->stats.nodes_deleted++;
    }

    /* Release lock. */
    Graph_SetMatrixPolicy(g, SYNC_AND_MINIMIZE_SPACE);
    Graph_ReleaseLock(op->gc->g);
    array_free(inferred_edges);
    // array_free(_temp);
}

OpBase* NewDeleteOp(AST_DeleteNode *ast_delete_node, QueryGraph *qg, ResultSet *result_set, AST *ast) {
    OpDelete *op_delete = malloc(sizeof(OpDelete));

    op_delete->gc = GraphContext_GetFromTLS();
    op_delete->ast = ast;
    op_delete->node_count = 0;
    op_delete->edge_count = 0;
    op_delete->nodes_to_delete = malloc(sizeof(int) * Vector_Size(ast_delete_node->graphEntities));
    op_delete->edges_to_delete = malloc(sizeof(int) * Vector_Size(ast_delete_node->graphEntities));
    op_delete->deleted_nodes = array_new(Node, 32);
    op_delete->deleted_edges = array_new(Edge, 32);
    op_delete->result_set = result_set;
    
    _LocateEntities(op_delete, qg, ast_delete_node);

    // Set our Op operations
    OpBase_Init(&op_delete->op);
    op_delete->op.name = "Delete";
    op_delete->op.type = OPType_DELETE;
    op_delete->op.consume = OpDeleteConsume;
    op_delete->op.reset = OpDeleteReset;
    op_delete->op.free = OpDeleteFree;

    return (OpBase*)op_delete;
}

Record OpDeleteConsume(OpBase *opBase) {
    OpDelete *op = (OpDelete*)opBase;
    OpBase *child = op->op.children[0];

    Record r = child->consume(child);
    if(!r) return NULL;

    /* Enqueue entities for deletion. */
    for(int i = 0; i < op->node_count; i++) {
        Node *n = Record_GetNode(r, op->nodes_to_delete[i]);
        op->deleted_nodes = array_append(op->deleted_nodes, *n);
    }

    for(int i = 0; i < op->edge_count; i++) {
        Edge *e = Record_GetEdge(r, op->edges_to_delete[i]);
        op->deleted_edges = array_append(op->deleted_edges, *e);
    }

    return r;
}

OpResult OpDeleteReset(OpBase *ctx) {
    return OP_OK;
}

void OpDeleteFree(OpBase *ctx) {
    OpDelete *op = (OpDelete*)ctx;

    _DeleteEntities(op);

    free(op->nodes_to_delete);
    free(op->edges_to_delete);
    array_free(op->deleted_nodes);
    array_free(op->deleted_edges);
}
