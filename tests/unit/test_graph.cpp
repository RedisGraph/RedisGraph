/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "../googletest/include/gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../../src/graph/graph.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"
#include "../../src/graph/node_iterator.h"
#include "../../src/util/simple_timer.h"
#include "../../src/arithmetic/tuples_iter.h"

#ifdef __cplusplus
}
#endif

// Console text colors for benchmark printing
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

class GraphTest: public ::testing::Test {
  protected:
    static void SetUpTestCase() {
      // Initialize GraphBLAS.
      GrB_Info info;
      OK(GrB_init(GrB_NONBLOCKING));
      srand(time(NULL));
    }

    static void TearDownTestCase() {
      GrB_finalize();
    }
};

/* TODO _print_matrix is unused */
void _print_matrix(GrB_Matrix M) {
    GrB_Index nrows;
    GrB_Index ncols;
    GrB_Matrix_nrows(&nrows, M);
    GrB_Matrix_ncols(&ncols, M);

    for(unsigned int i = 0; i < nrows; i++) {
        for(unsigned int j = 0; j < ncols; j++) {
            bool x = false;
            GrB_Matrix_extractElement_BOOL(&x, M, i, j);
            printf("%d ", x);
        }
        printf("\n");
    }
    printf("\n\n\n");
}

/* TODO the various _ methods should perhaps be moved to a
 * gcc-compiled utility file, though they do work correctly here. */
