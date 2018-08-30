/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "../../deps/googletest/include/gtest/gtest.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../src/graph/graph.h"
#include "../../src/util/simple_timer.h"
#include "../../src/arithmetic/tuples_iter.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"
#include "../../src/util/datablock/datablock_iterator.h"

#ifdef __cplusplus
}
#endif

// Console text colors for benchmark printing
#define KGRN "\x1B[32m"
#define KRED "\x1B[31m"
#define KNRM "\x1B[0m"

class GraphTest : public ::testing::Test
{
  protected:
    static void SetUpTestCase()
    {
        // Initialize GraphBLAS.
        GrB_init(GrB_NONBLOCKING);
        srand(time(NULL));
    }

    static void TearDownTestCase()
    {
        GrB_finalize();
    }

    Graph *_random_graph(int nodes, int relations)
    {
        Graph *g = Graph_New(nodes);
        Graph_CreateNodes(g, nodes, NULL, NULL);

        for (int i = 0; i < relations; i++)
            Graph_AddRelationMatrix(g);

        int connectionCount = 0;
        EdgeDesc *connections = (EdgeDesc*)malloc(sizeof(EdgeDesc) * nodes * nodes);

        double mid_point = RAND_MAX / 2;
        for (int i = 0; i < nodes; i++)
        {
            for (int j = 0; j < nodes; j++)
            {
                if (rand() > mid_point)
                {
                    if (i == j) continue;
                    int relation = rand() % relations;
                    connections[connectionCount].srcId = i;
                    connections[connectionCount].destId = j;
                    connections[connectionCount].relationId = relation;
                    connectionCount++;
                }
            }
        }

        Graph_ConnectNodes(g, connections, connectionCount, NULL);
        return g;
    }

    void _print_matrix(GrB_Matrix M)
    {
        GrB_Index nrows;
        GrB_Index ncols;
        GrB_Matrix_nrows(&nrows, M);
        GrB_Matrix_ncols(&ncols, M);

        for (unsigned int i = 0; i < nrows; i++)
        {
            for (unsigned int j = 0; j < ncols; j++)
            {
                bool x = false;
                GrB_Matrix_extractElement_BOOL(&x, M, i, j);
                printf("%d ", x);
            }
            printf("\n");
        }
        printf("\n\n\n");
    }

    void _test_node_creation(Graph *g, size_t node_count)
    {
        GrB_Index ncols, nrows, nvals;
        DataBlockIterator *it;

        // Create nodes.
        Graph_CreateNodes(g, node_count, NULL, &it);

        // Validate nodes creation.
        EXPECT_EQ(GrB_Matrix_nrows(&nrows, g->adjacency_matrix), GrB_SUCCESS);
        EXPECT_EQ(GrB_Matrix_ncols(&ncols, g->adjacency_matrix), GrB_SUCCESS);
        EXPECT_EQ(GrB_Matrix_nvals(&nvals, g->adjacency_matrix), GrB_SUCCESS);

        EXPECT_EQ(nvals, 0);                  // No connection were formed.
        EXPECT_EQ(ncols, Graph_NodeCount(g)); // Graph's adjacency matrix dimensions.
        EXPECT_EQ(nrows, Graph_NodeCount(g));
        EXPECT_EQ(Graph_NodeCount(g), node_count);

        // Make sure we've received an iterator over created nodes.
        Node *n;
        unsigned int new_node_count = 0;
        while ((n = (Node *)DataBlockIterator_Next(it)) != NULL)
        {
            new_node_count++;
        }
        EXPECT_EQ(new_node_count, node_count);
        DataBlockIterator_Free(it);
    }

