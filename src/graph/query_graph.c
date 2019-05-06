/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "query_graph.h"
#include "../arithmetic/arithmetic_expression.h"
#include "../util/arr.h"
#include <assert.h>

static void _QueryGraph_AddASTRef(QueryGraph *qg, const cypher_astnode_t *ast_entity, void *qg_entity) {
    int rc = TrieMap_Add(qg->ast_references, (char*)&ast_entity, sizeof(ast_entity), qg_entity, TrieMap_DONT_CARE_REPLACE);
    assert(rc == 1);
}

static void _BuildQueryGraphAddNode(const GraphContext *gc,
                             const AST *ast,
                             const cypher_astnode_t *ast_entity,
                             QueryGraph *qg) {

    // TODO would it be better to build the QG map of hashes? More consistent with AST,
    // but requires accesses from elsewhere to hash first.
    // AST_IDENTIFIER identifier = AST_EntityHash(ast_entity);
    Node *n = QueryGraph_GetEntityByASTRef(qg, ast_entity);

    // Node and AST entity already mapped, do nothing
    if (n) return;

    // Check if node has been mapped using a different AST entity
    AR_ExpNode *exp = AST_GetEntity(ast, ast_entity);
    n = QueryGraph_GetEntityByASTRef(qg, exp->operand.variadic.ast_ref);

    unsigned int nlabels = cypher_ast_node_pattern_nlabels(ast_entity);
    const char *label = (nlabels > 0) ? cypher_ast_label_get_name(cypher_ast_node_pattern_get_label(ast_entity, 0)) : NULL;
    if (n) {
        // Node has been mapped - add current AST entity reference
        _QueryGraph_AddASTRef(qg, ast_entity, (void*)n);
        /* Merge nodes. */
        if (!n->label && label) n->label = strdup(label);
    } else {
        /* Create a new node, set its properties, and add it to the graph. */
        char *alias = exp->operand.variadic.entity_alias;
        n = Node_New(label, alias);
        _QueryGraph_AddASTRef(qg, ast_entity, (void*)n);
        qg->nodes = array_append(qg->nodes, n);
    }

    // Set node label ID.
    if(n->label == NULL) {
        Node_SetLabelID(n, GRAPH_NO_RELATION);
    } else {
        Schema *s = GraphContext_GetSchema(gc, n->label, SCHEMA_NODE);
        if(s) {
            Node_SetLabelID(n, s->id);
        } else {
            // Query refers to a non-existent label.
            Node_SetLabelID(n, GRAPH_UNKNOWN_RELATION);
        }
    }
}

static void _BuildQueryGraphAddEdge(const GraphContext *gc,
                        const AST *ast,
                        const cypher_astnode_t *ast_entity,
                        const cypher_astnode_t *l_entity,
                        const cypher_astnode_t *r_entity,
                        QueryGraph *qg) {

    Edge *e = QueryGraph_GetEntityByASTRef(qg, ast_entity);

    /* Check for duplications. */
    if (e) return;

    AR_ExpNode *exp = AST_GetEntity(ast, ast_entity);
    char *alias = exp->operand.variadic.entity_alias;

    const cypher_astnode_t *src_node;
    const cypher_astnode_t *dest_node;

    // Determine relation between edge and its nodes.
    if(cypher_ast_rel_pattern_get_direction(ast_entity) == CYPHER_REL_OUTBOUND) {
        src_node = l_entity;
        dest_node = r_entity;
    } else {
        src_node = r_entity;
        dest_node = l_entity;
    }

    // TODO Update logic if we stop mapping all nodes first
    Node *src = QueryGraph_GetEntityByASTRef(qg, src_node);
    Node *dest = QueryGraph_GetEntityByASTRef(qg, dest_node);

    // TODO multi-reltype?
    uint nreltypes = cypher_ast_rel_pattern_nreltypes(ast_entity);
    const char *reltype = NULL;
    if (nreltypes == 1) {
        reltype = cypher_ast_reltype_get_name(cypher_ast_rel_pattern_get_reltype(ast_entity, 0));
    }

    e = Edge_New(src, dest, reltype, alias);

    //Set edge relation ID.
    if(reltype == NULL) {
        Edge_SetRelationID(e, GRAPH_NO_RELATION);
    } else {
        Schema *s = GraphContext_GetSchema(gc, reltype, SCHEMA_EDGE);
        if(s) {
            Edge_SetRelationID(e, s->id);
        } else {
            // Query refers to a none existing label.
            Edge_SetRelationID(e, GRAPH_UNKNOWN_RELATION);
        }
    }

    _QueryGraph_AddASTRef(qg, ast_entity, (void*)e);

    Node_ConnectNode(src, dest, e);
    qg->edges = array_append(qg->edges, e);
}

QueryGraph* QueryGraph_New(size_t node_cap, size_t edge_cap) {
    QueryGraph *qg = rm_malloc(sizeof(QueryGraph));

    qg->nodes = array_new(Node*, node_cap);
    qg->edges = array_new(Edge*, edge_cap);

    qg->ast_references = NewTrieMap();

    return qg;
}

