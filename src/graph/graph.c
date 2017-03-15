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
                src = Node_Clone(e->src);
                Graph_AddNode(clone, src);
            }
            if(dest == NULL) {
                dest = Node_Clone(e->dest);
                Graph_AddNode(clone, dest);
            }

            ConnectNode(src, dest, e->relationship);
        }
    }
    return clone;
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

Vector* Graph_ShortestPath(Graph *g, Node *src, Node *dest) {
    Vector* res = NULL;
    Graph *reversedGraph = NewGraph();
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
            Vector *path = NewVector(Node*, 1);
            Vector_Push(path, dest);

            Edge *reversedEdge;
            // Walk reversed graph to construct path.
            while(Vector_Get(currentNodeClone->outgoingEdges, 0, &reversedEdge) != 0) {
                currentNodeClone = reversedEdge->dest;
                Vector_Push(path, _getNodeByInternalID(g, currentNodeClone->internalId));
            }
            res = path;
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
                ConnectNode(neighborClone, currentNodeClone, e->relationship);
            }
        }
        node_idx++;
    }

    Vector_Free(nodesToVisit);
    Graph_Free(reversedGraph);
    return res;
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