    void _test_edge_creation(Graph *g, size_t node_count)
    {
        // Form connections.
        int relationCount = 3;
        size_t edge_count = (node_count - 1);
        EdgeDesc connections[edge_count];

        // Introduce relations types.
        for (int i = 0; i < relationCount; i++)
            Graph_AddRelationMatrix(g);

        // Describe connections;
        // Node I is connected to Node I+1,
        // Connection type is relationships[I%4].
        int node_id = 0;
        for (unsigned int i = 0; i < edge_count; i++)
        {
            connections[i].srcId = node_id;                     // Source node id.
            connections[i].destId = node_id + 1;                // Destination node id.
            connections[i].relationId = (i % relationCount);    // Relation.
            node_id++;
        }

        Graph_ConnectNodes(g, connections, edge_count, NULL);

        // Validate edges creation,
        // expecting number of none zero entries to be edge_count.
        GrB_Index nvals;
        EXPECT_EQ(GrB_Matrix_nvals(&nvals, g->adjacency_matrix), GrB_SUCCESS);
        EXPECT_EQ(nvals, edge_count);

        // Inspect graph matrices;
        // Graph's adjacency matrix should include all connections,
        // relation matrices should include edges of a certain relation.
        for (unsigned int i = 0; i < edge_count; i++)
        {
            int src_id = connections[i].srcId;
            int dest_id = connections[i].destId;
            int r = connections[i].relationId;
            bool v = false;

            // src_id connected to dest_id.
            EXPECT_EQ(GrB_Matrix_extractElement_BOOL(&v, g->adjacency_matrix, dest_id, src_id), GrB_SUCCESS);
            EXPECT_TRUE(v);

            // Test relation matrix.
            v = false;
            GrB_Matrix mat = Graph_GetRelationMatrix(g, r);
            EXPECT_EQ(GrB_Matrix_extractElement_BOOL(&v, mat, dest_id, src_id), GrB_SUCCESS);
            EXPECT_TRUE(v);
        }
    }

    void _test_graph_resize(Graph *g)
    {
        GrB_Index ncols, nrows;
        size_t prev_node_count = Graph_NodeCount(g);
        size_t node_count = prev_node_count * 16;

        Graph_CreateNodes(g, node_count, NULL, NULL);

        // Validate nodes creation.
        EXPECT_EQ(Graph_NodeCount(g), prev_node_count + node_count);
        // Graph's adjacency matrix dimensions.
        EXPECT_EQ(GrB_Matrix_nrows(&nrows, g->adjacency_matrix), GrB_SUCCESS);
        EXPECT_EQ(GrB_Matrix_ncols(&ncols, g->adjacency_matrix), GrB_SUCCESS);
        EXPECT_EQ(ncols, Graph_NodeCount(g));
        EXPECT_EQ(nrows, Graph_NodeCount(g));

        // Verify number of created nodes.
        Node *n;
        unsigned int new_node_count = 0;
        DataBlockIterator *it = Graph_ScanNodes(g);
        while ((n = (Node *)DataBlockIterator_Next(it)) != NULL)
        {
            new_node_count++;
        }
        EXPECT_EQ(new_node_count, prev_node_count + node_count);
        DataBlockIterator_Free(it);

        // Relation matrices get resize lazily,
        // Try to fetch one of the specific relation matrices and verify its dimenstions.
        EXPECT_GT(g->relation_count, 0);
        for (unsigned int i = 0; i < g->relation_count; i++)
        {
            GrB_Matrix r = Graph_GetRelationMatrix(g, i);
            EXPECT_EQ(GrB_Matrix_nrows(&nrows, r), GrB_SUCCESS);
            EXPECT_EQ(GrB_Matrix_ncols(&ncols, r), GrB_SUCCESS);
            EXPECT_EQ(ncols, Graph_NodeCount(g));
            EXPECT_EQ(nrows, Graph_NodeCount(g));
        }
    }
};

