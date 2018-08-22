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

void QueryGraph_AddNode(QueryGraph *g, Node *n, char *alias) {
    _QueryGraph_AddEntity((GraphEntity*)n,
    alias,
    (GraphEntity ***)&g->nodes,
    &g->node_aliases,
    &g->node_count,
    &g->node_cap);
}

/* Adds properties to given graph entity
 * based on the property-set specified in the AST entity. */
void _BuildQueryGraphAddProps(AST_GraphEntity *entity, GraphEntity* e) {
    if(entity->properties) {
        size_t prop_count = Vector_Size(entity->properties);
        char *keys[prop_count];
        SIValue values[prop_count];

        for(int prop_idx = 0; prop_idx < prop_count; prop_idx+=2) {
            SIValue *key;
            SIValue *value;
            Vector_Get(entity->properties, prop_idx, &key);
            Vector_Get(entity->properties, prop_idx+1, &value);

            values[prop_idx/2] = *value;
            keys[prop_idx/2] = key->stringval;
        }

        GraphEntity_Add_Properties(e, prop_count/2, keys, values);
    }
}

// Extend node with label and attributes from graph entity.
void _MergeNodeWithGraphEntity(Node *n, const AST_GraphEntity *ge) {
    if(n->label == NULL && ge->label != NULL)
        n->label = strdup(ge->label);

    if(ge->properties) {
        size_t propCount = Vector_Size(ge->properties);
        for(int i = 0; i < propCount; i+=2) {
            SIValue *key;
            SIValue *val;
            Vector_Get(ge->properties, i, &key);
            if(Node_Get_Property(n, key->stringval) == PROPERTY_NOTFOUND) {
                Vector_Get(ge->properties, i+1, &val);
                Node_Add_Properties(n, 1, &key->stringval, val);
            }
        }
    }
}

void _BuildQueryGraphAddNode(AST_GraphEntity *entity, QueryGraph *graph) {
    /* Check for duplications. */
    Node *n = QueryGraph_GetNodeByAlias(graph, entity->alias);
    if(n == NULL) {
        /* Create a new node, set its properties, and add it to the graph. */
        Node *n = Node_New(INVALID_ENTITY_ID, entity->label);
        _BuildQueryGraphAddProps(entity, (GraphEntity*)n);
        QueryGraph_AddNode(graph, n, entity->alias);
    } else {
        /* Merge nodes. */
        _MergeNodeWithGraphEntity(n, entity);
    }
}

void _BuildQueryGraphAddEdge(const Graph *g,
                        AST_GraphEntity *entity,
                        AST_GraphEntity *l_entity,
                        AST_GraphEntity *r_entity,
                        RedisModuleCtx *ctx,
                        const char *graph_name,
                        QueryGraph *graph) {

    /* Check for duplications. */
    if(QueryGraph_GetEdgeByAlias(graph, entity->alias) != NULL) return;

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

    Node *src = QueryGraph_GetNodeByAlias(graph, src_node->alias);
    Node *dest = QueryGraph_GetNodeByAlias(graph, dest_node->alias);
    Edge *e = Edge_New(INVALID_ENTITY_ID, src, dest, edge->ge.label);

    // Get relation matrix.
    if(edge->ge.label == NULL) {
        e->relationship_id = GRAPH_NO_RELATION;
        e->mat = Graph_GetAdjacencyMatrix(g);
    } else {
        LabelStore *s = LabelStore_Get(ctx, STORE_EDGE, graph_name, edge->ge.label);
        if(s) {
            e->relationship_id = s->id;
            e->mat = Graph_GetRelationMatrix(g, s->id);
        }
        else {
            /* Use a zeroed matrix.
             * TODO: either use a static zero matrix, or free this one. */
            GrB_Matrix_new(&e->mat, GrB_BOOL, Graph_NodeCount(g), Graph_NodeCount(g));
        }
    }

    QueryGraph_ConnectNodes(graph, src, dest, e, edge->ge.alias);
}

