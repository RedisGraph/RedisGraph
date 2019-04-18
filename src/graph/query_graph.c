/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "query_graph.h"
#include "../arithmetic/arithmetic_expression.h"
#include "../schema/schema.h"
#include <assert.h>

GraphEntity* _QueryGraph_GetEntityById(GraphEntity **entity_list, int entity_count, long int id) {
    int i;

    for(i = 0; i < entity_count; i++) {
        GraphEntity *entity = entity_list[i];
        if(ENTITY_GET_ID(entity) == id) {
            return entity;
        }
    }
    return NULL;
}

void _QueryGraph_AddEntity(GraphEntity *entity, char *alias, GraphEntity ***entity_list,
                     char ***alias_list, size_t *entity_count, size_t *entity_cap) {
    
    if(*entity_cap <= *entity_count) {
        *entity_cap *= 2; 
        *entity_list = realloc(*entity_list, sizeof(GraphEntity*) * (*entity_cap));
        *alias_list = realloc(*alias_list, sizeof(char*) * (*entity_cap));
    }

    (*entity_list)[*entity_count] = entity;
    (*alias_list)[*entity_count] = alias;
    (*entity_count)++;
}

void _QueryGraph_AddEdge(QueryGraph *g, Edge *e, char *alias) {
    _QueryGraph_AddEntity((GraphEntity*)e,
                     alias,
                     (GraphEntity ***)(&g->edges),
                     &g->edge_aliases,
                     &g->edge_count,
                     &g->edge_cap);
}

GraphEntity* _QueryGraph_GetEntityByAlias(GraphEntity **entity_list, char **alias_list, int entity_count, const char* alias) {
    for(int i = 0; i < entity_count; i++) {
        char *entity_alias = alias_list[i];
        if(strcmp(entity_alias, alias) == 0) {
            return entity_list[i];
        }
    }
    return NULL;
}

char* _QueryGraph_GetEntityAlias(GraphEntity *entity, GraphEntity **entities, char **aliases, int entity_count) {
    int i;
    for(i = 0; i < entity_count; i++) {
        if(ENTITY_GET_ID(entities[i]) == ENTITY_GET_ID(entity)) {
            return aliases[i];
        }
    }
    
    return NULL;
}

int _QueryGraph_ContainsEntity(GraphEntity *entity, GraphEntity **entities, int entity_count) {
    int i;
    for(i = 0; i < entity_count; i++) {
        GraphEntity *e = entities[i];
        if(e == entity) {
            return 1;
        }
    }
    return 0;
}

void QueryGraph_AddNode(QueryGraph *g, Node *n, char *alias) {
    _QueryGraph_AddEntity((GraphEntity*)n,
    alias,
    (GraphEntity ***)&g->nodes,
    &g->node_aliases,
    &g->node_count,
    &g->node_cap);
}

// Extend node with label and attributes from graph entity.
void _MergeNodeWithGraphEntity(Node *n, const char *label) {
    if(n->label == NULL && label != NULL) n->label = strdup(label);
}

