#include "../../src/graph/graph.h"
#include "../../src/graph/GraphBLAS.h"
#include "../../src/graph/node_iterator.h"
#include "../../src/util/simple_timer.h"

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
    Graph_CreateNodes(g, node_count, NULL, &it);

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
    GrB_Index connections[edge_count*3];

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

    Graph_CreateNodes(g, node_count, NULL, NULL);

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

    // Relation matrices get resize lazily,
    // Try to fetch one of the specific relation matrices and verify its dimenstions.
    assert(g->relation_count > 0);
    for(int i = 0; i < g->relation_count; i++) {
        GrB_Matrix r = Graph_GetRelationMatrix(g, i);
        OK(GrB_Matrix_nrows(&nrows, r));
        OK(GrB_Matrix_ncols(&ncols, r));
        assert(ncols == g->node_cap);
        assert(nrows == g->node_cap);
    }
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

void benchmark_node_creation_with_labels() {
    printf("benchmark_node_creation_with_labels\n");
    double tic [2];
    int samples = 64;
    int label_count = 3;
    double timings[samples];
    int outliers = 0;
    float threshold = 0.0018;
    // size_t n = GRAPH_DEFAULT_NODE_CAP;
    size_t n = 1000000;
    Graph *g = Graph_New(n);    
    
    // Introduce labels and relations to graph.
    for(int i = 0; i < label_count; i++) {
        Graph_AddRelationMatrix(g); // Typed relation.
        Graph_AddLabelMatrix(g);    // Typed node.
    }
    
    int labels[n];

    // Create N nodes with labels.
    for(int i = 0; i < samples; i++) {        
        // Associate nodes to labels.
        for(int j = 0; j < n; j++) {
            labels[j] = (rand() % label_count)-1;
        }

        simple_tic(tic);
        Graph_CreateNodes(g, n, labels, NULL);
        timings[i] = simple_toc(tic);
        printf("%zu Nodes created, time: %.6f sec\n", n, timings[i]);
        if(timings[i] > threshold) outliers++;
    }

    if(outliers > samples * 0.1) {
        printf("Node creation took too long\n");
        for(int i = 0; i < samples; i++) {
            printf("%zu Nodes created, time: %.6f sec\n", n, timings[i]);
        }
        // assert(false);
    }

    Graph_Free(g);
}

// Test graph creation time.
void benchmark_node_creation_no_labels() {
    printf("benchmark_node_creation_no_labels\n");
    double tic [2], t;
    int samples = 64;
    double timings[samples];
    int outliers = 0;
    float threshold = 0.000006;
    // size_t n = GRAPH_DEFAULT_NODE_CAP;
    size_t n = 1000000;
    Graph *g = Graph_New(n);

    for(int i = 0; i < samples; i++) {
        // Create N nodes, don't use labels.
        simple_tic(tic);
        Graph_CreateNodes(g, n, NULL, NULL);
        timings[i] = simple_toc(tic);
        printf("%zu Nodes created, time: %.6f sec\n", n, timings[i]);
        if(timings[i] > threshold) outliers++;
    }

    if(outliers > samples * 0.1) {
        printf("Node creation took too long\n");
        for(int i = 0; i < samples; i++) {
            printf("%zu Nodes created, time: %.6f sec\n", n, timings[i]);
        }
        // assert(false);
    }
    
    Graph_Free(g);
}

void benchmark_edge_creation_no_relationships() {
    printf("benchmark_edge_creation_no_relationships\n");
    double tic [2], t;
    int samples = 64;
    double timings[samples];
    int outliers = 0;
    float threshold = 0.001;
    // int edge_count = GRAPH_DEFAULT_NODE_CAP * 1.10;
    // int node_count = GRAPH_DEFAULT_NODE_CAP;
    int edge_count = 1000000 * 1.10;
    int node_count = 1000000;
    GrB_Index connections[edge_count*3];

    Graph *g = Graph_New(GRAPH_DEFAULT_NODE_CAP);
    Graph_CreateNodes(g, node_count, NULL, NULL);

    for(int i = 0; i < samples; i++) {
        // Describe connections;
        // Node I is connected to Node I+1.
        for(int i = 0; i < edge_count*3; i+=3) {
            connections[i] = rand()%node_count;     // Source node id.
            connections[i+1] = rand()%node_count;   // Destination node id.
            connections[i+2] = GRAPH_NO_RELATION;   // Relation.
        }

        simple_tic(tic);
        Graph_ConnectNodes(g, edge_count*3, connections);
        timings[i] = simple_toc(tic);
        printf("%d Formed connections, time: %.6f sec\n", edge_count, timings[i]);
        if(timings[i] > threshold) outliers++;
    }
    
    if(outliers > samples * 0.1) {
        printf("Node creation took too long\n");
        for(int i = 0; i < samples; i++) {
            printf("%d Formed connections, time: %.6f sec\n", edge_count, timings[i]);
        }
        // assert(false);
    }

    Graph_Free(g);
}

void benchmark_edge_creation_with_relationships() {
    printf("benchmark_edge_creation_with_relationships\n");
    double tic [2];
    int samples = 64;
    double timings[samples];
    int outliers = 0;
    float threshold = 0.002;
    int edge_count = 1000000 * 1.10;
    int node_count = 1000000;
    int relation_count = 3;
    GrB_Index connections[edge_count*3];
    
    Graph *g = Graph_New(GRAPH_DEFAULT_NODE_CAP);

    // Introduce relations types.
    for(int i = 0; i < relation_count; i++) {
        Graph_AddRelationMatrix(g);
    }

    Graph_CreateNodes(g, node_count, NULL, NULL);

    for(int i = 0; i < samples; i++) {
        // Describe connections;
        // Node I is connected to Node I+1,
        // Connection type is relationships[I%4].
        for(int i = 0; i < edge_count*3; i+=3) {
            connections[i] = rand()%node_count;     // Source node id.
            connections[i+1] = rand()%node_count;   // Destination node id.
            connections[i+2] = (i%(relation_count+1))-1; // Relation, (-1 for GRAPH_NO_RELATION).
        }

        simple_tic(tic);
        Graph_ConnectNodes(g, edge_count*3, connections);
        timings[i] = simple_toc(tic);
        printf("%d Formed connections, time: %.6f sec\n", edge_count, timings[i]);
        if(timings[i] > threshold) outliers++;
    }
    
    if(outliers > samples * 0.1) {
        printf("Node creation took too long\n");
        for(int i = 0; i < samples; i++) {
            printf("%d Formed connections, time: %.6f sec\n", edge_count, timings[i]);
        }
        // assert(false);
    }

    Graph_Free(g);
}

void benchmark_graph() {
    benchmark_node_creation_no_labels();
    benchmark_node_creation_with_labels();
    benchmark_edge_creation_with_relationships();
    benchmark_edge_creation_no_relationships();
    printf("%sgraph benchmark - PASS!%s\n", KGRN, KNRM);
}

int main(int argc, char **argv) {
    // Initialize GraphBLAS.
    GrB_Info info;
    OK(GrB_init(GrB_NONBLOCKING));
    srand(time(NULL));

    test_new_graph();
    test_graph_construction();
    // benchmark_graph();

    GrB_finalize();

	printf("%stest_graph - PASS!%s\n", KGRN, KNRM);
    return 0;
}
