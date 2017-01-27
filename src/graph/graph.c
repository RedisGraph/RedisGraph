#include "graph.h"
#include <uuid/uuid.h>

Graph* NewGraph() {
    Graph* g = (Graph*)malloc(sizeof(Graph));
    g->nodes = NewVector(Node*, 0);
    return g;
}

Node* _getNodeByInternalID(const Graph* g, uuid_t id) {
    for(int i = 0; i < Vector_Size(g->nodes); i++) {
        Node* node;
        Vector_Get(g->nodes, i, &node);

        if(uuid_compare(node->internalId, id) == 0) {
            return node;
        }
    }
    return NULL;
}

Graph* Graph_Clone(const Graph* graph) {
    Graph* clone = NewGraph();

    for(int i = 0; i < Vector_Size(graph->nodes); i++) {
        Node* node;
        Vector_Get(graph->nodes, i, &node);

        for(int j = 0; j < Vector_Size(node->outgoingEdges); j++) {
            Edge* e;
            Vector_Get(node->outgoingEdges, j, &e);
            Node* src = _getNodeByInternalID(clone, e->src->internalId);
            Node* dest = _getNodeByInternalID(clone, e->dest->internalId);

            if(src == NULL) {
                src = Graph_AddNode(clone, e->src->alias, e->src->id);
                uuid_copy(src->internalId, e->src->internalId);
            }
            if(dest == NULL) {
                dest = Graph_AddNode(clone, e->dest->alias, e->dest->id);
                uuid_copy(dest->internalId, e->dest->internalId);
            }

            ConnectNode(src, dest, e->relationship);
        }
    }
    return clone;
}

// Search the graph for a node with given alias 
Node* Graph_GetNodeByAlias(const Graph* g, const char* alias) {
    if(alias == NULL) {
        return NULL;
    }

    for(int i = 0; i < Vector_Size(g->nodes); i++) {
        Node* node;
        Vector_Get(g->nodes, i, &node);

        if(node->alias != NULL && strcmp(node->alias, alias) == 0) {
            return node;
        }
    }
    return NULL;
}

// Adds a new node to the graph
Node* Graph_AddNode(Graph* g, const char* alias, const char* id) {
    Node* n = Graph_GetNodeByAlias(g, alias);
    
    if(n == NULL) {
        n = NewNode(alias, id);
        Vector_Push(g->nodes, n);
    } else {
        // Update node ID in case it is missing
        if(n->id == NULL && id != NULL) {
            n->id = (char*)malloc(sizeof(char) * (strlen(id) + 1));
            strcpy(n->id, id);
        }
    }

    return n;
}

Vector* Graph_GetNDegreeNodes(Graph* g, int degree) {
    Vector* nodes = NewVector(Node*, 0);
    Node* n = NULL;

    // First iteration, place at the begining of the vector
    // nodes which don't have an ID.
    for(int i = 0; i < Vector_Size(g->nodes); i++) {
        Vector_Get(g->nodes, i, &n);
        if(n->incomingEdges == degree && n->id == NULL) {
            Vector_Push(nodes, n);
        }
    }

    // Second iteration, place at the end of the vector
    // nodes which have an ID.
    for(int i = 0; i < Vector_Size(g->nodes); i++) {
        Vector_Get(g->nodes, i, &n);
        if(n->incomingEdges == degree && n->id != NULL) {
            Vector_Push(nodes, n);
        }
    }

    return nodes;
}

// Frees entire graph.
void Graph_Free(Graph* g) {
    // Free graph's nodes
    for(int i = 0; i < Vector_Size(g->nodes); i++) {
        Node* n;
        Vector_Get(g->nodes, i, &n);
        FreeNode(n);
    }

    Vector_Free(g->nodes);
    free(g);
}