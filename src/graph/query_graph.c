/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "query_graph.h"
#include "../parser/ast.h"
#include "../stores/store.h"
#include <assert.h>

// Perform DFT on a node to map the connected component it is a part of.
int _BuildConnectedComponent(TrieMap *visited, QueryGraph *component, Node *n) {
  // Add the node alias to the triemap if it is not already present.
  int is_new = TrieMap_Add(visited, n->alias, strlen(n->alias), NULL, TrieMap_DONT_CARE_REPLACE);
  // Nothing needs to be done on previously-visited nodes
  if (!is_new) return 0;

  // Add node to graph
  QueryGraph_AddNode(component, n, n->alias);
  // Recursively visit every node connected to the current in either direction
  Edge *e;
  for (int i = 0; i < Vector_Size(n->outgoing_edges); i ++) {
    Vector_Get(n->outgoing_edges, i, &e);
    if (_BuildConnectedComponent(visited, component, e->dest)) {
      // If an outgoing edge was newly visited, connect src and dest
      QueryGraph_ConnectNodes(component, n, e->dest, e, e->alias);
    }
  }
  for (int i = 0; i < Vector_Size(n->incoming_edges); i ++) {
    Vector_Get(n->incoming_edges, i, &e);
    if (_BuildConnectedComponent(visited, component, e->src)) {
      // If an incoming edge was newly visited, connect dest and src
      QueryGraph_ConnectNodes(component, e->dest, n, e, e->alias);
    }
  }

  return 1;
}

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

// Extend node with label and attributes from graph entity.
void _MergeNodeWithGraphEntity(Node *n, const AST_GraphEntity *ge) {
    if(n->label == NULL && ge->label != NULL) n->label = strdup(ge->label);
}

Node* _BuildQueryGraphAddNode(const GraphContext *gc,
                             AST_GraphEntity *entity,
                             QueryGraph *qg) {
    const Graph *g = gc->g;
    /* Check for duplications. */
    Node *n = QueryGraph_GetNodeByAlias(qg, entity->alias);
    if(n == NULL) {
        /* Create a new node, set its properties, and add it to the graph. */
        n = Node_New(entity->label, entity->alias);
        QueryGraph_AddNode(qg, n, entity->alias);
    } else {
        /* Merge nodes. */
        _MergeNodeWithGraphEntity(n, entity);
    }

    /* Set node matrix.
     * TODO: revisit when supporting multiple labels. */
    if(n->label && !n->mat) {
        LabelStore *s = GraphContext_GetStore(gc, entity->label, STORE_NODE);
        if(s) {
            n->mat = Graph_GetLabel(g, s->id);
        } else {
            /* Use a zeroed matrix.
             * TODO: either use a static zero matrix, or free this one. */
            GrB_Matrix_new(&n->mat, GrB_BOOL, Graph_NodeCount(g), Graph_NodeCount(g));
        }
    }

    return n;
}

void _BuildQueryGraphAddEdge(const GraphContext *gc,
                        AST_LinkEntity *edge,
                        Node *src,
                        Node *dest,
                        QueryGraph *qg) {

    const Graph *g = gc->g;

    // Swap source and destination nodes if traversing from right to left
    if(edge->direction == N_RIGHT_TO_LEFT) {
        Node *tmp = src;
        src = dest;
        dest = tmp;
    }

    Edge *e = Edge_New(src, dest, edge->ge.label, edge->ge.alias);

    // Get relation matrix.
    if(edge->ge.label == NULL) {
        Edge_SetRelationID(e, GRAPH_NO_RELATION);
        e->mat = Graph_GetAdjacencyMatrix(g);
    } else {
        LabelStore *s = GraphContext_GetStore(gc, edge->ge.label, STORE_EDGE);
        if(s) {
            Edge_SetRelationID(e, s->id);
            e->mat = Graph_GetRelationMatrix(g, s->id);
        }
        else {
            /* Use a zeroed matrix.
             * TODO: either use a static zero matrix, or free this one. */
            GrB_Matrix_new(&e->mat, GrB_BOOL, Graph_NodeCount(g), Graph_NodeCount(g));
        }
    }

    QueryGraph_ConnectNodes(qg, src, dest, e, edge->ge.alias);
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

void BuildQueryGraph(const GraphContext *gc, QueryGraph *qg, Vector *entities) {
    /* Store node pointers to reduce the number of lookups in edge construction.
     * Only indices corresponding to real nodes will be valid references, and pointers
     * may appear repeatedly. */
    Node *nodes[Vector_Size(entities)];

    /* Introduce nodes first. */
    for(int i = 0; i < Vector_Size(entities); i++) {
        AST_GraphEntity *entity;
        Vector_Get(entities, i, &entity);
        if(entity->t != N_ENTITY) continue;
        nodes[i] = _BuildQueryGraphAddNode(gc, entity, qg);
    }

    /* Introduce edges. */
    for(int i = 0; i < Vector_Size(entities); i++) {
        AST_GraphEntity *entity;
        Vector_Get(entities, i, &entity);
        if(entity->t != N_LINK) continue;
        _BuildQueryGraphAddEdge(gc, (AST_LinkEntity*)entity, nodes[i-1], nodes[i+1], qg);
    }
}

void QueryGraph_AddNode(QueryGraph *g, Node *n, char *alias) {
    _QueryGraph_AddEntity((GraphEntity*)n,
    alias,
    (GraphEntity ***)&g->nodes,
    &g->node_aliases,
    &g->node_count,
    &g->node_cap);
}

QueryGraph** QueryGraph_ConnectedComponents(const QueryGraph *qg, int *component_count) {
    // Construct a triemap to track whether each node has been previously visited
    TrieMap *visited = NewTrieMap();
    // Store a starting point for each distinct component
    QueryGraph **subgraphs = malloc(qg->node_count * sizeof(QueryGraph*));
    int count = 0;
    subgraphs[count] = QueryGraph_New(2, 2);
    QueryGraph *component_graph = subgraphs[count];
    for (int i = 0; i < qg->node_count; i ++) {
        Node *current_node = qg->nodes[i];
        if (_BuildConnectedComponent(visited, component_graph, current_node)) {
            // We've fully traversed a component; start a new subgraph.
            count ++;
            subgraphs[count] = QueryGraph_New(2, 2);
            component_graph = subgraphs[count];
        }
    }
    TrieMap_Free(visited, TrieMap_NOP_CB);
    // TODO free last graph if empty
    subgraphs = realloc(subgraphs, count * sizeof(QueryGraph*));
    *component_count = count;
    return subgraphs;
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
    /* Free graph's nodes. */
    int i;
    int nodeCount = g->node_count;
    int edgeCount = g->edge_count;

    for(i = 0; i < nodeCount; i++) Node_Free(g->nodes[i]);
    for(i = 0; i < edgeCount; i++) Edge_Free(g->edges[i]);

    free(g->nodes);
    free(g->edges);
    free(g->node_aliases);
    free(g->edge_aliases);
    free(g);
}
