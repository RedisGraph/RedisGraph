#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "../src/graph/graph.h"

void test_graph_clone() {
    Graph* origin = NewGraph();

    // (A:1)-[]->()-[]->(B)
    Node* a = Graph_AddNode(origin, "A", "1");
    Node* blank = Graph_AddNode(origin, "", "");
    Node* b = Graph_AddNode(origin, "B", "");
    
    ConnectNode(a, blank, "");
    ConnectNode(blank, b, "");

    Graph* clone = Graph_Clone(origin);

    // Validate
    assert(Vector_Size(origin->nodes) == Vector_Size(clone->nodes));

    Node* aClone = Graph_GetNodeByAlias(clone, "A");
    Node* blankClone = Graph_GetNodeByAlias(clone, "");
    Node* bClone = Graph_GetNodeByAlias(clone, "B");

    assert(aClone != NULL);
    assert(blankClone != NULL);
    assert(bClone != NULL);

    assert(strcmp(aClone->alias, a->alias) == 0);
    assert(strcmp(blankClone->alias, blank->alias) == 0);
    assert(strcmp(bClone->alias, b->alias) == 0);

    assert(strcmp(aClone->id, a->id) == 0);
    assert(strcmp(blankClone->id, blank->id) == 0);
    assert(strcmp(bClone->id, b->id) == 0);

    assert(Vector_Size(aClone->outgoingEdges) == Vector_Size(a->outgoingEdges));
    assert(Vector_Size(blankClone->outgoingEdges) == Vector_Size(blank->outgoingEdges));
    assert(Vector_Size(bClone->outgoingEdges) == Vector_Size(b->outgoingEdges));


    Edge* e;
    Vector_Get(aClone->outgoingEdges, 0, &e);
    assert(e != NULL);
    assert(e->src == aClone);
    assert(e->dest == blankClone);

    Vector_Get(blankClone->outgoingEdges, 0, &e);
    assert(e != NULL);
    assert(e->src == blankClone);
    assert(e->dest == bClone);
}

int main(int argc, char **argv) {
    test_graph_clone();
	printf("PASS!");
    return 0;
}