/* TODO benchmark functions are currently not invoked */
void benchmark_node_creation_with_labels()
{
    printf("benchmark_node_creation_with_labels\n");
    double tic[2];
    int samples = 64;
    int label_count = 3;
    double timings[samples];
    int outliers = 0;
    float threshold = 0.0018;
    // size_t n = GRAPH_DEFAULT_NODE_CAP;
    size_t n = 1000000;
    Graph *g = Graph_New(n);

    // Introduce labels and relations to graph.
    for (int i = 0; i < label_count; i++)
    {
        Graph_AddRelationMatrix(g); // Typed relation.
        Graph_AddLabelMatrix(g);    // Typed node.
    }

    int labels[n];

    // Create N nodes with labels.
    for (int i = 0; i < samples; i++)
    {
        // Associate nodes to labels.
        for (unsigned int j = 0; j < n; j++)
        {
            labels[j] = (rand() % label_count) - 1;
        }

        simple_tic(tic);
        Graph_CreateNodes(g, n, labels, NULL);
        timings[i] = simple_toc(tic);
        printf("%zu Nodes created, time: %.6f sec\n", n, timings[i]);
        if (timings[i] > threshold)
            outliers++;
    }

    if (outliers > samples * 0.1)
    {
        printf("Node creation took too long\n");
        for (int i = 0; i < samples; i++)
        {
            printf("%zu Nodes created, time: %.6f sec\n", n, timings[i]);
        }
        // assert(false);
    }

    Graph_Free(g);
}

// Test graph creation time.
void benchmark_node_creation_no_labels()
{
    printf("benchmark_node_creation_no_labels\n");
    double tic[2];
    int samples = 64;
    double timings[samples];
    int outliers = 0;
    float threshold = 0.000006;
    // size_t n = GRAPH_DEFAULT_NODE_CAP;
    size_t n = 1000000;
    Graph *g = Graph_New(n);

    for (int i = 0; i < samples; i++)
    {
        // Create N nodes, don't use labels.
        simple_tic(tic);
        Graph_CreateNodes(g, n, NULL, NULL);
        timings[i] = simple_toc(tic);
        printf("%zu Nodes created, time: %.6f sec\n", n, timings[i]);
        if (timings[i] > threshold)
            outliers++;
    }

    if (outliers > samples * 0.1)
    {
        printf("Node creation took too long\n");
        for (int i = 0; i < samples; i++)
        {
            printf("%zu Nodes created, time: %.6f sec\n", n, timings[i]);
        }
        // assert(false);
    }

    Graph_Free(g);
}



void benchmark_edge_creation_with_relationships()
{
    printf("benchmark_edge_creation_with_relationships\n");
    double tic[2];
    int samples = 64;
    double timings[samples];
    int outliers = 0;
    float threshold = 0.002;
    int edge_count = 1000000 * 1.10;
    int node_count = 1000000;
    int relation_count = 3;
    EdgeDesc connections[edge_count];

    Graph *g = Graph_New(GRAPH_DEFAULT_NODE_CAP);

    // Introduce relations types.
    for (int i = 0; i < relation_count; i++)
        Graph_AddRelationMatrix(g);

    Graph_CreateNodes(g, node_count, NULL, NULL);

    for (int i = 0; i < samples; i++)
    {
        // Describe connections;
        // Node I is connected to Node I+1,
        // Connection type is relationships[I%4].
        for (int j = 0; j < edge_count; j++)
        {
            connections[j].srcId = rand() % node_count;     // Source node id.
            connections[j].destId = rand() % node_count;    // Destination node id.
            connections[j].relationId = i % relation_count; // Relation.
        }

        simple_tic(tic);
        Graph_ConnectNodes(g, connections, edge_count, NULL);
        timings[i] = simple_toc(tic);
        printf("%d Formed connections, time: %.6f sec\n", edge_count, timings[i]);
        if (timings[i] > threshold)
            outliers++;
    }

    if (outliers > samples * 0.1)
    {
        printf("Node creation took too long\n");
        for (int i = 0; i < samples; i++)
        {
            printf("%d Formed connections, time: %.6f sec\n", edge_count, timings[i]);
        }
        // assert(false);
    }

    Graph_Free(g);
}

void benchmark_graph()
{
    benchmark_node_creation_no_labels();
    benchmark_node_creation_with_labels();
    benchmark_edge_creation_with_relationships();
    printf("%sgraph benchmark - PASS!%s\n", KGRN, KNRM);
}

