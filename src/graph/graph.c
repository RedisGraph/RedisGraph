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
        printf("Adding a new node (%s:%s) to the graph\n", alias, id);
        n = NewNode(alias, id);
        Vector_Push(g->nodes, n);
    }

    return n;
}

Node* Graph_GetNDegreeNode(Graph* g, int degree) {
    Node* n = NULL;
    for(int i = 0; i < Vector_Size(g->nodes); i++) {
        Vector_Get(g->nodes, i, &n);
        if(n->incomingEdges == degree) {
            break;
        }
    }

    if(n == NULL) {
        printf("No node with %d input degree\n", degree);
        return NULL;
    }
    
    return n;
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