Graph* _random_graph(int nodes, int relations) {
    Graph *g = Graph_New(nodes);
    Graph_CreateNodes(g, nodes, NULL, NULL);

    for(int i = 0; i < relations; i++) {
        Graph_AddRelationMatrix(g);
    }

    int connectionCount = 0;
    GrB_Index *connections = (GrB_Index*)malloc(sizeof(GrB_Index) * 3 * nodes * nodes);

    double mid_point = RAND_MAX/2;
    for(int i = 0; i < nodes; i++) {
        for(int j = 0; j < nodes; j++) {
            if(rand() > mid_point) {
                if(i == j) continue;
                int relation = (rand() % (relations+1) - 1);
                connections[connectionCount++] = i;
                connections[connectionCount++] = j;
                connections[connectionCount++] = relation;
            }
        }
    }

    Graph_ConnectNodes(g, connectionCount, connections);
    return g;
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

    EXPECT_EQ(nvals, 0);                     // No connection were formed.
    EXPECT_EQ(ncols, g->node_count);         // Graph's adjacency matrix dimensions.
    EXPECT_EQ(nrows, g->node_count);
    EXPECT_EQ(g->node_count, node_count);
    EXPECT_LE(g->node_count, g->node_cap);

    // Make sure we've received an iterator over created nodes.
    Node *n;
    unsigned int new_node_count = 0;
    while((n = NodeIterator_Next(it)) != NULL) { new_node_count++; }
    EXPECT_EQ(new_node_count, node_count);
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
    for(unsigned int i = 0; i < edge_count*3; i+=3) {
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
    EXPECT_EQ(nvals, edge_count);

    // Inspect graph matrices;
    // Graph's adjacency matrix should include all connections,
    // relation matrices should include edges of a certain relation.
    for(unsigned int i = 0; i < edge_count*3; i+=3) {
        int src_id = connections[i];
        int dest_id = connections[i+1];
        int r = connections[i+2];
        bool v = false;

        // src_id connected to dest_id.
        OK(GrB_Matrix_extractElement_BOOL(&v, g->adjacency_matrix, src_id, dest_id));
        EXPECT_TRUE(v);

        if(r != GRAPH_NO_RELATION) {
            // Test relation matrix.
            v = false;
            GrB_Matrix mat = Graph_GetRelationMatrix(g, r);
            OK(GrB_Matrix_extractElement_BOOL(&v, mat, src_id, dest_id));
            EXPECT_TRUE(v);
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
    EXPECT_EQ(g->node_count, prev_node_count + node_count);
    // Graph's adjacency matrix dimensions.
    OK(GrB_Matrix_nrows(&nrows, g->adjacency_matrix));
    OK(GrB_Matrix_ncols(&ncols, g->adjacency_matrix));
    EXPECT_EQ(ncols, g->node_count);
    EXPECT_EQ(nrows, g->node_count);
    EXPECT_LE(g->node_count, g->node_cap);

    // Verify number of created nodes.
    Node *n;
    unsigned int new_node_count = 0;
    NodeIterator *it = Graph_ScanNodes(g);
    while((n = NodeIterator_Next(it)) != NULL) { new_node_count++; }
    EXPECT_EQ(new_node_count, prev_node_count + node_count);
    NodeIterator_Free(it);

    // Relation matrices get resize lazily,
    // Try to fetch one of the specific relation matrices and verify its dimenstions.
    EXPECT_GT(g->relation_count, 0);
    for(unsigned int i = 0; i < g->relation_count; i++) {
        GrB_Matrix r = Graph_GetRelationMatrix(g, i);
        OK(GrB_Matrix_nrows(&nrows, r));
        OK(GrB_Matrix_ncols(&ncols, r));
        EXPECT_EQ(ncols, g->node_count);
        EXPECT_EQ(nrows, g->node_count);
    }
}

/* TODO benchmark functions are currently not invoked */
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
        for(unsigned int j = 0; j < n; j++) {
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
    double tic [2];
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
    double tic [2];
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

/*
int main(int argc, char **argv) {
    // Initialize GraphBLAS.
    GrB_Info info;
    OK(GrB_init(GrB_NONBLOCKING));
    srand(time(NULL));

    // benchmark_graph();

    GrB_finalize();

	printf("%stest_graph - PASS!%s\n", KGRN, KNRM);
    return 0;
}
*/

// Validate the creation of a graph,
// Make sure graph's defaults are applied.
TEST_F(GraphTest, NewGraph) {
    GrB_Info info;
    GrB_Index ncols, nrows, nvals;
    Graph *g = Graph_New(GRAPH_DEFAULT_NODE_CAP);

    OK(GrB_Matrix_ncols(&ncols, g->adjacency_matrix));
    OK(GrB_Matrix_nrows(&nrows, g->adjacency_matrix));
    OK(GrB_Matrix_nvals(&nvals, g->adjacency_matrix));

    EXPECT_NE(g->nodes_blocks, nullptr);
    EXPECT_NE(g->_relations, nullptr);
    EXPECT_NE(g->_labels, nullptr);
    EXPECT_NE(g->adjacency_matrix, nullptr);
    EXPECT_EQ(g->node_count, 0);
    EXPECT_EQ(g->node_cap, GRAPH_DEFAULT_NODE_CAP);
    EXPECT_EQ(nrows, GRAPH_DEFAULT_NODE_CAP);
    EXPECT_EQ(ncols, GRAPH_DEFAULT_NODE_CAP);
    EXPECT_EQ(nvals, 0);

    Graph_Free(g);
}

// Tests node and edge creation.
TEST_F(GraphTest, GraphConstruction) {
    size_t node_count = GRAPH_DEFAULT_NODE_CAP/2;
    Graph *g = Graph_New(node_count);
    _test_node_creation(g, node_count);
    _test_edge_creation(g, node_count);

    // Introduce additional nodes which will cause graph to resize.
    _test_graph_resize(g);

    Graph_Free(g);
}

TEST_F(GraphTest, RemoveNodes) {
    unsigned int nodeCount = 32;
    int relationCount = 4;

    Graph *g = _random_graph(nodeCount, relationCount);

    struct {
        GrB_Vector row;
        GrB_Vector col;
        GrB_Matrix m;
    } matrices [g->relation_count+1];

    matrices[0].m = g->adjacency_matrix;
    matrices[0].col = NULL;
    matrices[0].row = NULL;

    for(unsigned int i = 0; i < g->relation_count; i++) {
        matrices[i+1].m = Graph_GetRelationMatrix(g, i);
        matrices[i+1].col = NULL;
        matrices[i+1].row = NULL;
    }

    GrB_Descriptor desc;
    GrB_Descriptor_new(&desc);
    GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);

    // Remove all nodes, one node at a time.
    int nodesToDelete[1] = {0};
    for(unsigned int nodeID = 0; nodeID < nodeCount; nodeID++) {
        GrB_Index replacementID = g->node_count - 1;
        GrB_Index nrows = g->node_count;

        for(unsigned int i = 0; i < g->relation_count+1; i++) {
            if(matrices[i].col != NULL)
                GrB_Vector_free(&matrices[i].col);
            if(matrices[i].row != NULL)
                GrB_Vector_free(&matrices[i].row);

            GrB_Vector_new(&(matrices[i].col), GrB_BOOL, nrows);
            GrB_Vector_new(&(matrices[i].row), GrB_BOOL, nrows);

            // Extract replacemnt column.
            GrB_Col_extract(matrices[i].col, NULL, NULL, matrices[i].m, GrB_ALL, nrows, replacementID, NULL);
            // Set col at position nodeID to 0, as this entry going to be remove.
            GrB_Vector_setElement_BOOL(matrices[i].col, false, 0);
            // Extract replacemnt row.
            GrB_Col_extract(matrices[i].row, NULL, NULL, matrices[i].m, GrB_ALL, nrows, replacementID, desc);
            // Set row at position nodeID to 0, as this entry going to be remove.
            GrB_Vector_setElement_BOOL(matrices[i].row, false, 0);
        }
        Graph_DeleteNodes(g, nodesToDelete, 1);
        EXPECT_EQ(g->node_count, nodeCount - nodeID - 1);

        /* Make sure removed entity relations were updated
         * to the last entity relations. */
        // Validate updated column.
        for(unsigned int i = 0; i < g->relation_count+1; i++) {
            GrB_Matrix M;

            if(i == 0) M = matrices[0].m;   // Adjacency matrix.
            else M = Graph_GetRelationMatrix(g, i-1);   // Relation matrix.

            GrB_Vector col = matrices[i].col;
            GrB_Vector row = matrices[i].row;

            for(unsigned int rowIdx = 0; rowIdx < g->node_count; rowIdx++) {
                bool actual = false;
                bool expected = false;
                GrB_Matrix_extractElement_BOOL(&actual, M, rowIdx, 0);
                GrB_Vector_extractElement_BOOL(&expected, col, rowIdx);
                EXPECT_EQ(actual, expected);
            }

            // Validate updated row.
            for(unsigned int colIdx = 0; colIdx < g->node_count; colIdx++) {
                bool actual = false;
                bool expected = false;
                GrB_Matrix_extractElement_BOOL(&actual, M, 0, colIdx);
                GrB_Vector_extractElement_BOOL(&expected, row, colIdx);
                EXPECT_EQ(actual, expected);
            }
        }
    }

    // Validate empty matrix.
    EXPECT_EQ(g->node_count, 0);
    GrB_Index nvals;
    GrB_Matrix_nvals(&nvals, g->adjacency_matrix);
    EXPECT_EQ(nvals, 0);

    // Clean up.
    GrB_Descriptor_free(&desc);
    for(unsigned int i = 0; i < g->relation_count+1; i++) {
        GrB_Vector_free(&matrices[i].col);
        GrB_Vector_free(&matrices[i].row);
    }
    Graph_Free(g);
}


TEST_F(GraphTest, RemoveMultipleNodes) {
    // Delete two node.
    // One which is the latest node introduced to the graph
    // and the very first node.

    // Expecting the first node to be replaced with node at position
    // N-1, where N is the number of nodes in the graph.
    double tic [2];

    Graph *g = Graph_New(32);
    Graph_CreateNodes(g, 8, NULL, NULL);

    GrB_Index connections[3*9];

    // First node.
    connections[0] = 0;
    connections[1] = 2;
    connections[2] = GRAPH_NO_RELATION;

    connections[3] = 0;
    connections[4] = 6;
    connections[5] = GRAPH_NO_RELATION;

    connections[6] = 0;
    connections[7] = 7;
    connections[8] = GRAPH_NO_RELATION;

    // Right before last node.
    connections[9] = 6;
    connections[10] = 0;
    connections[11] = GRAPH_NO_RELATION;

    connections[12] = 6;
    connections[13] = 1;
    connections[14] = GRAPH_NO_RELATION;

    connections[15] = 6;
    connections[16] = 7;
    connections[17] = GRAPH_NO_RELATION;

    // Last node.
    connections[18] = 7;
    connections[19] = 0;
    connections[20] = GRAPH_NO_RELATION;

    connections[21] = 7;
    connections[22] = 1;
    connections[23] = GRAPH_NO_RELATION;

    connections[24] = 7;
    connections[25] = 6;
    connections[26] = GRAPH_NO_RELATION;

    Graph_ConnectNodes(g, 27, connections);

    // Delete first and last nodes.
    int nodeToDelete[2];
    nodeToDelete[0] = 7; // Remove last node.
    nodeToDelete[1] = 0; // Remove first node.

    simple_tic(tic);
    Graph_DeleteNodes(g, nodeToDelete, 2);
    double elapsed = simple_toc(tic);
    // printf("Nodes deletion took: %14.6f ms\n", elapsed*1000);

    EXPECT_EQ(g->node_count, 8-2);
    GrB_Descriptor desc;
    GrB_Descriptor_new(&desc);
    GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);

    // Validation.
    bool x = false;
    GrB_Vector row;
    GrB_Vector col;
    GrB_Index rowNvals;
    GrB_Index colNvals;

    GrB_Vector_new(&row, GrB_BOOL, g->node_count);
    GrB_Vector_new(&col, GrB_BOOL, g->node_count);

    // Make sure last and before last rows and column are zeroed out.
    GrB_Matrix adj = Graph_GetAdjacencyMatrix(g);

    GrB_Col_extract(row, NULL, NULL, adj, GrB_ALL, g->node_count, 7, desc);
    GrB_Col_extract(col, NULL, NULL, adj, GrB_ALL, g->node_count, 7, NULL);
    GrB_Vector_nvals(&rowNvals, row);
    GrB_Vector_nvals(&colNvals, col);
    EXPECT_EQ(rowNvals, 0);
    EXPECT_EQ(colNvals, 0);

    GrB_Col_extract(row, NULL, NULL, adj, GrB_ALL, g->node_count, 6, desc);
    GrB_Col_extract(col, NULL, NULL, adj, GrB_ALL, g->node_count, 6, NULL);
    GrB_Vector_nvals(&rowNvals, row);
    GrB_Vector_nvals(&colNvals, col);
    EXPECT_EQ(rowNvals, 0);
    EXPECT_EQ(colNvals, 0);

    // Validate replaced first row and column.
    GrB_Col_extract(row, NULL, NULL, adj, GrB_ALL, g->node_count, 0, desc);
    GrB_Col_extract(col, NULL, NULL, adj, GrB_ALL, g->node_count, 0, NULL);
    GrB_Vector_nvals(&rowNvals, row);
    GrB_Vector_nvals(&colNvals, col);

    GrB_Vector_extractElement_BOOL(&x, row, 1);
    EXPECT_EQ(rowNvals, 1);
    EXPECT_EQ(colNvals, 0);
    EXPECT_EQ(x, true);

    // Clean up.
    GrB_Descriptor_free(&desc);
    GrB_Vector_free(&row);
    GrB_Vector_free(&col);
    Graph_Free(g);
}

TEST_F(GraphTest, RemoveEdges) {
    GrB_Index row;
    GrB_Index col;
    GrB_Index relationEdgeCount;
    int nodeCount = 32;
    int edgeCount = 4;
    Graph *g = _random_graph(nodeCount, edgeCount);
    GrB_Matrix adj = Graph_GetAdjacencyMatrix(g);
    bool exists = false;

    // Delete every edge in the graph.
    for(unsigned int i = 0; i < g->relation_count; i++) {
        GrB_Matrix M = Graph_GetRelationMatrix(g, i);
        GrB_Matrix_nvals(&relationEdgeCount, M);
        int edgeToDelete[relationEdgeCount * 2];

        TuplesIter *iter = TuplesIter_new(M);

        int edgeIdx = 0;
        // We cannot modify the matrix being interated,
        // Collect edge indicies.
        while(TuplesIter_next(iter, &row, &col) != TuplesIter_DEPLETED) {
            edgeToDelete[edgeIdx++] = row;
            edgeToDelete[edgeIdx++] = col;
        }

        TuplesIter_free(iter);

        // Delete edges.
        for(unsigned int j = 0; j < relationEdgeCount*2; j+=2) {
            row = edgeToDelete[j];
            col = edgeToDelete[j+1];
            Graph_DeleteEdge(g, row, col);
        }

        // Validate delete.
        for(unsigned int j = 0; j < relationEdgeCount*2; j+=2) {
            exists = false;
            GrB_Matrix_extractElement_BOOL(&exists, adj, row, col);
            EXPECT_EQ(exists, false);
            GrB_Matrix_extractElement_BOOL(&exists, M, row, col);
            EXPECT_EQ(exists, false);
        }

        iter = TuplesIter_new(M);
        EXPECT_EQ(TuplesIter_next(iter, &row, &col), TuplesIter_DEPLETED);

        GrB_Matrix_nvals(&relationEdgeCount, M);
        EXPECT_EQ(relationEdgeCount, 0);
    }
}
