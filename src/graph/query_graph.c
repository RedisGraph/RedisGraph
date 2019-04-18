/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "query_graph.h"
#include "../arithmetic/arithmetic_expression.h"
#include "../util/arr.h"
#include <assert.h>

static void _QueryGraph_AddEdge(QueryGraph *qg, Edge *e, char *alias) {
    qg->edges = array_append(qg->edges, e);
    qg->edge_aliases = array_append(qg->edge_aliases, alias);
}

static GraphEntity* _QueryGraph_GetEntityByAlias(GraphEntity **entity_list, char **alias_list, const char* alias) {
    uint entity_count = array_len(entity_list);
    for(uint i = 0; i < entity_count; i ++) {
        char *entity_alias = alias_list[i];
        if(!strcmp(entity_alias, alias)) {
            return entity_list[i];
        }
    }
    return NULL;
}

static bool _QueryGraph_ContainsEntity(GraphEntity *entity, GraphEntity **entities) {
    uint entity_count = array_len(entities);
    for (uint i = 0; i < entity_count; i ++) {
        if(entities[i] == entity) return true;
    }
    return false;
}

static AR_ExpNode* _AST_GetNodeExpression(const NEWAST *ast, const cypher_astnode_t *node) {
    AR_ExpNode *exp = NULL;
    char *alias = NULL;

    const cypher_astnode_t *identifier = cypher_ast_node_pattern_get_identifier(node);
    if (identifier) {
        alias = (char*)cypher_ast_identifier_get_name(identifier);
        uint id = NEWAST_GetAliasID(ast, alias);
        exp = NEWAST_GetEntity(ast, id);
    } else {
        exp = NEWAST_SeekEntity(ast, node); // TODO bad
        char *alias = exp->operand.variadic.entity_alias;
    }
    assert(exp);
    return exp;
}

static void _BuildQueryGraphAddNode(const GraphContext *gc,
                             const NEWAST *ast,
                             const cypher_astnode_t *ast_entity,
                             QueryGraph *qg) {

    AR_ExpNode *exp = _AST_GetNodeExpression(ast, ast_entity);
    char *alias = exp->operand.variadic.entity_alias;

    /* Check for duplications. */
    Node *n = QueryGraph_GetNodeByAlias(qg, alias);
    unsigned int nlabels = cypher_ast_node_pattern_nlabels(ast_entity);
    const char *label = (nlabels > 0) ? cypher_ast_label_get_name(cypher_ast_node_pattern_get_label(ast_entity, 0)) : NULL;
    if(n == NULL) {
        /* Create a new node, set its properties, and add it to the graph. */
        n = Node_New(label, alias);
        QueryGraph_AddNode(qg, n, alias);
    } else {
        /* Merge nodes. */
        if (!n->label && label) n->label = strdup(label);
    }

    // Set node label ID.
    if(n->label == NULL) {
        Node_SetLabelID(n, GRAPH_NO_RELATION);
    } else {
        Schema *s = GraphContext_GetSchema(gc, n->label, SCHEMA_NODE);
        if(s) {
            Node_SetLabelID(n, s->id);
        } else {
            // Query refers to a none existing label.
            Node_SetLabelID(n, GRAPH_UNKNOWN_RELATION);
        }
    }
}

