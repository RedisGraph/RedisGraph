#include "./simple_timer.h"
#include "../../src/graph/graph.h"
#include "../../src/graph/GraphBLAS.h"
#include "../../src/graph/node_iterator.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assert.h"

// Console text colors
#define KGRN  "\x1B[32m"
#define KRED  "\x1B[31m"
#define KNRM  "\x1B[0m"

// OK(method) is a macro that calls a GraphBLAS method and checks the status;
// if a failure occurs, it prints the detailed error message.
#define OK(method)                                          \
{                                                           \
    info = method ;                                         \
    if (! (info == GrB_SUCCESS || info == GrB_NO_VALUE))    \
    {                                                       \
        printf ("file %s line %d\n", __FILE__, __LINE__) ;  \
        printf ("%s\n", GrB_error ( )) ;                    \
        assert (false);                                     \
    }                                                       \
}

void _test_node_creation(Graph *g, size_t node_count) {
    GrB_Info info;
    GrB_Index ncols, nrows, nvals;
    NodeIterator *it;

    // Create nodes.
    it = Graph_CreateNodes(g, node_count, NULL);

    // Validate nodes creation.
    OK(GrB_Matrix_nrows(&nrows, g->adjacency_matrix));
    OK(GrB_Matrix_ncols(&ncols, g->adjacency_matrix));
    OK(GrB_Matrix_nvals(&nvals, g->adjacency_matrix));

    assert(nvals == 0);                     // No connection were formed.
    assert(ncols == g->node_cap);           // Graph's adjacency matrix dimensions.
    assert(nrows == g->node_cap);
    assert(g->node_count == node_count);
    assert(g->node_count <= g->node_cap);

    // Make sure we've received an iterator over created nodes.
    Node *n;
    int new_node_count = 0;
    while((n = NodeIterator_Next(it)) != NULL) { new_node_count++; }
    assert(new_node_count==node_count);
    NodeIterator_Free(it);
}

void _test_edge_creation(Graph *g, size_t node_count) {
    // Form connections.
    GrB_Info info;
    size_t edge_count = (node_count-1);
    int connections[edge_count*3];

    // Introduce relations types.
    for(int i = 0; i < 3; i++) {
        Graph_AddRelationMatrix(g);
    }

    // Describe connections;
    // Node I is connected to Node I+1,
    // Connection type is relationships[I%4].
    int node_id = 0;
    for(int i = 0; i < edge_count*3; i+=3) {
        connections[i] = node_id;       // Source node id.
        connections[i+1] = node_id+1;   // Destination node id.
        connections[i+2] = (i%4)-1;     // Relation, (-1 for GRAPH_NO_RELATION).
        node_id++;
    }

    Graph_ConnectNodes(g, edge_count*3, connections);

    // Validate edges creation,
    // expecting number of none zero entries to be edge_count.
    GrB_Index nvals;
    OK(GrB_Matrix_nvals(&nvals, g->adjacency_matrix));
    assert(nvals == edge_count);

    // Inspect graph matrices;
    // Graph's adjacency matrix should include all connections,
    // relation matrices should include edges of a certain relation.
    for(int i = 0; i < edge_count*3; i+=3) {
        int src_id = connections[i];
        int dest_id = connections[i+1];
        int r = connections[i+2];
        bool v = false;
        
        // src_id connected to dest_id.
        OK(GrB_Matrix_extractElement_BOOL(&v, g->adjacency_matrix, src_id, dest_id));
        assert(v);

        if(r != GRAPH_NO_RELATION) {
            // Test relation matrix.
            v = false;
            GrB_Matrix mat = Graph_GetRelationMatrix(g, r);
            OK(GrB_Matrix_extractElement_BOOL(&v, mat, src_id, dest_id));
            assert(v);
        }
    }
}

void _test_graph_resize(Graph *g) {
    GrB_Info info;
    GrB_Index ncols, nrows;
    size_t prev_node_count = g->node_count;
    size_t node_count = g->node_cap * 2;

    Graph_CreateNodes(g, node_count, NULL);

    // Validate nodes creation.
    assert(g->node_count == prev_node_count + node_count);
    // Graph's adjacency matrix dimensions.
    OK(GrB_Matrix_nrows(&nrows, g->adjacency_matrix));
    OK(GrB_Matrix_ncols(&ncols, g->adjacency_matrix));
    assert(ncols == g->node_cap);
    assert(nrows == g->node_cap);
    assert(g->node_count <= g->node_cap);

    // Verify number of created nodes.
    Node *n;
    int new_node_count = 0;
    NodeIterator *it = Graph_ScanNodes(g);
    while((n = NodeIterator_Next(it)) != NULL) { new_node_count++; }
    assert(new_node_count == prev_node_count + node_count);
    NodeIterator_Free(it);
}

