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
#include "../../src/util/rmalloc.h"

#ifdef __cplusplus
}
#endif

class AllPathsTest: public ::testing::Test {
    protected:
    static void SetUpTestCase()
    {
        // Initialize GraphBLAS.
        GrB_init(GrB_NONBLOCKING);

        // Use the malloc family for allocations
        Alloc_Reset();
    }

    static void TearDownTestCase()
    {
        GrB_finalize();
    }

    static Graph* BuildGraph()
    {
        Edge e;
        Node n;
        size_t nodeCount = 4;
        Graph *g = Graph_New(nodeCount, nodeCount);
        int relation = Graph_AddRelationType(g);
        for(int i = 0; i < 4; i++) Graph_CreateNode(g, GRAPH_NO_LABEL, &n);

        /* Connections:
         * 0 -> 1
         * 0 -> 2
         * 1 -> 0
         * 1 -> 2
         * 2 -> 1
         * 2 -> 3
         * 3 -> 0 */

        // Connections:
        // 0 -> 1
        Graph_ConnectNodes(g, 0, 1, relation, &e);
        // 0 -> 2
        Graph_ConnectNodes(g, 0, 2, relation, &e);
        // 1 -> 0
        Graph_ConnectNodes(g, 1, 0, relation, &e);
        // 1 -> 2
        Graph_ConnectNodes(g, 1, 2, relation, &e);
        // 2 -> 1
        Graph_ConnectNodes(g, 2, 1, relation, &e);
        // 2 -> 3
        Graph_ConnectNodes(g, 2, 3, relation, &e);
        // 3 -> 0
        Graph_ConnectNodes(g, 3, 0, relation, &e);
        return g;
    }
};

TEST_F(AllPathsTest, NoPaths) {
    Graph *g = BuildGraph();

    NodeID src = 0;
    unsigned int minLen = 999;
    unsigned int maxLen = minLen + 1;
    size_t pathsCap = 1;
    Path *paths = (Path*)malloc(sizeof(Path) * pathsCap);
    size_t pathsCount = AllPaths(g, GRAPH_NO_RELATION, src, GRAPH_EDGE_DIR_OUTGOING, minLen, maxLen, &pathsCap, &paths);
    ASSERT_EQ(pathsCount, 0);
    
    free(paths);
    Graph_Free(g);
}

TEST_F(AllPathsTest, LongestPaths) {
    Graph *g = BuildGraph();

    NodeID src = 0;
    unsigned int minLen = 0;
    unsigned int maxLen = UINT_MAX;
    size_t pathsCap = 1;
    Path *paths = (Path*)malloc(sizeof(Path) * pathsCap);
    size_t pathsCount = AllPaths(g, GRAPH_NO_RELATION, src, GRAPH_EDGE_DIR_OUTGOING, minLen, maxLen, &pathsCap, &paths);

    unsigned int longestPath = 0;
    for(int i = 0; i < pathsCount; i++) {
        size_t pathLen = Path_len(paths[i]);
        if(longestPath < pathLen) longestPath = pathLen;
    }

    // 0,2,3,0,1,2,1,0
    ASSERT_EQ(longestPath, 7);

    for(int i = 0; i < pathsCount; i++) Path_free(paths[i]);
    free(paths);
    Graph_Free(g);
}

TEST_F(AllPathsTest, UpToThreeLegsPaths) {
    Graph *g = BuildGraph();

    NodeID src = 0;
    unsigned int minLen = 1;
    unsigned int maxLen = 3;
    size_t pathsCap = 2;
    Path *paths = (Path*)malloc(sizeof(Path) * pathsCap);
    size_t pathsCount = AllPaths(g, GRAPH_NO_RELATION, src, GRAPH_EDGE_DIR_OUTGOING, minLen, maxLen, &pathsCap, &paths);
    ASSERT_EQ(pathsCount, 12);

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
    

    for(int i = 0; i < pathsCount; i++) {
        NodeID *expectedPath = expectedPaths[i];
        size_t expectedPathLen = expectedPath[0];
        expectedPath++; // Skip path length.
        bool expectedPathFound = false;

        for(int j = 0; j < pathsCount; j++) {
            Path p = paths[j];
            if(Path_len(p) != expectedPathLen) continue;
            
            int k = 0;
            for(; k < expectedPathLen; k++) {
                Edge e = p[k];
                if(Edge_GetSrcNodeID(&e) != expectedPath[k]) break;
                if(Edge_GetDestNodeID(&e) != expectedPath[k+1]) break;
            }
            if(k == expectedPathLen) {
                expectedPathFound = true;
                break;
            }
        }
        ASSERT_TRUE(expectedPathFound);
    }

    for(int i = 0; i < pathsCount; i++) Path_free(paths[i]);
    
    free(paths);
    Graph_Free(g);
}

TEST_F(AllPathsTest, TwoLegPaths) {
    Graph *g = BuildGraph();

    NodeID src = 0;
    unsigned int minLen = 2;
    unsigned int maxLen = 2;
    size_t pathsCap = 1;
    Path *paths = (Path*)malloc(sizeof(Path) * pathsCap);
    size_t pathsCount = AllPaths(g, GRAPH_NO_RELATION, src, GRAPH_EDGE_DIR_OUTGOING, minLen, maxLen, &pathsCap, &paths);
    ASSERT_EQ(pathsCount, 4);

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

    
    for(int i = 0; i < pathsCount; i++) {
        NodeID *expectedPath = expectedPaths[i];
        bool expectedPathFound = false;

        for(int j = 0; j < pathsCount; j++) {
            Path p = paths[j];
            ASSERT_EQ(Path_len(p), 2);
            Edge e0 = p[0];
            Edge e1 = p[1];
            if(Edge_GetSrcNodeID(&e0) != expectedPath[0]) continue;
            if(Edge_GetDestNodeID(&e0) != expectedPath[1]) continue;
            if(Edge_GetSrcNodeID(&e1) != expectedPath[1]) continue;
            if(Edge_GetDestNodeID(&e1) != expectedPath[2]) continue;
            expectedPathFound = true;
            break;
        }
        ASSERT_TRUE(expectedPathFound);
    }

    for(int i = 0; i < pathsCount; i++) Path_free(paths[i]);
    free(paths);
    Graph_Free(g);
}
