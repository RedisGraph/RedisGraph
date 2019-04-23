/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./op_delete.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "../../util/arr.h"
#include "../../util/qsort.h"
#include <assert.h>

void _DeleteEntities(OpDelete *op) {
    /* Lock everything. */
    Graph_AcquireWriteLock(op->gc->g);

    /* We must start with edge deletion as node deletion moves nodes around. */
    size_t deletedEdgeCount = array_len(op->deleted_edges);
    for(int i = 0; i < deletedEdgeCount; i++) {
        Edge *e = op->deleted_edges + i;
        if(Graph_DeleteEdge(op->gc->g, e))
            if(op->result_set) op->result_set->stats.relationships_deleted++;
    }

    size_t deletedNodeCount = array_len(op->deleted_nodes);
    for(int i = 0; i < deletedNodeCount; i++) {
        Node *n = op->deleted_nodes + i;
        GraphContext_DeleteNodeFromIndices(op->gc, n);
        Graph_DeleteNode(op->gc->g, n);
        if(op->result_set) op->result_set->stats.nodes_deleted++;
    }

    /* Release lock. */
    Graph_ReleaseLock(op->gc->g);
}

OpBase* NewDeleteOp(const cypher_astnode_t *delete_clause, ResultSet *result_set) {
    OpDelete *op_delete = malloc(sizeof(OpDelete));

    op_delete->gc = GraphContext_GetFromTLS();
    NEWAST *ast = NEWAST_GetFromTLS();

    uint delete_count = cypher_ast_delete_nexpressions(delete_clause);
    uint *nodes_to_delete = array_new(uint, delete_count);
    uint *edges_to_delete = array_new(uint, delete_count);

    for (uint i = 0; i < delete_count; i ++) {
        const cypher_astnode_t *ast_expr = cypher_ast_delete_get_expression(delete_clause, i);
        assert(cypher_astnode_type(ast_expr) == CYPHER_AST_IDENTIFIER);
        const char *alias = cypher_ast_identifier_get_name(ast_expr);
        AR_ExpNode *entity = NEWAST_GetEntityFromAlias(ast, (char*)alias);
        assert(entity);
        uint id = entity->record_idx;
        assert(id != NOT_IN_RECORD);
        cypher_astnode_type_t type = cypher_astnode_type(entity->operand.variadic.ast_ref);
        if (type == CYPHER_AST_NODE_PATTERN) {
            nodes_to_delete = array_append(nodes_to_delete, id);
        } else if (type == CYPHER_AST_REL_PATTERN) {
            edges_to_delete = array_append(edges_to_delete, id);
        } else {
            assert(false);
        }
    }

    op_delete->node_count = array_len(nodes_to_delete);
    op_delete->edge_count = array_len(edges_to_delete);
    op_delete->nodes_to_delete = nodes_to_delete;
    op_delete->edges_to_delete = edges_to_delete;
    op_delete->deleted_nodes = array_new(Node, 32);
    op_delete->deleted_edges = array_new(Edge, 32);
    op_delete->result_set = result_set;

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

    array_free(op->nodes_to_delete);
    array_free(op->edges_to_delete);
    array_free(op->deleted_nodes);
    array_free(op->deleted_edges);
}