// Validate the creation of a graph,
// Make sure graph's defaults are applied.
TEST_F(GraphTest, NewGraph)
{
    GrB_Index ncols, nrows, nvals;
    Graph *g = Graph_New(GRAPH_DEFAULT_NODE_CAP);

    EXPECT_EQ(GrB_Matrix_ncols(&ncols, g->adjacency_matrix), GrB_SUCCESS);
    EXPECT_EQ(GrB_Matrix_nrows(&nrows, g->adjacency_matrix), GrB_SUCCESS);
    EXPECT_EQ(GrB_Matrix_nvals(&nvals, g->adjacency_matrix), GrB_SUCCESS);

    EXPECT_TRUE(g->nodes != NULL);
    EXPECT_TRUE(g->_relations != NULL);
    EXPECT_TRUE(g->_labels != NULL);
    EXPECT_TRUE(g->adjacency_matrix != NULL);
    EXPECT_EQ(Graph_NodeCount(g), 0);
    EXPECT_EQ(nrows, GRAPH_DEFAULT_NODE_CAP);
    EXPECT_EQ(ncols, GRAPH_DEFAULT_NODE_CAP);
    EXPECT_EQ(nvals, 0);

    Graph_Free(g);
}

// Tests node and edge creation.
TEST_F(GraphTest, GraphConstruction)
{
    size_t node_count = GRAPH_DEFAULT_NODE_CAP / 2;
    Graph *g = Graph_New(node_count);
    _test_node_creation(g, node_count);
    // _test_edge_creation(g, node_count);

    // Introduce additional nodes which will cause graph to resize.
    // _test_graph_resize(g);

    Graph_Free(g);
}

TEST_F(GraphTest, RemoveNodes)
{
    unsigned int nodeCount = 32;
    int relationCount = 4;

    Graph *g = _random_graph(nodeCount, relationCount);

    struct
    {
        GrB_Vector row;
        GrB_Vector col;
        GrB_Matrix m;
    } matrices[g->relation_count + 1];

    matrices[0].m = g->adjacency_matrix;
    matrices[0].col = NULL;
    matrices[0].row = NULL;

    for (unsigned int i = 0; i < g->relation_count; i++)
    {
        matrices[i + 1].m = Graph_GetRelationMatrix(g, i);
        matrices[i + 1].col = NULL;
        matrices[i + 1].row = NULL;
    }

    GrB_Descriptor desc;
    GrB_Descriptor_new(&desc);
    GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);

    // Remove all nodes, one node at a time.
    NodeID nodesToDelete[1] = {0};
    for (unsigned int nodeID = 0; nodeID < nodeCount; nodeID++)
    {
        GrB_Index replacementID = Graph_NodeCount(g) - 1;
        GrB_Index nrows = Graph_NodeCount(g);

        for (unsigned int i = 0; i < g->relation_count + 1; i++)
        {
            if (matrices[i].col != NULL)
                GrB_Vector_free(&matrices[i].col);
            if (matrices[i].row != NULL)
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
        EXPECT_EQ(Graph_NodeCount(g), nodeCount - nodeID - 1);

        /* Make sure removed entity relations were updated
         * to the last entity relations. */
        // Validate updated column.
        for (unsigned int i = 0; i < g->relation_count + 1; i++)
        {
            GrB_Matrix M;

            if (i == 0)
                M = matrices[0].m; // Adjacency matrix.
            else
                M = Graph_GetRelationMatrix(g, i - 1); // Relation matrix.

            GrB_Vector col = matrices[i].col;
            GrB_Vector row = matrices[i].row;

            for (unsigned int rowIdx = 0; rowIdx < Graph_NodeCount(g); rowIdx++)
            {
                bool actual = false;
                bool expected = false;
                GrB_Matrix_extractElement_BOOL(&actual, M, rowIdx, 0);
                GrB_Vector_extractElement_BOOL(&expected, col, rowIdx);
                EXPECT_EQ(actual, expected);
            }

            // Validate updated row.
            for (unsigned int colIdx = 0; colIdx < Graph_NodeCount(g); colIdx++)
            {
                bool actual = false;
                bool expected = false;
                GrB_Matrix_extractElement_BOOL(&actual, M, 0, colIdx);
                GrB_Vector_extractElement_BOOL(&expected, row, colIdx);
                EXPECT_EQ(actual, expected);
            }
        }
    }

    // Validate empty matrix.
    EXPECT_EQ(Graph_NodeCount(g), 0);
    GrB_Index nvals;
    GrB_Matrix_nvals(&nvals, g->adjacency_matrix);
    EXPECT_EQ(nvals, 0);

    // Clean up.
    GrB_Descriptor_free(&desc);
    for (unsigned int i = 0; i < g->relation_count + 1; i++)
    {
        GrB_Vector_free(&matrices[i].col);
        GrB_Vector_free(&matrices[i].row);
    }
    Graph_Free(g);
}

