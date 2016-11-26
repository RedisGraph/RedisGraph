#include "graph.h"

Graph* NewGraph() {
    Graph* g = (Graph*)malloc(sizeof(Graph));
    g->nodes = NewVector(Node*, 0);
    return g;
}

// Search the graph for a node with given alias 
Node* Graph_GetNodeByAlias(const Graph* g, const char* alias) {
    for(int i = 0; i < Vector_Size(g->nodes); i++) {
        Node* node;
        Vector_Get(g->nodes, i, &node);

        if(strcmp(node->alias, alias) == 0) {
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
        if(n->incomingEdges == degree && strlen(n->id) == 0) {
            Vector_Push(nodes, n);
        }
    }

    // Second iteration, place at the end of the vector
    // nodes which have an ID.
    for(int i = 0; i < Vector_Size(g->nodes); i++) {
        Vector_Get(g->nodes, i, &n);
        if(n->incomingEdges == degree && strlen(n->id) > 0) {
            Vector_Push(nodes, n);
        }
    }

    return nodes;
}

// Frees entire graph.
void FreeGraph(Graph* g) {
    // Free graph's nodes
    for(int i = 0; i < Vector_Size(g->nodes); i++) {
        Node* n;
        Vector_Get(g->nodes, i, &n);
        FreeNode(n);
    }

    Vector_Free(g->nodes);
    free(g);
}