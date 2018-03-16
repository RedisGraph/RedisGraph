#include "query_graph.h"
#include <assert.h>

GraphEntity* _QueryGraph_GetEntityById(GraphEntity **entity_list, int entity_count, long int id) {
    int i;

    for(i = 0; i < entity_count; i++) {
        GraphEntity *entity = entity_list[i];
        if(entity->id == id) {
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
        if(entities[i]->id == entity->id) {
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

QueryGraph* NewQueryGraph() {
    QueryGraph* g = (QueryGraph*)malloc(sizeof(QueryGraph));
    g->node_count = 0;
    g->edge_count = 0;
    g->node_cap = DEFAULT_GRAPH_CAP;
    g->edge_cap = DEFAULT_GRAPH_CAP;
    g->nodes = (Node**)malloc(sizeof(Node*) * g->node_cap);
    g->edges = (Edge**)malloc(sizeof(Edge*) * g->edge_cap);
    g->node_aliases = (char**)malloc(sizeof(char*) * g->node_cap);
    g->edge_aliases = (char**)malloc(sizeof(char*) * g->edge_cap);
    return g;
}

QueryGraph* NewQueryGraph_WithCapacity(size_t node_cap, size_t edge_cap) {
    QueryGraph *graph = NewQueryGraph();
    graph->node_cap = node_cap;
    graph->edge_cap = edge_cap;
    graph->nodes = (Node**)malloc(sizeof(Node*) * node_cap);
    graph->edges = (Edge**)malloc(sizeof(Edge*) * edge_cap);
    graph->node_aliases = (char**)malloc(sizeof(char*) * node_cap);
    graph->edge_aliases = (char**)malloc(sizeof(char*) * edge_cap);
    return graph;
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

int QueryGraph_AddNode(QueryGraph *g, Node *n, char *alias) {
    if(!QueryGraph_ContainsNode(g, n)) {
        _QueryGraph_AddEntity((GraphEntity*)n,
        alias,
        (GraphEntity ***)&g->nodes,
        &g->node_aliases,
        &g->node_count,
        &g->node_cap);
        return 1;
    }
    return 0;
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

Vector* QueryGraph_GetNDegreeNodes(const QueryGraph* g, int degree) {
    Vector* nodes = NewVector(Node*, 0);
    Node* n = NULL;
    
    for(int i = 0; i < g->node_count; i++) {
        n = g->nodes[i];
        if(Vector_Size(n->incoming_edges) == degree) {
            Vector_Push(nodes, n);
        }
    }

    return nodes;
}

char* QueryGraph_GetNodeAlias(const QueryGraph *g, const Node *n) {
    assert(g && n);
    
    int i;
    int node_count = g->node_count;

    for(i = 0; i < node_count; i++) {
        if(n == g->nodes[i]) {
            return g->node_aliases[i];
        }
    }

    return NULL;
}

char* QueryGraph_GetEdgeAlias(const QueryGraph *g, const Edge *e) {
    assert(g && e);
    
    int i;
    int edge_count = g->edge_count;

    for(i = 0; i < edge_count; i++) {
        if(e == g->edges[i]) {
            return g->edge_aliases[i];
        }
    }
    
    return NULL;
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

    for(i = 0; i < nodeCount; i++) {
        Node* n = g->nodes[i];
        Node_Free(n);
    }

    /* TODO: Free edges. */

    /* Edges are freed internaly by nodes */
    free(g->nodes);
    free(g->edges);
    free(g);
}