void QueryGraph_AddPath(const GraphContext *gc, const AST *ast, QueryGraph *qg, const cypher_astnode_t *path) {
    uint nelems = cypher_ast_pattern_path_nelements(path);
    /* Introduce nodes first. */
    for (uint i = 0; i < nelems; i += 2) {
        const cypher_astnode_t *ast_node = cypher_ast_pattern_path_get_element(path, i);
        _BuildQueryGraphAddNode(gc, ast, ast_node, qg);
    }
    for (uint i = 1; i < nelems; i += 2) {
        // TODO still need to introduce nodes first? not doing that would simplify this logic
        const cypher_astnode_t *l_entity = cypher_ast_pattern_path_get_element(path, i - 1);
        const cypher_astnode_t *r_entity = cypher_ast_pattern_path_get_element(path, i + 1);
        const cypher_astnode_t *entity = cypher_ast_pattern_path_get_element(path, i);

        _BuildQueryGraphAddEdge(gc, ast, entity, l_entity, r_entity, qg);
    }

}

/* Build a complete query graph from the clauses that can introduce entities
 * (MATCH, MERGE, and CREATE) */
// TODO Depending on how path uniqueness is specified, this may be too inclusive?
QueryGraph* BuildQueryGraph(const GraphContext *gc, const AST *ast) {
    /* Predetermine graph size: (entities in both MATCH and CREATE clauses)
     * have graph object maintain an entity capacity, to avoid reallocs,
     * problem was reallocs done by CREATE clause, which invalidated old references in ExpandAll. */
    // TODO We previously counted all graph entities prior to building the QueryGraph (apparently
    // to avoid the realloc bug described here. Does this problem still exist?
    // If so, re-introduce similar logic.
    size_t node_count;
    size_t edge_count;
    node_count = edge_count = AST_RecordLength(ast);
    // _Determine_Graph_Size(old_ast, &node_count, &edge_count);
    QueryGraph *qg = QueryGraph_New(node_count, edge_count);

    // We are interested in every path held in a MATCH or CREATE pattern,
    // and the (single) path described by a MERGE clause.

    const cypher_astnode_t **match_clauses = AST_CollectReferencesInRange(ast, CYPHER_AST_MATCH);
    uint match_count = array_len(match_clauses);
    for (uint i = 0; i < match_count; i ++) {
        const cypher_astnode_t *pattern = cypher_ast_match_get_pattern(match_clauses[i]);
        uint npaths = cypher_ast_pattern_npaths(pattern);
        for (uint j = 0; j < npaths; j ++) {
            const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, j);
            QueryGraph_AddPath(gc, ast, qg, path);
        }
    }
    array_free(match_clauses);

    // CREATE clauses
    const cypher_astnode_t **create_clauses = AST_CollectReferencesInRange(ast, CYPHER_AST_CREATE);
    uint create_count = array_len(create_clauses);
    for (uint i = 0; i < create_count; i ++) {
        const cypher_astnode_t *pattern = cypher_ast_create_get_pattern(create_clauses[i]);
        uint npaths = cypher_ast_pattern_npaths(pattern);
        for (uint j = 0; j < npaths; j ++) {
            const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, j);
            QueryGraph_AddPath(gc, ast, qg, path);
        }
    }
    array_free(create_clauses);

    // MERGE clauses
    const cypher_astnode_t **merge_clauses = AST_CollectReferencesInRange(ast, CYPHER_AST_MERGE);
    uint merge_count = array_len(merge_clauses);
    for (uint i = 0; i < merge_count; i ++) {
        const cypher_astnode_t *path = cypher_ast_merge_get_pattern_path(merge_clauses[i]);
        QueryGraph_AddPath(gc, ast, qg, path);
    }
    array_free(merge_clauses);

    return qg;
}

void* QueryGraph_GetEntityByASTRef(const QueryGraph *qg, const cypher_astnode_t *ref) {
    void *ge = TrieMap_Find(qg->ast_references, (void*)&ref, sizeof(ref));
    if (ge == TRIEMAP_NOTFOUND) return NULL;
    return ge;
}

/* Frees entire graph. */
void QueryGraph_Free(QueryGraph* qg) {
    if (!qg) return;

    /* Free QueryGraph nodes. */
    uint nodeCount = array_len(qg->nodes);
    for (uint i = 0; i < nodeCount; i ++) {
        Node_Free(qg->nodes[i]);
    }

    /* Free QueryGraph edges. */
    uint edgeCount = array_len(qg->edges);
    for(uint i = 0; i < edgeCount; i ++) {
        Edge_Free(qg->edges[i]);
    }

    TrieMap_Free(qg->ast_references, TrieMap_NOP_CB);
    array_free(qg->nodes);
    array_free(qg->edges);
    rm_free(qg);
}