static void _BuildQueryGraphAddEdge(const GraphContext *gc,
                        const NEWAST *ast,
                        const cypher_astnode_t *entity,
                        const cypher_astnode_t *l_entity,
                        const cypher_astnode_t *r_entity,
                        QueryGraph *qg) {

    AR_ExpNode *exp = NULL;
    char *alias = NULL;
    const cypher_astnode_t *identifier = cypher_ast_rel_pattern_get_identifier(entity);
    if (identifier) {
        alias = (char*)cypher_ast_identifier_get_name(identifier);
        uint id = NEWAST_GetAliasID(ast, alias);
        exp = NEWAST_GetEntity(ast, id);
    } else {
        exp = NEWAST_SeekEntity(ast, entity); // TODO bad
        alias = exp->operand.variadic.entity_alias;
    }
    assert(exp);

    /* Check for duplications. */
    if(QueryGraph_GetEdgeByAlias(qg, alias) != NULL) return;

    const cypher_astnode_t *src_node;
    const cypher_astnode_t *dest_node;

    // Determine relation between edge and its nodes.
    if(cypher_ast_rel_pattern_get_direction(entity) == CYPHER_REL_OUTBOUND) {
        src_node = l_entity;
        dest_node = r_entity;
    } else {
        src_node = r_entity;
        dest_node = l_entity;
    }

    AR_ExpNode *src_exp = _AST_GetNodeExpression(ast, src_node);
    AR_ExpNode *dest_exp = _AST_GetNodeExpression(ast, dest_node);
    char *src_alias = src_exp->operand.variadic.entity_alias;
    char *dest_alias = dest_exp->operand.variadic.entity_alias;
    Node *src = QueryGraph_GetNodeByAlias(qg, src_alias);
    Node *dest = QueryGraph_GetNodeByAlias(qg, dest_alias);

    // TODO multi-reltype?
    uint nreltypes = cypher_ast_rel_pattern_nreltypes(entity);
    const char *reltype = NULL;
    if (nreltypes == 1) {
        reltype = cypher_ast_reltype_get_name(cypher_ast_rel_pattern_get_reltype(entity, 0));
    }
    Edge *e = Edge_New(src, dest, reltype, alias);

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

    QueryGraph_ConnectNodes(qg, src, dest, e, alias);
}

QueryGraph* QueryGraph_New(size_t node_cap, size_t edge_cap) {
    QueryGraph *qg = rm_malloc(sizeof(QueryGraph));

    qg->nodes = array_new(Node*, node_cap);
    qg->edges = array_new(Edge*, edge_cap);
    qg->node_aliases = array_new(char*, node_cap);
    qg->edge_aliases = array_new(char*, edge_cap);

    return qg;
}

void QueryGraph_AddPath(const GraphContext *gc, const NEWAST *ast, QueryGraph *qg, const cypher_astnode_t *path) {
    uint nelems = cypher_ast_pattern_path_nelements(path);
    /* Introduce nodes first. */
    for (uint i = 0; i < nelems; i += 2) {
        const cypher_astnode_t *ast_node = cypher_ast_pattern_path_get_element(path, i);
        _BuildQueryGraphAddNode(gc, ast, ast_node, qg);
    }
    for (uint i = 1; i < nelems; i += 2) {
        const cypher_astnode_t *l_entity = cypher_ast_pattern_path_get_element(path, i - 1);
        const cypher_astnode_t *r_entity = cypher_ast_pattern_path_get_element(path, i + 1);
        const cypher_astnode_t *entity = cypher_ast_pattern_path_get_element(path, i);

        _BuildQueryGraphAddEdge(gc, ast, entity, l_entity, r_entity, qg);
    }

}

/* Build a complete query graph from the clauses that can introduce entities
 * (MATCH, MERGE, and CREATE) */
