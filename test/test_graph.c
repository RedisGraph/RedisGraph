#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "../src/graph/graph.h"

void test_graph_shortestpath() {
    Graph *graph = NewGraph();
    Node *a = NewNode("a", "a");
    Node *b = NewNode("b", "b");
    Node *c = NewNode("c", "c");
    Node *d = NewNode("d", "d");
    Node *e = NewNode("e", "e");
    Node *f = NewNode("f", "f");
    Node *g = NewNode("g", "g");

    Graph_AddNode(graph, a);
    Graph_AddNode(graph, b);
    Graph_AddNode(graph, c);
    Graph_AddNode(graph, d);
    Graph_AddNode(graph, e);
    Graph_AddNode(graph, f);
    Graph_AddNode(graph, g);

    Edge *ab = NewEdge("1", NULL, a, b, "");
    ConnectNode(a, b, ab);

    Edge *ac = NewEdge("2", NULL, a, c, "");
    ConnectNode(a, c, ac);

    Edge *bd = NewEdge("3", NULL, b, d, "");
    ConnectNode(b, d, bd);

    Edge *be = NewEdge("4", NULL, b, e, "");
    ConnectNode(b, e, be);

    Edge *cf = NewEdge("5", NULL, c, f, "");
    ConnectNode(c, f, cf);
    
    Edge *ef = NewEdge("6", NULL, e, f, "");
    ConnectNode(e, f, ef);
    
    Edge *fg = NewEdge("7", NULL, f, g, "");
    ConnectNode(f, g, fg);

    // path: a->c->f->g
    // path a->b->e->f->g

    Graph *path = Graph_ShortestPath(graph, a, g);
    assert(Vector_Size(path->nodes) == 4);

    // Validate path
    assert(Graph_ContainsNode(graph, a) == 1);
    assert(Graph_ContainsNode(graph, c) == 1);
    assert(Graph_ContainsNode(graph, f) == 1);
    assert(Graph_ContainsNode(graph, g) == 1);

    path = Graph_ShortestPath(graph, g, a);
    assert(path == NULL);
}

void test_graph_clone() {
    Graph* origin = NewGraph();

    // (A:1)-[]->()-[]->(B)
    Node *a = NewNode("A", "1");
    Node *b = NewNode("B", "2");
    Node *blank = NewNode("", "");

    Edge *e1 = NewEdge("1", NULL, a, blank, "");
    Edge *e2 = NewEdge("2", NULL, blank, b, "");

    Graph_AddNode(origin, a);
    Graph_AddNode(origin, blank);
    Graph_AddNode(origin, b);

    ConnectNode(a, blank, e1);
    ConnectNode(blank, b, e2);

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

void test_graph_compare() {
    Graph* origin = NewGraph();

    // (A:1)-[]->()-[]->(B)
    Node *a = NewNode("A", "1");
    Node *b = NewNode("B", "2");
    Node *blank = NewNode("", "");

    Graph_AddNode(origin, a);
    Graph_AddNode(origin, blank);
    Graph_AddNode(origin, b);

    ConnectNode(a, blank, NewEdge("1", NULL, a, blank, ""));
    ConnectNode(blank, b, NewEdge("2", NULL, blank, b, ""));

    Graph* clone = Graph_Clone(origin);

    assert(Graph_Compare(origin, clone) == 0);

    Node *c = NewNode("C", "3");
    Graph_AddNode(origin, c);

    assert(Graph_Compare(origin, clone) == 1);
}

int main(int argc, char **argv) {
    test_graph_clone();
    test_graph_shortestpath();
    test_graph_compare();
	printf("PASS!\n");
    return 0;
}