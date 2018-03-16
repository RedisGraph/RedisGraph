#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "../../src/graph/graph.h"

void test_graph_creation() {
    Graph *graph = NewGraph();

    assert(graph->nodes != NULL);
    assert(graph->edges != NULL);
    assert(graph->node_aliases != NULL);
    assert(graph->edge_aliases != NULL);
    assert(graph->node_count == 0);
    assert(graph->edge_count == 0);

    Graph_Free(graph);
}

void test_graph_construction() {
    Node fake_node;
    Edge fake_edge;
    
    Graph *graph = NewGraph();
    Node *person_node = NewNode(1l, "person");
	Node *city_node = NewNode(2l, "city");
	Edge *edge = NewEdge(3l, person_node, city_node, "lives");
    
    assert(Graph_AddNode(graph, person_node, "Joe"));
    assert(Graph_AddNode(graph, city_node, "NYC"));
    Graph_ConnectNodes(graph, person_node, city_node, edge, "relation");

    assert(Graph_ContainsNode(graph, person_node));
    assert(Graph_ContainsNode(graph, city_node));
    assert(!Graph_ContainsNode(graph, &fake_node));
    
    assert(Graph_ContainsEdge(graph, edge));    
    assert(!Graph_ContainsEdge(graph, &fake_edge));
    
    assert(Graph_GetNodeById(graph, person_node->id) == person_node);
    assert(Graph_GetNodeById(graph, city_node->id) == city_node);
    assert(Graph_GetNodeById(graph, 24l) == NULL);

    assert(Graph_GetEdgeById(graph, edge->id) == edge);
    assert(Graph_GetEdgeById(graph, 24l) == NULL);

    assert(Graph_GetNodeByAlias(graph, "Joe") == person_node);
    assert(Graph_GetNodeByAlias(graph, "NYC") == city_node);
    assert(Graph_GetNodeByAlias(graph, "fake_alias") == NULL);

    assert(Graph_GetEdgeByAlias(graph, "relation") == edge);
    assert(Graph_GetEdgeByAlias(graph, "fake_alias") == NULL);

    assert(strcmp(Graph_GetNodeAlias(graph, person_node), "Joe") == 0);
    assert(strcmp(Graph_GetNodeAlias(graph, city_node), "NYC") == 0);
    assert(Graph_GetNodeAlias(graph, &fake_node) == NULL);

    assert(strcmp(Graph_GetEdgeAlias(graph, edge), "relation") == 0);
    assert(Graph_GetEdgeAlias(graph, &fake_edge) == NULL); 

    assert(Graph_GetNodeRef(graph, person_node) == &graph->nodes[0]);
    assert(Graph_GetNodeRef(graph, city_node) == &graph->nodes[1]);
    assert(Graph_GetNodeRef(graph, &fake_node) == NULL);
    
    assert(Graph_GetEdgeRef(graph, edge) == &graph->edges[0]);
    assert(Graph_GetEdgeRef(graph, &fake_edge) == NULL);

    Vector *one_input_degree_nodes = Graph_GetNDegreeNodes(graph, 0);
    assert(Vector_Size(one_input_degree_nodes) == 1);
    Node *one_input_degree_node;
    Vector_Get(one_input_degree_nodes, 0, &one_input_degree_node);
    assert(one_input_degree_node == person_node);
    assert(Vector_Size(Graph_GetNDegreeNodes(graph, 2)) == 0);
    
    Graph_Free(graph);
}

void test_graph_id_less_entities() {
    Node fake_node;
    Edge fake_edge;
    
    Graph *graph = NewGraph();
    Node *person_node = NewNode(INVALID_ENTITY_ID, "person");
	Node *city_node = NewNode(INVALID_ENTITY_ID, "city");
	Edge *edge = NewEdge(INVALID_ENTITY_ID, person_node, city_node, "lives");
    
    assert(Graph_AddNode(graph, person_node, "Joe"));
    assert(Graph_AddNode(graph, city_node, "NYC"));
    Graph_ConnectNodes(graph, person_node, city_node, edge, "relation");

    assert(Graph_ContainsNode(graph, person_node));
    assert(Graph_ContainsNode(graph, city_node));
    assert(!Graph_ContainsNode(graph, &fake_node));
    
    assert(Graph_ContainsEdge(graph, edge));    
    assert(!Graph_ContainsEdge(graph, &fake_edge));
    
    assert(Graph_GetNodeById(graph, person_node->id) == NULL);
    assert(Graph_GetNodeById(graph, city_node->id) == NULL);
    assert(Graph_GetNodeById(graph, 24l) == NULL);

    assert(Graph_GetEdgeById(graph, edge->id) == NULL);
    assert(Graph_GetEdgeById(graph, 24l) == NULL);

    assert(Graph_GetNodeByAlias(graph, "Joe") == person_node);
    assert(Graph_GetNodeByAlias(graph, "NYC") == city_node);
    assert(Graph_GetNodeByAlias(graph, "fake_alias") == NULL);

    assert(Graph_GetEdgeByAlias(graph, "relation") == edge);
    assert(Graph_GetEdgeByAlias(graph, "fake_alias") == NULL);

    assert(strcmp(Graph_GetNodeAlias(graph, person_node), "Joe") == 0);
    assert(strcmp(Graph_GetNodeAlias(graph, city_node), "NYC") == 0);
    assert(Graph_GetNodeAlias(graph, &fake_node) == NULL);

    assert(strcmp(Graph_GetEdgeAlias(graph, edge), "relation") == 0);
    assert(Graph_GetEdgeAlias(graph, &fake_edge) == NULL); 

    assert(Graph_GetNodeRef(graph, person_node) == &graph->nodes[0]);
    assert(Graph_GetNodeRef(graph, city_node) == &graph->nodes[1]);
    assert(Graph_GetNodeRef(graph, &fake_node) == NULL);
    
    assert(Graph_GetEdgeRef(graph, edge) == &graph->edges[0]);
    assert(Graph_GetEdgeRef(graph, &fake_edge) == NULL);

    Vector *one_input_degree_nodes = Graph_GetNDegreeNodes(graph, 0);
    assert(Vector_Size(one_input_degree_nodes) == 1);
    Node *one_input_degree_node;
    Vector_Get(one_input_degree_nodes, 0, &one_input_degree_node);
    assert(one_input_degree_node == person_node);
    assert(Vector_Size(Graph_GetNDegreeNodes(graph, 2)) == 0);
    
    Graph_Free(graph);
}

int main(int argc, char **argv) {
    test_graph_creation();
    test_graph_construction();

	printf("test_graph - PASS!\n");
    return 0;
}