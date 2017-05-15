#include "graph.h"

Graph* NewGraph() {
    Graph* g = (Graph*)malloc(sizeof(Graph));
    g->nodes = NewVector(Node*, 0);
    return g;
}

Node* _getNodeByInternalID(const Graph* g, int id) {
    for(int i = 0; i < Vector_Size(g->nodes); i++) {
        Node* node;
        Vector_Get(g->nodes, i, &node);

        if(node->internalId == id) {
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

        Graph_AddNode(clone, Node_Clone(node));

        for(int j = 0; j < Vector_Size(node->outgoingEdges); j++) {
            Edge* e;
            Vector_Get(node->outgoingEdges, j, &e);
            Node* src = _getNodeByInternalID(clone, e->src->internalId);
            Node* dest = _getNodeByInternalID(clone, e->dest->internalId);

            if(src == NULL) {
                src = Node_Clone(e->src);
                Graph_AddNode(clone, src);
            }
            if(dest == NULL) {
                dest = Node_Clone(e->dest);
                Graph_AddNode(clone, dest);
            }

            ConnectNode(src, dest, NewEdge(e->id, e->alias, src, dest, e->relationship));
        }
    }
    return clone;
}

int Graph_Compare(const Graph *a, const Graph *b) {
    /* NOTE: graph structure is not comapred
     * for the timebeing this is enough 
     * TODO: comapre edges */

    const int EQUAL = 0;
    const int DIFFER = 1;

    if(a == NULL || b == NULL) {
        return DIFFER;
    }

    // Same pointer
    if(a == b) {
        return EQUAL;
    }
    
    // Graphs must contain the same number of nodes
    if(Vector_Size(a->nodes) != Vector_Size(b->nodes)) {
        return DIFFER;
    }
    
    // Make sure each node exists in both graphs
    for(int i = 0; i < Vector_Size(a->nodes); i++) {
        Node *An;
        Vector_Get(a->nodes, i, &An);
        
        Node *Bn = _getNodeByInternalID(b, An->internalId);
        if(Bn == NULL) {
            return DIFFER;
        }

        // Nodes should have the same ID.
        if(An->id == NULL && Bn->id == NULL) {
            continue;
        }

        if((An->id == NULL && Bn->id != NULL) || (An->id != NULL && Bn->id == NULL)) {
            return DIFFER;
        }

        if(strcmp(An->id, Bn->id) != 0) {
            return DIFFER;
        }
    }
    
    return EQUAL;
}

int Graph_ContainsNode(const Graph *graph, const Node *node) {
    return _getNodeByInternalID(graph, node->internalId) != NULL;
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

// TODO: Find a faster way to search for an edge.
Edge* Graph_GetEdgeByAlias(const Graph *g, const char *alias) {
    for(int i = 0; i < Vector_Size(g->nodes); i++) {
        Node *n;
        Vector_Get(g->nodes, i, &n);

        for (int j = 0; j < Vector_Size(n->outgoingEdges); j++) {
            Edge *e;
            Vector_Get(n->outgoingEdges, j, &e);

            if(e->alias != NULL) {
                if(strcmp(e->alias, alias) == 0) {
                    return e;
                }
            }
        }
    }
    return NULL;
}

// Adds a new node to the graph
int Graph_AddNode(Graph* g, Node *n) {
    if(Graph_ContainsNode(g, n)) {
        return 0;
    }
    Vector_Push(g->nodes, n);
    return 1;
}

Vector* Graph_GetNDegreeNodes(Graph* g, int degree) {
    Vector* nodes = NewVector(Node*, 0);
    Node* n = NULL;
    
    for(int i = 0; i < Vector_Size(g->nodes); i++) {
        Vector_Get(g->nodes, i, &n);
        if(Vector_Size(n->incomingEdges) == degree) {
            Vector_Push(nodes, n);
        }
    }

    return nodes;
}

Graph* Graph_ShortestPath(Graph *g, Node *src, Node *dest) {
    Graph *reversedGraph = NewGraph();
    Graph *path = NULL;
    Node *srcClone = Node_Clone(src);
    Graph_AddNode(reversedGraph, srcClone);

    // Add src node to nodes to visit
    int nodes_count = Vector_Size(g->nodes);
    int node_idx = 0;
    Vector *nodesToVisit = NewVector(Node*, nodes_count);
    Vector_Push(nodesToVisit, src);

    // As long as there are nodes to visit
    // TODO: introduce a queue data-structure.
    // while(Vector_Size(nodesToVisit) > 0) {
    while(node_idx < nodesToVisit->top) {
        Node *currentNode;
        // Vector_Pop(nodesToVisit, &currentNode);
        Vector_Get(nodesToVisit, node_idx, &currentNode);

        Node *currentNodeClone = _getNodeByInternalID(reversedGraph, currentNode->internalId);

        // Have we reached destination node?
        if(Node_Compare(currentNode, dest)) {
            // Add dest node to path.
            path = NewGraph();
            Node *pathNodeCurrent = Node_Clone(dest);
            Graph_AddNode(path, pathNodeCurrent);

            Edge *reversedEdge;
            // Walk reversed graph to construct path.
            while(Vector_Get(currentNodeClone->outgoingEdges, 0, &reversedEdge) != 0) {
                Node *pathNodeNext = Node_Clone(reversedEdge->dest);
                Graph_AddNode(path, pathNodeNext);
                ConnectNode(pathNodeNext, pathNodeCurrent, NewEdge(NULL, NULL, pathNodeNext, pathNodeCurrent,reversedEdge->relationship));
                currentNodeClone = _getNodeByInternalID(reversedGraph, pathNodeNext->internalId);
                pathNodeCurrent = pathNodeNext;
            }
            break;
        }

        // Add new nodes to visit
        for(int i = 0; i < Vector_Size(currentNode->outgoingEdges); i++) {
            Edge *e;
            Vector_Get(currentNode->outgoingEdges, i, &e);
            Node *neighbor = e->dest;

            // Make sure we don't visit a node more than once.
            if(!Graph_ContainsNode(reversedGraph, neighbor)) {
                Vector_Push(nodesToVisit, neighbor);

                // Add neighbor to reversedGraph
                Node *neighborClone = Node_Clone(neighbor);
                Graph_AddNode(reversedGraph, neighborClone);
                ConnectNode(neighborClone, currentNodeClone, NewEdge(NULL, NULL, neighborClone, currentNodeClone, e->relationship));
            }
        }
        node_idx++;
    }

    Vector_Free(nodesToVisit);
    Graph_Free(reversedGraph);
    return path;
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