TEST_F(GraphTest, RemoveMultipleNodes)
{
    // Delete two node.
    // One which is the latest node introduced to the graph
    // and the very first node.

    // Expecting the first node to be replaced with node at position
    // N-1, where N is the number of nodes in the graph.
    double tic[2];

    Graph *g = Graph_New(32);
    Graph_CreateNodes(g, 8, NULL, NULL);
    int relation = Graph_AddRelationMatrix(g);

    EdgeDesc connections[9];

    // First node.
    connections[0].srcId = 0;
    connections[0].destId = 2;
    connections[0].relationId = relation;

    connections[1].srcId = 0;
    connections[1].destId = 6;
    connections[1].relationId = relation;

    connections[2].srcId = 0;
    connections[2].destId = 7;
    connections[2].relationId = relation;

    // Right before last node.
    connections[3].srcId = 6;
    connections[3].destId = 0;
    connections[3].relationId = relation;

    connections[4].srcId = 6;
    connections[4].destId = 1;
    connections[4].relationId = relation;

    connections[5].srcId = 6;
    connections[5].destId = 7;
    connections[5].relationId = relation;

    // Last node.
    connections[6].srcId = 7;
    connections[6].destId = 0;
    connections[6].relationId = relation;

    connections[7].srcId = 7;
    connections[7].destId = 1;
    connections[7].relationId = relation;

    connections[8].srcId = 7;
    connections[8].destId = 6;
    connections[8].relationId = relation;

    Graph_ConnectNodes(g, connections, 9, NULL);

    // Delete first and last nodes.
    NodeID nodeToDelete[2];
    nodeToDelete[0] = 0; // Remove first node.
    nodeToDelete[1] = 7; // Remove last node.

    simple_tic(tic);
    Graph_DeleteNodes(g, nodeToDelete, 2);

    EXPECT_EQ(Graph_NodeCount(g), 8 - 2);
    GrB_Descriptor desc;
    GrB_Descriptor_new(&desc);
    GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);

    // Validation.
    bool x = false;
    GrB_Vector row;
    GrB_Vector col;
    GrB_Index rowNvals;
    GrB_Index colNvals;

    GrB_Vector_new(&row, GrB_BOOL, Graph_NodeCount(g));
    GrB_Vector_new(&col, GrB_BOOL, Graph_NodeCount(g));

    // Make sure last and before last rows and column are zeroed out.
    GrB_Matrix adj = Graph_GetAdjacencyMatrix(g);

    GrB_Col_extract(row, NULL, NULL, adj, GrB_ALL, Graph_NodeCount(g), 7, desc);
    GrB_Col_extract(col, NULL, NULL, adj, GrB_ALL, Graph_NodeCount(g), 7, NULL);
    GrB_Vector_nvals(&rowNvals, row);
    GrB_Vector_nvals(&colNvals, col);
    EXPECT_EQ(rowNvals, 0);
    EXPECT_EQ(colNvals, 0);

    GrB_Col_extract(row, NULL, NULL, adj, GrB_ALL, Graph_NodeCount(g), 6, desc);
    GrB_Col_extract(col, NULL, NULL, adj, GrB_ALL, Graph_NodeCount(g), 6, NULL);
    GrB_Vector_nvals(&rowNvals, row);
    GrB_Vector_nvals(&colNvals, col);
    EXPECT_EQ(rowNvals, 0);
    EXPECT_EQ(colNvals, 0);

    // Validate replaced first row and column.
    GrB_Col_extract(row, NULL, NULL, adj, GrB_ALL, Graph_NodeCount(g), 0, desc);
    GrB_Col_extract(col, NULL, NULL, adj, GrB_ALL, Graph_NodeCount(g), 0, NULL);
    GrB_Vector_nvals(&rowNvals, row);
    GrB_Vector_nvals(&colNvals, col);

    GrB_Vector_extractElement_BOOL(&x, col, 1);
    EXPECT_EQ(rowNvals, 0);
    EXPECT_EQ(colNvals, 1);
    EXPECT_EQ(x, true);

    // Clean up.
    GrB_Descriptor_free(&desc);
    GrB_Vector_free(&row);
    GrB_Vector_free(&col);
    Graph_Free(g);
}

