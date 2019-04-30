/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../../deps/googletest/include/gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../../src/util/arr.h"
#include "../../src/util/rmalloc.h"
#include "../../src/graph/query_graph.h"
#include "../../src/algorithms/algorithms.h"

#ifdef __cplusplus
}
#endif

class BFSTest: public ::testing::Test {
    protected:
    static void SetUpTestCase()
    {
        // Use the malloc family for allocations
        Alloc_Reset();
    }

    static QueryGraph* BuildGraph()
    {
        // (A)->(B)
        // (B)->(C)
        // (B)->(D)
        size_t node_cap = 4;
        size_t edge_cap = 3;

        // Create nodes.
        const char *label = "L";
        const char *relation = "R";

        Node *A = Node_New(label, "A");
        Node *B = Node_New(label, "B");
        Node *C = Node_New(label, "C");
        Node *D = Node_New(label, "D");

        Edge *AB = Edge_New(A, B, relation, "AB");
        Edge *BC = Edge_New(B, C, relation, "BC");
        Edge *BD = Edge_New(B, D, relation, "BD");

        QueryGraph *g = QueryGraph_New(node_cap, edge_cap);
        QueryGraph_AddNode(g, A);
        QueryGraph_AddNode(g, B);
        QueryGraph_AddNode(g, C);
        QueryGraph_AddNode(g, D);

        QueryGraph_ConnectNodes(g, A, B, AB);
        QueryGraph_ConnectNodes(g, B, C, BC);
        QueryGraph_ConnectNodes(g, B, D, BD);

        return g;
    }
};

TEST_F(BFSTest, BFSLevels) {
    Node *S;                    // BFS starts here.
    Node **nodes;               // Nodes reached by BFS.
    QueryGraph *g;              // Graph traversed.

    g = BuildGraph();
    S = QueryGraph_GetNodeByAlias(g, "A");

    Node *A = QueryGraph_GetNodeByAlias(g, "A");
    Node *B = QueryGraph_GetNodeByAlias(g, "B");
    Node *C = QueryGraph_GetNodeByAlias(g, "C");
    Node *D = QueryGraph_GetNodeByAlias(g, "D");

    Node *expected_level_0[1] = {A};
    Node *expected_level_1[1] = {B};
    Node *expected_level_2[2] = {C, D};
    Node *expected_level_3[0];
    Node *expected_level_deepest[2] = {C, D};
    
    Node **expected[4] = {
        expected_level_0,
        expected_level_1,
        expected_level_2,
        expected_level_3
    };

    //------------------------------------------------------------------------------
    // BFS depth 0 - 3
    //------------------------------------------------------------------------------

    for(int level = 0; level < 4; level++) {
        nodes = BFS(S, level);
        Node **expectation = expected[level];

        int node_count = array_len(nodes);
        for(int i = 0; i < node_count; i++) {
            bool found = false;
            for(int j = 0; j < node_count; j++) {
                if(nodes[i] == expectation[j]) {
                    found = true;
                    break;
                }
            }
            ASSERT_TRUE(found);
        }

        array_free(nodes); 
    }
   
    //------------------------------------------------------------------------------
    // BFS depth BFS_LOWEST_LEVEL
    //------------------------------------------------------------------------------

    nodes = BFS(S, BFS_LOWEST_LEVEL);

    // Determine number of expected nodes.
    int expected_node_count = sizeof(expected_level_deepest) / sizeof(expected_level_deepest[0]);
    ASSERT_EQ(expected_node_count, array_len(nodes));

    for(int i = 0; i < expected_node_count; i++) {
        bool found = false;
        for(int j = 0; j < expected_node_count; j++) {
            if(nodes[i] == expected_level_deepest[j]) {
                found = true;
                break;
            }
        }
        ASSERT_TRUE(found);
    }

    // Clean up.
    array_free(nodes);    
    QueryGraph_Free(g);
}
