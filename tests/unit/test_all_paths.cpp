/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Apache License, Version 2.0,
 * modified with the Commons Clause restriction.
 */

#include "../../deps/googletest/include/gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../../src/algorithms/algorithms.h"
#include "../../src/util/arr.h"

#ifdef __cplusplus
}
#endif

class AllPathsTest: public ::testing::Test {
    protected:
    static void SetUpTestCase()
    {
        // Initialize GraphBLAS.
        GrB_init(GrB_NONBLOCKING);
    }

    static void TearDownTestCase()
    {
        GrB_finalize();
    }

    static Graph* BuildGraph()
    {
        size_t nodeCount = 4;

        Graph *g = Graph_New(nodeCount);
        int relation = Graph_AddRelation(g);
        Graph_CreateNodes(g, nodeCount, NULL, NULL);

        /* Connections:
         * 0 -> 1
         * 0 -> 2
         * 1 -> 0
         * 1 -> 2
         * 2 -> 1
         * 2 -> 3
         * 3 -> 0 */

        int connectionCount = 7;
        EdgeDesc *connections = (EdgeDesc*)malloc(sizeof(EdgeDesc) * connectionCount);
        
        // Connections:
        // 0 -> 1
        connections[0].srcId = 0;
        connections[0].destId = 1;
        connections[0].relationId = relation;

        // 0 -> 2
        connections[1].srcId = 0;
        connections[1].destId = 2;
        connections[1].relationId = relation;

        // 1 -> 0
        connections[2].srcId = 1;
        connections[2].destId = 0;
        connections[2].relationId = relation;

        // 1 -> 2
        connections[3].srcId = 1;
        connections[3].destId = 2;
        connections[3].relationId = relation;

        // 2 -> 1
        connections[4].srcId = 2;
        connections[4].destId = 1;
        connections[4].relationId = relation;

        // 2 -> 3
        connections[5].srcId = 2;
        connections[5].destId = 3;
        connections[5].relationId = relation;

        // 3 -> 0
        connections[6].srcId = 3;
        connections[6].destId = 0;
        connections[6].relationId = relation;

        Graph_ConnectNodes(g, connections, connectionCount, NULL);        
        free(connections);
        return g;
    }
};

TEST_F(AllPathsTest, NoPaths) {
    Graph *g = BuildGraph();

    NodeID src = 0;
    unsigned int minLen = 999;
    unsigned int maxLen = minLen + 1;
    size_t pathCount = 0;
    Path *paths = array_new(Path, 0);
    AllPaths(g, GRAPH_NO_RELATION, src, minLen, maxLen, &paths);
    EXPECT_EQ(array_len(paths), 0);
    Graph_Free(g);
}

TEST_F(AllPathsTest, LongestPaths) {
    Graph *g = BuildGraph();

    NodeID src = 0;
    unsigned int minLen = 0;
    unsigned int maxLen = UINT_MAX;
    size_t pathCount = 0;
    Path *paths = array_new(Path, 0);
    AllPaths(g, GRAPH_NO_RELATION, src, minLen, maxLen, &paths);

    unsigned int longestPath = 0;
    for(int i = 0; i < pathCount; i++) {
        size_t pathLen = Path_len(paths[i]);
        if(longestPath < pathLen) longestPath = pathLen;
    }

    // 0,2,3,0,1,2,1,0
    EXPECT_EQ(longestPath, 7);
}

TEST_F(AllPathsTest, UpToThreeLegsPaths) {
    Graph *g = BuildGraph();

    NodeID src = 0;
    unsigned int minLen = 1;
    unsigned int maxLen = 3;
    size_t pathCount = 0;
    Path *paths = array_new(Path, 12);
    AllPaths(g, GRAPH_NO_RELATION, src, minLen, maxLen, &paths);
    ASSERT_EQ(array_len(paths), 12);

    /* Connections:
     * 0 -> 1
     * 0 -> 2
     * 1 -> 0
     * 1 -> 2
     * 2 -> 1
     * 2 -> 3
     * 3 -> 0 */

    // One leg paths.
    NodeID p0[3] = {1, 0, 1};
    NodeID p1[3] = {1, 0, 2};

    // Two leg paths.
    NodeID p2[4] = {2, 0, 1, 0};
    NodeID p3[4] = {2, 0, 1, 2};
    NodeID p4[4] = {2, 0, 2, 1};
    NodeID p5[4] = {2, 0, 2, 3};

    // Three leg paths.
    NodeID p6[5] = {3, 0, 1, 0, 2}; // Can't reuse edge (0,1)!
    NodeID p7[5] = {3, 0, 1, 2, 1};
    NodeID p8[5] = {3, 0, 1, 2, 3};
    NodeID p9[5] = {3, 0, 2, 1, 0};
    NodeID p10[5] = {3, 0, 2, 1, 2};
    NodeID p11[5] = {3, 0, 2, 3, 0};

    NodeID *expectedPaths[12] = {p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11};
    

    for(int i = 0; i < pathCount; i++) {
        NodeID *expectedPath = expectedPaths[i];
        size_t expectedPathLen = expectedPath[0];
        expectedPath++; // Skip path length.
        bool expectedPathFound = false;

        for(int j = 0; j < pathCount; j++) {
            Path p = paths[j];
            if(Path_len(p) != expectedPathLen) continue;
            
            int k = 0;
            for(; k < expectedPathLen; k++) {
                Edge *e = p[k];
                if(Edge_GetSrcNodeID(e) != expectedPath[k]) break;
                if(Edge_GetDestNodeID(e) != expectedPath[k+1]) break;
            }
            if(k == expectedPathLen) {
                expectedPathFound = true;
                break;
            }
        }
        EXPECT_TRUE(expectedPathFound);
    }

    Graph_Free(g);
}

TEST_F(AllPathsTest, TwoLegPaths) {
    Graph *g = BuildGraph();

    NodeID src = 0;
    unsigned int minLen = 2;
    unsigned int maxLen = 2;
    size_t pathCount = 0;
    Path *paths = array_new(Path, 4);
    AllPaths(g, GRAPH_NO_RELATION, src, minLen, maxLen, &paths);
    ASSERT_EQ(array_len(paths), 4);

    /* Connections:
     * 0 -> 1
     * 0 -> 2
     * 1 -> 0
     * 1 -> 2
     * 2 -> 1
     * 2 -> 3
     * 3 -> 0 */
    NodeID p0[3] = {0, 1, 0};
    NodeID p1[3] = {0, 1, 2};
    NodeID p2[3] = {0, 2, 1};
    NodeID p3[3] = {0, 2, 3};
    NodeID *expectedPaths[4] = {p0, p1, p2, p3};

    
    for(int i = 0; i < pathCount; i++) {
        NodeID *expectedPath = expectedPaths[i];
        bool expectedPathFound = false;

        for(int j = 0; j < pathCount; j++) {
            Path p = paths[j];
            ASSERT_EQ(Path_len(p), 2);
            Edge *e0 = p[0];
            Edge *e1 = p[1];
            if(Edge_GetSrcNodeID(e0) != expectedPath[0]) continue;
            if(Edge_GetDestNodeID(e0) != expectedPath[1]) continue;
            if(Edge_GetSrcNodeID(e1) != expectedPath[1]) continue;
            if(Edge_GetDestNodeID(e1) != expectedPath[2]) continue;
            expectedPathFound = true;
            break;
        }
        EXPECT_TRUE(expectedPathFound);
    }

    Graph_Free(g);
}