AR_ExpNode* _AST_GetNodeExpression(const NEWAST *ast, const cypher_astnode_t *node) {
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

void _BuildQueryGraphAddNode(const GraphContext *gc,
                             const NEWAST *ast,
                             const cypher_astnode_t *ast_entity,
                             QueryGraph *qg) {
    const Graph *g = gc->g;

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
        _MergeNodeWithGraphEntity(n, label);
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

void _BuildQueryGraphAddEdge(const GraphContext *gc,
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

    const Graph *g = gc->g;
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
    QueryGraph* g = malloc(sizeof(QueryGraph));
    g->node_count = 0;
    g->edge_count = 0;
    g->node_cap = node_cap;
    g->edge_cap = edge_cap;

    g->nodes = malloc(sizeof(Node*) * node_cap);
    g->edges = malloc(sizeof(Edge*) * edge_cap);
    g->node_aliases = malloc(sizeof(char*) * node_cap);
    g->edge_aliases = malloc(sizeof(char*) * edge_cap);

    return g;
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

// TODO currently unused
void BuildQueryGraph(const GraphContext *gc, QueryGraph *qg, const cypher_astnode_t *pattern) {
    NEWAST *ast = NEWAST_GetFromTLS();
    uint npaths = cypher_ast_pattern_npaths(pattern);
    for (uint i = 0; i < npaths; i ++) {
        const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, i);
        QueryGraph_AddPath(gc, ast, qg, path);
    }
}

Node* QueryGraph_GetNodeById(const QueryGraph *g, long int id) {
    if(id == INVALID_ENTITY_ID) return NULL;
    return (Node*)_QueryGraph_GetEntityById((GraphEntity **)g->nodes, g->node_count, id);
}

Edge* QueryGraph_GetEdgeById(const QueryGraph *g, long int id) {
    if(id == INVALID_ENTITY_ID) return NULL;
    return (Edge*)_QueryGraph_GetEntityById((GraphEntity **)g->edges, g->edge_count, id);
}

int QueryGraph_ContainsNode(const QueryGraph *graph, const Node *node) {
    if(!graph->node_count) return 0;
    return _QueryGraph_ContainsEntity((GraphEntity *)node,
                                 (GraphEntity **)graph->nodes,
                                 graph->node_count);
}

int QueryGraph_ContainsEdge(const QueryGraph *graph, const Edge *edge) {
    if(!graph->edge_count) return 0;
    return _QueryGraph_ContainsEntity((GraphEntity *)edge,
                                 (GraphEntity **)graph->edges,
                                 graph->edge_count);
}

void QueryGraph_ConnectNodes(QueryGraph *g, Node *src, Node *dest, Edge *e, char *edge_alias) {
    assert(QueryGraph_ContainsNode(g, src) && QueryGraph_ContainsNode(g, dest) && !QueryGraph_ContainsEdge(g, e));
    Node_ConnectNode(src, dest, e);
    _QueryGraph_AddEdge(g, e, edge_alias);
}

Node* QueryGraph_GetNodeByAlias(const QueryGraph* g, const char* alias) {
    if(alias == NULL) return NULL;
    return (Node*)_QueryGraph_GetEntityByAlias((GraphEntity **)g->nodes, g->node_aliases, g->node_count, alias);
}

Edge* QueryGraph_GetEdgeByAlias(const QueryGraph *g, const char *alias) {
    if(alias == NULL) return NULL;
    return (Edge*)_QueryGraph_GetEntityByAlias((GraphEntity **)g->edges, g->edge_aliases, g->edge_count, alias);
}

/* Returns either a node or an edge with the given alias
 * we start by searching for a node with given alias,
 * in-case we did not find a node ansering to alias
 * we'll continue our search with edges.
 * TODO: return also entity type. */
GraphEntity* QueryGraph_GetEntityByAlias(const QueryGraph *g, const char *alias) {
    if(g == NULL) return NULL;

    GraphEntity *entity = (GraphEntity *)QueryGraph_GetNodeByAlias(g, alias);
    if(entity) {
        return entity;
    }
    
    return (GraphEntity*)QueryGraph_GetEdgeByAlias(g, alias);
}

GraphEntity** QueryGraph_GetEntityRef(const QueryGraph *g, const char *alias) {
    int i;
    char *entity_alias;

    if(g == NULL) return NULL;

    /* Search graph nodes. */
    for(i = 0; i < g->node_count; i++) {
        entity_alias = g->node_aliases[i];
        if(strcmp(entity_alias, alias) == 0) {
            return (GraphEntity**)&g->nodes[i];
        }
    }

    /* Search graph edges. */
    for(i = 0; i < g->edge_count; i++) {
        entity_alias = g->edge_aliases[i];
        if(strcmp(entity_alias, alias) == 0) {
            return (GraphEntity**)&g->edges[i];
        }
    }

    /* Entity doesn't exists in graph. */
    return NULL;
}

Node** QueryGraph_GetNodeRef(const QueryGraph *g, const Node *n) {
    assert(g && n);
    
    int i;
    int node_count = g->node_count;

    for(i = 0; i < node_count; i++) {
        if(n == g->nodes[i]) {
            return &(g->nodes[i]);
        }
    }

    return NULL;
}

Edge** QueryGraph_GetEdgeRef(const QueryGraph *g, const Edge *e) {
    assert(g && e);
    
    int i;
    int edge_count = g->edge_count;

    for(i = 0; i < edge_count; i++) {
        if(e == g->edges[i]) {
            return &(g->edges[i]);
        }
    }
    
    return NULL;
}

/* Frees entire graph. */
void QueryGraph_Free(QueryGraph* g) {
    if (!g) return;

    /* Free graph's nodes. */
    int i;
    int nodeCount = g->node_count;
    int edgeCount = g->edge_count;

    for(i = 0; i < nodeCount; i++) Node_Free(g->nodes[i]);
    for(i = 0; i < edgeCount; i++) {
        Edge *e = g->edges[i];
        Edge_Free(e);
    }

    free(g->nodes);
    free(g->edges);
    free(g->node_aliases);
    free(g->edge_aliases);
    free(g);
}
