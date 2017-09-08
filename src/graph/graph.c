#include "graph.h"
#include <assert.h>

GraphEntity* _Graph_GetEntityById(GraphEntity **entity_list, int entity_count, long int id) {
    int i;

    for(i = 0; i < entity_count; i++) {
        GraphEntity *entity = entity_list[i];
        if(entity->id == id) {
            return entity;
        }
    }
    return NULL;
}

int _Graph_AddEntity(GraphEntity *entity, char *alias, GraphEntity ***entity_list, char ***alias_list, int *entityCount) {
    if(*entity_list == NULL) {
        *entity_list = (GraphEntity**)malloc(sizeof(GraphEntity*));
        *alias_list = (char**)malloc(sizeof(char*));
	} else {
		*entity_list = realloc(*entity_list, sizeof(GraphEntity*) * (*entityCount + 1));
        *alias_list = realloc(*alias_list, sizeof(char*) * (*entityCount + 1));
	}

    (*entity_list)[*entityCount] = entity;
    (*alias_list)[*entityCount] = alias;
    (*entityCount)++;
    return 1;
}

int _Graph_AddEdge(Graph *g, Edge *e, char *alias) {
    return _Graph_AddEntity((GraphEntity*)e,
                            alias,
                            (GraphEntity ***)(&g->edges),
                            &g->edge_aliases,
                            &g->edge_count);
}

GraphEntity* _Graph_GetEntityByAlias(GraphEntity **entity_list, char **alias_list, int entity_count, const char* alias) {
    for(int i = 0; i < entity_count; i++) {
        char *entity_alias = alias_list[i];
        if(strcmp(entity_alias, alias) == 0) {
            return entity_list[i];
        }
    }
    return NULL;
}

char* _Graph_GetEntityAlias(GraphEntity *entity, GraphEntity **entities, char **aliases, int entity_count) {
    int i;
    for(i = 0; i < entity_count; i++) {
        if(entities[i]->id == entity->id) {
            return aliases[i];
        }
    }
    
    return NULL;
}

int _Graph_ContainsEntity(GraphEntity *entity, GraphEntity **entities, int entity_count) {
    int i;
    for(i = 0; i < entity_count; i++) {
        GraphEntity *e = entities[i];
        if(e == entity) {
            return 1;
        }
    }
    return 0;
}

Graph* NewGraph() {
    Graph* g = (Graph*)malloc(sizeof(Graph));
    g->nodes = NULL;
    g->edges = NULL;
    g->node_aliases = NULL;
    g->edge_aliases = NULL;
    g->node_count = 0;
    g->edge_count = 0;
    return g;
}

Node* Graph_GetNodeById(const Graph *g, long int id) {
    if(id == INVALID_ENTITY_ID) return NULL;
    return (Node*)_Graph_GetEntityById((GraphEntity **)g->nodes, g->node_count, id);
}

Edge* Graph_GetEdgeById(const Graph *g, long int id) {
    if(id == INVALID_ENTITY_ID) return NULL;
    return (Edge*)_Graph_GetEntityById((GraphEntity **)g->edges, g->edge_count, id);
}

int Graph_ContainsNode(const Graph *graph, const Node *node) {
    if(!graph->node_count) return 0;
    return _Graph_ContainsEntity((GraphEntity *)node,
                                 (GraphEntity **)graph->nodes,
                                 graph->node_count);
}

int Graph_ContainsEdge(const Graph *graph, const Edge *edge) {
    if(!graph->edge_count) return 0;
    return _Graph_ContainsEntity((GraphEntity *)edge,
                                 (GraphEntity **)graph->edges,
                                 graph->edge_count);
}

int Graph_AddNode(Graph* g, Node *n, char *alias) {
    if(!Graph_ContainsNode(g, n)) {
        return _Graph_AddEntity((GraphEntity*)n,
        alias,
        (GraphEntity ***)&g->nodes,
        &g->node_aliases,
        &g->node_count);
    }
    return 0;
}

int Graph_ConnectNodes(Graph *g, Node *src, Node *dest, Edge *e, char *edge_alias) {
    assert(Graph_ContainsNode(g, src) && Graph_ContainsNode(g, dest) && !Graph_ContainsEdge(g, e));
    Node_ConnectNode(src, dest, e);
    return _Graph_AddEdge(g, e, edge_alias);
}

Node* Graph_GetNodeByAlias(const Graph* g, const char* alias) {
    if(alias == NULL) return NULL;
    return (Node*)_Graph_GetEntityByAlias((GraphEntity **)g->nodes, g->node_aliases, g->node_count, alias);
}

Edge* Graph_GetEdgeByAlias(const Graph *g, const char *alias) {
    if(alias == NULL) return NULL;
    return (Edge*)_Graph_GetEntityByAlias((GraphEntity **)g->edges, g->edge_aliases, g->edge_count, alias);
}

/* Returns either a node or an edge with the given alias
 * we start by searching for a node with given alias,
 * in-case we did not find a node ansering to alias
 * we'll continue our search with edges.
 * TODO: return also entity type. */
GraphEntity* Graph_GetEntityByAlias(const Graph *g, const char *alias) {
    GraphEntity *entity = (GraphEntity *)Graph_GetNodeByAlias(g, alias);
    if(entity) {
        return entity;
    }
    
    return (GraphEntity*)Graph_GetEdgeByAlias(g, alias);
}

Vector* Graph_GetNDegreeNodes(Graph* g, int degree) {
    Vector* nodes = NewVector(Node*, 0);
    Node* n = NULL;
    
    for(int i = 0; i < g->node_count; i++) {
        n = g->nodes[i];
        if(Vector_Size(n->incomingEdges) == degree) {
            Vector_Push(nodes, n);
        }
    }

    return nodes;
}

char* Graph_GetNodeAlias(const Graph *g, const Node *n) {
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

char* Graph_GetEdgeAlias(const Graph *g, const Edge *e) {
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

Node** Graph_GetNodeRef(const Graph *g, const Node *n) {
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

Edge** Graph_GetEdgeRef(const Graph *g, const Edge *e) {
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
void Graph_Free(Graph* g) {
    /* Free graph's nodes. */
    int i;
    int nodeCount = g->node_count;

    for(i = 0; i < nodeCount; i++) {
        Node* n = g->nodes[i];
        FreeNode(n);
    }

    /* TODO: Free edges. */

    /* Edges are freed internaly by nodes */
    free(g->nodes);
    free(g->edges);
    free(g);
}