QueryGraph* QueryGraph_New() {
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
    QueryGraph *graph = QueryGraph_New();
    graph->node_cap = node_cap;
    graph->edge_cap = edge_cap;
    graph->nodes = (Node**)malloc(sizeof(Node*) * node_cap);
    graph->edges = (Edge**)malloc(sizeof(Edge*) * edge_cap);
    graph->node_aliases = (char**)malloc(sizeof(char*) * node_cap);
    graph->edge_aliases = (char**)malloc(sizeof(char*) * edge_cap);
    return graph;
}

void BuildQueryGraph(RedisModuleCtx *ctx, const Graph *g, const char *graph_name, QueryGraph *query_graph, Vector *entities) {    
    /* Introduce nodes first. */
    for(int i = 0; i < Vector_Size(entities); i++) {
        AST_GraphEntity *entity;
        Vector_Get(entities, i, &entity);
        if(entity->t != N_ENTITY) continue;
        _BuildQueryGraphAddNode(entity, query_graph);
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
        _BuildQueryGraphAddEdge(g, entity, l_entity, r_entity, ctx, graph_name, query_graph);
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

    assert(0);
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

ResultSetStatistics CommitGraph(RedisModuleCtx *ctx, const QueryGraph *qg, Graph *g, const char *graph_name) {
    ResultSetStatistics stats = {0};
    size_t node_count = qg->node_count;
    size_t edge_count = qg->edge_count;

    // Start by creating nodes.
    if(node_count > 0) {
        int labels[node_count];
        LabelStore *allStore = LabelStore_Get(ctx, STORE_NODE, graph_name, NULL);

        for(int i = 0; i < node_count; i++) {
            Node *n = qg->nodes[i];
            const char *label = n->label;

            // Set, create label.
            LabelStore *store = NULL;
            if(label == NULL) {
               labels[i] = GRAPH_NO_LABEL; 
            } else {
                store = LabelStore_Get(ctx, STORE_NODE, graph_name, label);
                /* This is the first time we encounter label, create its store */
                if(store == NULL) {
                    int label_id = Graph_AddLabelMatrix(g);
                    store = LabelStore_New(ctx, STORE_NODE, graph_name, label, label_id);
                    stats.labels_added++;
                }
                labels[i] = store->id;
            }

            // Update tracked schema.
            if(n->prop_count > 0) {
                char *properties[n->prop_count];
                for(int j = 0; j < n->prop_count; j++) properties[j] = n->properties[j].name;
                if(store) LabelStore_UpdateSchema(store, n->prop_count, properties);
                LabelStore_UpdateSchema(allStore, n->prop_count, properties);
            }
        }

        // Create nodes and set properties.
        DataBlockIterator *it;
        size_t graph_node_count = Graph_NodeCount(g);
        Graph_CreateNodes(g, node_count, labels, &it);

        for(int i = 0; i < node_count; i++) {
            Node *new_node = (Node*)DataBlockIterator_Next(it);
            Node *temp_node = qg->nodes[i];
            new_node->properties = temp_node->properties;
            new_node->prop_count = temp_node->prop_count;
            new_node->id = graph_node_count + i;
            temp_node->id = new_node->id;   /* Formed edges refer to temp_node. */
            temp_node->properties = NULL;   /* Prevents temp_node from freeing its property set. */
            stats.properties_set += new_node->prop_count;
        }

        stats.nodes_created = node_count;
    }

    // Create edges.
    if(edge_count > 0) {
        GrB_Index connections[edge_count * 3];

        for(int i = 0; i < edge_count; i++) {
            Edge *e = qg->edges[i];
            int con_idx = i*3;

            connections[con_idx] = e->src->id;
            connections[con_idx + 1] = e->dest->id;
            
            LabelStore *s = LabelStore_Get(ctx, STORE_EDGE, graph_name, e->relationship);
            if(s == NULL) {
                int relation_id = Graph_AddRelationMatrix(g);
                s = LabelStore_New(ctx, STORE_EDGE, graph_name, e->relationship, relation_id);
                s->id = relation_id;
            }

            connections[con_idx + 2] = s->id;
        }

        Graph_ConnectNodes(g, edge_count*3, connections);
        stats.relationships_created = edge_count;
    }
    return stats;
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

    /* Edges are freed internally by nodes. */
    free(g->nodes);
    free(g->edges);
    free(g);
}