// Validate the creation of a graph,
// Make sure graph's defaults are applied.
void test_new_graph() {
    GrB_Info info;
    GrB_Index ncols, nrows, nvals;
    Graph *g = Graph_New(GRAPH_DEFAULT_NODE_CAP);

    OK(GrB_Matrix_ncols(&ncols, g->adjacency_matrix));
    OK(GrB_Matrix_nrows(&nrows, g->adjacency_matrix));
    OK(GrB_Matrix_nvals(&nvals, g->adjacency_matrix));

    assert(g->nodes_blocks != NULL);
    assert(g->relations != NULL);
    assert(g->labels != NULL);
    assert(g->adjacency_matrix != NULL);
    assert(g->node_count == 0);
    assert(g->node_cap == GRAPH_DEFAULT_NODE_CAP);
    assert(nrows == GRAPH_DEFAULT_NODE_CAP);
    assert(ncols == GRAPH_DEFAULT_NODE_CAP);
    assert(nvals == 0);

    Graph_Free(g);
}

// Tests node and edge creation.
void test_graph_construction() {
    size_t node_count = GRAPH_DEFAULT_NODE_CAP/2;
    Graph *g = Graph_New(node_count);
    Node **nodes;
    _test_node_creation(g, node_count);
    _test_edge_creation(g, node_count);

    // Introduce additional nodes which will cause graph to resize.    
    _test_graph_resize(g);

    Graph_Free(g);
}

// Test graph creation time.
void benchmark_graph_creation() {
    double tic [2], t;
    GrB_Matrix m;
    
    // Create a graph with 1 milion entities;    
    simple_tic(tic);
    GrB_Matrix_new(&m, GrB_BOOL, 8, 8);
    t = simple_toc (tic) ;
    printf("Matrix creation, time: %.6f sec\n", t);
    
    simple_tic(tic);
    GxB_Matrix_resize(m, 1000000, 1000000);
    t = simple_toc (tic) ;
    printf("Matrix resize, time: %.6f sec\n", t);
    GrB_Matrix_free(&m);
    
    Graph *g = Graph_New(GRAPH_DEFAULT_NODE_CAP*2);

    size_t n = GRAPH_DEFAULT_NODE_CAP;
    
    simple_tic(tic);
    Graph_CreateNodes(g, n, NULL);
    t = simple_toc (tic) ;
    printf("%zu Nodes created, time: %.6f sec\n", n, t);
    assert(t < 0.000006);
    

    // Introduce labels and relations to graph.
    for(int i = 0; i < 3; i++) {
        Graph_AddRelationMatrix(g);
        Graph_AddLabelVector(g);
    }

    // Form connections
    // Average of 4 edges foreach node.
    n = g->node_count * 4;    
    int connections[n*3];
    for(int i = 0; i < n*3; i+=3) {
        connections[i] = rand() % g->node_count;
        connections[i+1] = rand() % g->node_count;
        connections[i+2] = rand() % 3;  // We've introduced 3 types of relations.
    }

    simple_tic(tic);
    Graph_ConnectNodes(g, n*3, connections);
    t = simple_toc (tic) ;
    printf("Form %zu connections, time: %.6f sec\n", n, t);
    assert(t < 0.01);

    // Add additional nodes such that graph will resize.
    n = GRAPH_DEFAULT_NODE_CAP*2;

    simple_tic(tic);
    Graph_CreateNodes(g, n, NULL);
    t = simple_toc (tic) ;
    printf("Graph resize, time: %.6f sec\n", t);

    Graph_Free(g);
}

int main(int argc, char **argv) {
    // Initialize GraphBLAS.
    GrB_Info info;
    OK(GrB_init(GrB_NONBLOCKING));
    srand(time(NULL));

    test_new_graph();
    test_graph_construction();
    benchmark_graph_creation();

    GrB_finalize();

	printf("%stest_graph - PASS!%s\n", KGRN, KNRM);
    return 0;
}