TEST_F(GraphTest, RemoveEdges)
{
    NodeID src;
    NodeID dest;
    GrB_Index relationEdgeCount;
    int nodeCount = 32;
    int edgeCount = 4;
    Graph *g = _random_graph(nodeCount, edgeCount);
    GrB_Matrix adj = Graph_GetAdjacencyMatrix(g);
    bool exists = false;

    // Delete every edge in the graph.
    for (unsigned int i = 0; i < g->relation_count; i++)
    {
        GrB_Matrix M = Graph_GetRelationMatrix(g, i);
        GrB_Matrix_nvals(&relationEdgeCount, M);
        int edgeToDelete[relationEdgeCount * 2];

        TuplesIter *iter = TuplesIter_new(M);

        int edgeIdx = 0;
        // We cannot modify the matrix being interated,
        // Collect edge indicies.
        while (TuplesIter_next(iter, &dest, &src) != TuplesIter_DEPLETED)
        {
            edgeToDelete[edgeIdx++] = src;
            edgeToDelete[edgeIdx++] = dest;
        }

        TuplesIter_free(iter);

        // Delete edges.
        for (unsigned int j = 0; j < relationEdgeCount * 2; j += 2)
        {
            src = edgeToDelete[j];
            dest = edgeToDelete[j + 1];
            Graph_DeleteEdge(g, src, dest, GRAPH_NO_RELATION);
        }

        // Validate delete.
        for (unsigned int j = 0; j < relationEdgeCount * 2; j += 2)
        {
            exists = false;
            GrB_Matrix_extractElement_BOOL(&exists, adj, dest, src);
            EXPECT_EQ(exists, false);
            GrB_Matrix_extractElement_BOOL(&exists, M, dest, src);
            EXPECT_EQ(exists, false);
        }

        iter = TuplesIter_new(M);
        EXPECT_EQ(TuplesIter_next(iter, &dest, &src), TuplesIter_DEPLETED);

        GrB_Matrix_nvals(&relationEdgeCount, M);
        EXPECT_EQ(relationEdgeCount, 0);
    }
}

TEST_F(GraphTest, GetNode)
{
    /* Create a graph with nodeCount nodes,
     * Make sure node retrival works as expected:
     * 1. try to get nodes (0 - nodeCount)
     * 2. try to get node with ID >= nodeCount. */

    Node *n = NULL;
    size_t nodeCount = 16;
    Graph *g = Graph_New(nodeCount);
    Graph_CreateNodes(g, nodeCount, NULL, NULL);
    
    // Get nodes 0 - nodeCount.
    for(NodeID i = 0; i < nodeCount; i++) {
        n = Graph_GetNode(g, i);
        EXPECT_TRUE(n != NULL);
    }

    Graph_Free(g);
}