// TODO Depending on how path uniqueness is specified, this may be too inclusive?
QueryGraph* BuildQueryGraph(const GraphContext *gc, const NEWAST *ast) {
    /* Predetermine graph size: (entities in both MATCH and CREATE clauses)
     * have graph object maintain an entity capacity, to avoid reallocs,
     * problem was reallocs done by CREATE clause, which invalidated old references in ExpandAll. */
    // TODO We previously counted all graph entities prior to building the QueryGraph (apparently
    // to avoid the realloc bug described here. Does this problem still exist?
    // If so, re-introduce similar logic.
    size_t node_count;
    size_t edge_count;
    node_count = edge_count = NEWAST_AliasCount(ast);
    // _Determine_Graph_Size(old_ast, &node_count, &edge_count);
    QueryGraph *qg = QueryGraph_New(node_count, edge_count);

    unsigned int clause_count = cypher_astnode_nchildren(ast->root);
    // We are interested in every path held in a MATCH or CREATE pattern,
    // and the (single) path described by a MERGE clause.
    const cypher_astnode_t *clauses[clause_count];

    // MATCH clauses
    uint match_count = NewAST_GetTopLevelClauses(ast->root, CYPHER_AST_MATCH, clauses);
    for (uint i = 0; i < match_count; i ++) {
        const cypher_astnode_t *pattern = cypher_ast_match_get_pattern(clauses[i]);
        uint npaths = cypher_ast_pattern_npaths(pattern);
        for (uint j = 0; j < npaths; j ++) {
            const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, j);
            QueryGraph_AddPath(gc, ast, qg, path);
        }
    }

    // CREATE clauses
    uint create_count = NewAST_GetTopLevelClauses(ast->root, CYPHER_AST_CREATE, clauses);
    for (uint i = 0; i < create_count; i ++) {
        const cypher_astnode_t *pattern = cypher_ast_create_get_pattern(clauses[i]);
        uint npaths = cypher_ast_pattern_npaths(pattern);
        for (uint j = 0; j < npaths; j ++) {
            const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, j);
            QueryGraph_AddPath(gc, ast, qg, path);
        }
    }

    // MERGE clauses
    uint merge_count = NewAST_GetTopLevelClauses(ast->root, CYPHER_AST_MERGE, clauses);
    for (uint i = 0; i < merge_count; i ++) {
        const cypher_astnode_t *path = cypher_ast_merge_get_pattern_path(clauses[i]);
        QueryGraph_AddPath(gc, ast, qg, path);
    }

    return qg;
}

bool QueryGraph_ContainsNode(const QueryGraph *qg, const Node *node) {
    if (array_len(qg->nodes) == 0) return false;
    return _QueryGraph_ContainsEntity((GraphEntity *)node, (GraphEntity **)qg->nodes);
}

bool QueryGraph_ContainsEdge(const QueryGraph *qg, const Edge *edge) {
    if (array_len(qg->edges) == 0) return false;
    return _QueryGraph_ContainsEntity((GraphEntity *)edge, (GraphEntity **)qg->edges);
}

void QueryGraph_AddNode(QueryGraph *qg, Node *n, char *alias) {
    qg->nodes = array_append(qg->nodes, n);
    qg->node_aliases = array_append(qg->node_aliases, alias);
}

void QueryGraph_ConnectNodes(QueryGraph *qg, Node *src, Node *dest, Edge *e, char *edge_alias) {
    assert(QueryGraph_ContainsNode(qg, src) && QueryGraph_ContainsNode(qg, dest) && !QueryGraph_ContainsEdge(qg, e));
    Node_ConnectNode(src, dest, e);
    _QueryGraph_AddEdge(qg, e, edge_alias);
}

Node* QueryGraph_GetNodeByAlias(const QueryGraph* qg, const char* alias) {
    if(alias == NULL) return NULL;
    return (Node*)_QueryGraph_GetEntityByAlias((GraphEntity **)qg->nodes, qg->node_aliases, alias);
}

Edge* QueryGraph_GetEdgeByAlias(const QueryGraph *qg, const char *alias) {
    if(alias == NULL) return NULL;
    return (Edge*)_QueryGraph_GetEntityByAlias((GraphEntity **)qg->edges, qg->edge_aliases, alias);
}

Node** QueryGraph_GetNodeRef(const QueryGraph *qg, const Node *n) {
    assert(qg && n);

    uint node_count = array_len(qg->nodes);
    for (uint i = 0; i < node_count; i ++) {
        if(n == qg->nodes[i]) return &(qg->nodes[i]);
    }

    return NULL;
}

Edge** QueryGraph_GetEdgeRef(const QueryGraph *qg, const Edge *e) {
    assert(qg && e);

    uint edge_count = array_len(qg->edges);
    for (uint i = 0; i < edge_count; i ++) {
        if(e == qg->edges[i]) return &(qg->edges[i]);
    }

    return NULL;
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

    array_free(qg->nodes);
    array_free(qg->edges);
    array_free(qg->node_aliases);
    array_free(qg->edge_aliases);
    rm_free(qg);
}
