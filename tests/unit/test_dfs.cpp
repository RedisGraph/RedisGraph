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

class DFSTest: public ::testing::Test {
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
        // (C)->(D)
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
        Edge *CD = Edge_New(C, D, relation, "CD");

        QueryGraph *g = QueryGraph_New(node_cap, edge_cap);
        QueryGraph_AddNode(g, A);
        QueryGraph_AddNode(g, B);
        QueryGraph_AddNode(g, C);
        QueryGraph_AddNode(g, D);

        QueryGraph_ConnectNodes(g, A, B, AB);
        QueryGraph_ConnectNodes(g, B, C, BC);
        QueryGraph_ConnectNodes(g, C, D, CD);

        return g;
    }
};

TEST_F(DFSTest, DFSLevels) {
    Node *S;        // DFS starts here.
    Edge **path;    // Path reached by DFS.
    QueryGraph *g;  // Graph traversed.

    g = BuildGraph();
    S = QueryGraph_GetNodeByAlias(g, "A");

    Edge *AB = QueryGraph_GetEdgeByAlias(g, "AB");
    Edge *BC = QueryGraph_GetEdgeByAlias(g, "BC");
    Edge *CD = QueryGraph_GetEdgeByAlias(g, "CD");

    Edge *expected_level_0[0] = {};
    Edge *expected_level_1[1] = {AB};
    Edge *expected_level_2[2] = {AB, BC};
    Edge *expected_level_3[3] = {AB, BC, CD};
    Edge *expected_level_4[0] = {};
    
    Edge **expected[5] = {
        expected_level_0,
        expected_level_1,
        expected_level_2,
        expected_level_3,
        expected_level_4
    };

    //------------------------------------------------------------------------------
    // DFS depth 0 - 4
    //------------------------------------------------------------------------------

    for(int level = 0; level < 5; level++) {
        path = DFS(S, level);
        Edge **expectation = expected[level];

        int edge_count = array_len(path);
        for(int i = 0; i < edge_count; i++) {
            bool found = false;
            for(int j = 0; j < edge_count; j++) {
                if(path[i] == expectation[j]) {
                    found = true;
                    break;
                }
            }
            ASSERT_TRUE(found);
        }
        array_free(path); 
    }

    // Clean up.
    QueryGraph_Free(g);
}