TEST_F(GraphTest, GetEdge)
{
    /* Create a graph with both nodes and edges.
     * Make sure edge retrival works as expected: 
     * 1. try to get edges by ID 
     * 2. try to get edges connecting source to destination node 
     * 3. try to get none existing edges. */

    Edge *e = NULL;
    int relationCount = 4;
    size_t nodeCount = 5;
    size_t edgeCount = 5;
    int relations[relationCount];

    Graph *g = Graph_New(nodeCount);
    Graph_CreateNodes(g, nodeCount, NULL, NULL);

    for(int i = 0; i < relationCount; i++) {
        relations[i] = Graph_AddRelationMatrix(g);
    }

    /* Connect nodes:
     * 1. nodes (0-1) will be connected by relation 0. 
     * 2. nodes (0-1) will be connected by relation 1.
     * 3. nodes (1-2) will be connected by relation 1.
     * 4. nodes (2-3) will be connected by relation 2.
     * 5. nodes (3-4) will be connected by relation 3. */

    size_t connectionCount = edgeCount;
    EdgeDesc connections[connectionCount];
    
    connections[0].srcId = 0;
    connections[0].destId = 1;
    connections[0].relationId = relations[0];

    connections[1].srcId = 0;
    connections[1].destId = 1;
    connections[1].relationId = relations[1];

    connections[2].srcId = 1;
    connections[2].destId = 2;
    connections[2].relationId = relations[1];

    connections[3].srcId = 2;
    connections[3].destId = 3;
    connections[3].relationId = relations[2];
    
    connections[4].srcId = 3;
    connections[4].destId = 4;
    connections[4].relationId = relations[3];

    Graph_ConnectNodes(g, connections, connectionCount, NULL);

    // Validations
    // Try to get edges by ID
    for(EdgeID i = 0; i < edgeCount; i++) {
        e = Graph_GetEdge(g, i);
        EXPECT_TRUE(e != NULL);
        EXPECT_EQ(e->id, i);
    }

    // Try to get edges connecting source to destination node.
    NodeID src;
    NodeID dest;
    int relation;
    EdgeIterator *it = EdgeIterator_New();

    /* Get all edges connecting node 0 to node 1,
     * expecting 2 edges. */
    src = 0;
    dest = 1;
    relation = GRAPH_NO_RELATION;   // Relation agnostic.
    Graph_GetEdgesConnectingNodes(g, src, dest, relation, it);
    EXPECT_EQ(it->edgeCount, 2);
    for(int i = 0; i < 2; i++) {
        e = EdgeIterator_Next(it);
        relation = relations[i];
        EXPECT_TRUE(e != NULL);
        EXPECT_EQ(Edge_GetSrcNodeID(e), src);
        EXPECT_EQ(Edge_GetDestNodeID(e), dest);
        EXPECT_EQ(Edge_GetRelationID(e), relation);
    }
    EdgeIterator_Reuse(it);

    // Try to get none existing edges:

    // Node 0 is not connected to 2.
    src = 0;
    dest = 2;
    relation = GRAPH_NO_RELATION;
    Graph_GetEdgesConnectingNodes(g, src, dest, relation, it);
    EXPECT_EQ(it->edgeCount, 0);
    EdgeIterator_Reset(it); // Reset edge count.

    // Node 0 is not connected to 1 via relation 2.
    src = 0;
    dest = 1;
    relation = relations[2];
    Graph_GetEdgesConnectingNodes(g, src, dest, relation, it);
    EXPECT_EQ(it->edgeCount, 0);
    EdgeIterator_Reset(it); // Reset edge count.

    // Node 1 is not connected to 0 via relation 0.
    src = 1;
    dest = 0;
    relation = relations[0];
    Graph_GetEdgesConnectingNodes(g, src, dest, relation, it);
    EXPECT_EQ(it->edgeCount, 0);
    EdgeIterator_Reset(it); // Reset edge count.

    // No node connects to itself.
    for(NodeID i = 0; i < nodeCount; i++) {
        for(int j = 0; j < relationCount; j++) {
            src = i;
            relation = relations[j];
            Graph_GetEdgesConnectingNodes(g, src, src, relation, it);
            EXPECT_EQ(it->edgeCount, 0);
            EdgeIterator_Reset(it); // Reset edge count.
        }
        relation = GRAPH_NO_RELATION;
        Graph_GetEdgesConnectingNodes(g, src, src, relation, it);
        EXPECT_EQ(it->edgeCount, 0);
        EdgeIterator_Reset(it); // Reset edge count.
    }

    EdgeIterator_Free(it);
    Graph_Free(g);
}
