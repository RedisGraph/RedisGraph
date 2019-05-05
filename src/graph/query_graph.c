/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "query_graph.h"
#include "../util/arr.h"
#include "../parser/ast.h"
#include "../schema/schema.h"
#include "../../deps/rax/rax.h"
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

void _QueryGraph_AddEdge(QueryGraph *g, Edge *e) {
    _QueryGraph_AddEntity((GraphEntity*)e,
                     e->alias,
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

bool _QueryGraph_ContainsEntity(GraphEntity *entity, GraphEntity **entities, int entity_count) {
    int i;
    for(i = 0; i < entity_count; i++) {
        GraphEntity *e = entities[i];
        if(e == entity) {
            return true;
        }
    }
    return false;
}

void QueryGraph_AddNode(QueryGraph *g, Node *n) {
    _QueryGraph_AddEntity((GraphEntity*)n,
    n->alias,
    (GraphEntity ***)&g->nodes,
    &g->node_aliases,
    &g->node_count,
    &g->node_cap);
}

// Extend node with label and attributes from graph entity.
void _MergeNodeWithGraphEntity(Node *n, const AST_GraphEntity *ge) {
    if(n->label == NULL && ge->label != NULL) n->label = strdup(ge->label);
}

void _BuildQueryGraphAddNode(const GraphContext *gc,
                             AST_GraphEntity *entity,
                             QueryGraph *qg) {
    const Graph *g = gc->g;
    /* Check for duplications. */
    Node *n = QueryGraph_GetNodeByAlias(qg, entity->alias);
    if(n == NULL) {
        /* Create a new node, set its properties, and add it to the graph. */
        n = Node_New(entity->label, entity->alias);
        QueryGraph_AddNode(qg, n);
    } else {
        /* Merge nodes. */
        _MergeNodeWithGraphEntity(n, entity);
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
                        AST_GraphEntity *entity,
                        AST_GraphEntity *l_entity,
                        AST_GraphEntity *r_entity,
                        QueryGraph *qg) {

    /* Check for duplications. */
    if(QueryGraph_GetEdgeByAlias(qg, entity->alias) != NULL) return;

    const Graph *g = gc->g;
    AST_LinkEntity* edge = (AST_LinkEntity*)entity;
    AST_NodeEntity *src_node;
    AST_NodeEntity *dest_node;

    // Determine relation between edge and its nodes.
    if(edge->direction == N_LEFT_TO_RIGHT) {
        src_node = l_entity;
        dest_node = r_entity;
    } else {
        src_node = r_entity;
        dest_node = l_entity;
    }

    Node *src = QueryGraph_GetNodeByAlias(qg, src_node->alias);
    Node *dest = QueryGraph_GetNodeByAlias(qg, dest_node->alias);
    Edge *e = Edge_New(src, dest, edge->ge.label, edge->ge.alias);
    
    // Incase of a variable length edge, set edge min/max hops.
    if(edge->length) {
        e->minHops = edge->length->minHops;
        e->maxHops = edge->length->maxHops;
    }

    // Set edge relation ID.
    if(edge->ge.label == NULL) {
        Edge_SetRelationID(e, GRAPH_NO_RELATION);
    } else {
        Schema *s = GraphContext_GetSchema(gc, edge->ge.label, SCHEMA_EDGE);
        if(s) {
            Edge_SetRelationID(e, s->id);
        } else {
            // Query refers to a none existing label.
            Edge_SetRelationID(e, GRAPH_UNKNOWN_RELATION);
        }
    }

    QueryGraph_ConnectNodes(qg, src, dest, e);
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
    /* Introduce nodes first. */
    for(int i = 0; i < Vector_Size(entities); i++) {
        AST_GraphEntity *entity;
        Vector_Get(entities, i, &entity);
        if(entity->t != N_ENTITY) continue;
        _BuildQueryGraphAddNode(gc, entity, qg);
    }

    /* Introduce edges. */
    for(int i = 0; i < Vector_Size(entities); i++) {
        AST_GraphEntity *entity;
        Vector_Get(entities, i, &entity);
        if(entity->t != N_LINK) continue;
        AST_GraphEntity *l_entity;
        AST_GraphEntity *r_entity;
        Vector_Get(entities, i-1, &l_entity);
        Vector_Get(entities, i+1, &r_entity);

        _BuildQueryGraphAddEdge(gc, entity, l_entity, r_entity, qg);
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

bool QueryGraph_ContainsNode(const QueryGraph *graph, const Node *node) {
    if(!graph->node_count) return false;
    return _QueryGraph_ContainsEntity((GraphEntity *)node,
                                 (GraphEntity **)graph->nodes,
                                 graph->node_count);
}

bool QueryGraph_ContainsEdge(const QueryGraph *graph, const Edge *edge) {
    if(!graph->edge_count) return false;
    return _QueryGraph_ContainsEntity((GraphEntity *)edge,
                                 (GraphEntity **)graph->edges,
                                 graph->edge_count);
}

void QueryGraph_ConnectNodes(QueryGraph *g, Node *src, Node *dest, Edge *e) {
    assert(QueryGraph_ContainsNode(g, src) && QueryGraph_ContainsNode(g, dest) && !QueryGraph_ContainsEdge(g, e));
    Node_ConnectNode(src, dest, e);
    _QueryGraph_AddEdge(g, e);
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

QueryGraph* QueryGraph_Clone(const QueryGraph *g) {
    QueryGraph *clone = QueryGraph_New(g->node_count, g->edge_count);
    
    // Clone nodes.
    for(int i = 0; i < g->node_count; i++) {
        // Clones node without its edges.
        Node *n = Node_Clone(g->nodes[i]);
        QueryGraph_AddNode(clone, n);
    }

    // Clone edges.
    for(int i = 0; i < g->edge_count; i++) {
        Edge *e = g->edges[i];
        Node *src = QueryGraph_GetNodeByAlias(clone, e->src->alias);
        Node *dest = QueryGraph_GetNodeByAlias(clone, e->dest->alias);
        Edge *clone_edge = Edge_New(src, dest, e->relationship, e->alias);
        clone_edge->mat = e->mat;
        clone_edge->relationID = e->relationID;
        QueryGraph_ConnectNodes(clone, src, dest, clone_edge);
    }

    return clone;
}

Node* QueryGraph_RemoveNode(QueryGraph *g, Node *n) {
    assert(g && n && n->alias);

    /* Remove node from query graph.
     * Remove all edges associated with node. */
    uint incoming_edge_count = array_len(n->incoming_edges);
    uint outgoing_edge_count = array_len(n->outgoing_edges);

    for(uint i = 0; i < incoming_edge_count; i++) {
        Edge *e = n->incoming_edges[i];
        QueryGraph_RemoveEdge(g, e);
    }
    for(uint i = 0; i < outgoing_edge_count; i++) {
        Edge *e = n->outgoing_edges[i];
        QueryGraph_RemoveEdge(g, e);
    }

    /* Remove node from graph nodes and alias arrays
     * the two are in sync. */
    for(int i = 0; i < g->node_count; i++) {
        if(strcmp(n->alias, g->node_aliases[i]) == 0) {
            /* Remove by migrating the last element
             * to the removed position. */
            g->nodes[i] = g->nodes[g->node_count-1];
            g->node_aliases[i] = g->node_aliases[g->node_count-1];
            break;
        }
    }

    // Update node count.
    g->node_count--;

    return n;
}

Edge* QueryGraph_RemoveEdge(QueryGraph *g, Edge *e) {
    assert(g && e && e->alias);

    // Disconnect nodes connected by edge.
    Node_RemoveOutgoingEdge(e->src, e);
    Node_RemoveIncomingEdge(e->dest, e);

    /* Remove edge from query graph.
     * Both edges and edge_aliases arrays are in sync. */
    for(int i = 0; i < g->edge_count; i++) {
        if(strcmp(e->alias, g->edge_aliases[i]) == 0) {
            /* Remove by migrating the last element
             * to the removed position. */
            g->edges[i] = g->edges[g->edge_count-1];
            g->edge_aliases[i] = g->edge_aliases[g->edge_count-1];
            break;
        }
    }

    // Update edge count.
    g->edge_count--;
    return e;
}

QueryGraph** QueryGraph_ConnectedComponents(const QueryGraph *qg) {
    Node *n;                                // Current node.
    Node **q = array_new(Node*, 1);         // Node frontier.
    void *seen;                             // Has node been visited?
    QueryGraph *g = QueryGraph_Clone(qg);   // Clone query graph.
    rax *visited;                           // Dictionary of visited nodes.
    QueryGraph **connected_components;      // List of connected components.

    // At least one connected component (the original graph).
    connected_components = array_new(QueryGraph*, 1);

    // As long as there are nodes to process.
    while(true) {
        visited = raxNew();

        // Get a random node and add it to the frontier.
        Node *s = g->nodes[0];
        q = array_append(q, s);

        // As long as there are nodes in the frontier.
        while(array_len(q)) {
            n = array_pop(q);

            // Mark n as visited.
            if(!raxInsert(visited, (unsigned char*)n->alias, strlen(n->alias), NULL, NULL)) {
                // We've already processed n.
                continue;
            }

            // Expand node N by visiting all of its neighbors
            for(int i = 0; i < array_len(n->outgoing_edges); i++) {
                Edge *e = n->outgoing_edges[i];
                char *dest = e->dest->alias;
                seen = raxFind(visited, (unsigned char*)dest, strlen(dest));
                if(seen == raxNotFound) q = array_append(q, e->dest);
            }
            for(int i = 0; i < array_len(n->incoming_edges); i++) {
                Edge *e = n->incoming_edges[i];
                char *src = e->src->alias;
                seen = raxFind(visited, (unsigned char*)src, strlen(src));
                if(seen == raxNotFound) q = array_append(q, e->src);
            }
        }

        /* Visited comprise the connected component defined by S.
         * Remove all none reachable nodes from current connected component.
         * Remove connected component from graph. */
        QueryGraph *cc = QueryGraph_Clone(g);
        uint node_count = g->node_count;
        for(int i = 0; i < node_count; i++) {
            void *reachable;
            n = g->nodes[i];
            reachable = raxFind(visited, (unsigned char*)n->alias, strlen(n->alias));

            /* If node is reachable, which means it is part of the 
             * connected component, then remove it from the graph,
             * otherwise, node isn't reachable, not part of the 
             * connected component. */
            if(reachable != raxNotFound) {
                QueryGraph_RemoveNode(g, n);
            } else {
                n = QueryGraph_GetNodeByAlias(cc, n->alias);
                QueryGraph_RemoveNode(cc, n);
            }
        }
    
        connected_components = array_append(connected_components, cc);
        
        // Clear visited dict for next iteration.
        raxFree(visited);

        // Exit when graph is empty.
        if(!g->node_count) break;
    }

    QueryGraph_Free(g);
    return connected_components;
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
