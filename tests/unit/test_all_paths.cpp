/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../../src/util/rmalloc.h"
#include "../../src/algorithms/algorithms.h"

#ifdef __cplusplus
}
#endif

class AllPathsTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {
		// Use the malloc family for allocations
		Alloc_Reset();

		// Initialize GraphBLAS.
		GrB_init(GrB_NONBLOCKING);
		GxB_Global_Option_set(GxB_FORMAT, GxB_BY_ROW); // all matrices in CSR format
		GxB_Global_Option_set(GxB_HYPER, GxB_NEVER_HYPER); // matrices are never hypersparse
	}

	static void TearDownTestCase() {
		GrB_finalize();
	}

	static Graph *BuildGraph() {
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

	NodeID srcNodeID = 0;
	unsigned int minLen = 999;
	unsigned int maxLen = minLen + 1;

	Node src;
	Graph_GetNode(g, srcNodeID, &src);

	int relationships[] = { GRAPH_NO_RELATION };
	AllPathsCtx *ctx = AllPathsCtx_New(&src, g, relationships, 1, GRAPH_EDGE_DIR_OUTGOING, minLen,
									   maxLen);
	Path p = AllPathsCtx_NextPath(ctx);

	ASSERT_TRUE(p == NULL);

	AllPathsCtx_Free(ctx);
	Graph_Free(g);
}

TEST_F(AllPathsTest, LongestPaths) {
	Graph *g = BuildGraph();

	NodeID srcNodeID = 0;
	Node src;
	Graph_GetNode(g, srcNodeID, &src);

	unsigned int minLen = 0;
	unsigned int maxLen = UINT_MAX - 2;
	int relationships[] = { GRAPH_NO_RELATION };
	AllPathsCtx *ctx = AllPathsCtx_New(&src, g, relationships, 1, GRAPH_EDGE_DIR_OUTGOING, minLen,
									   maxLen);
	Path path;

	unsigned int longestPath = 0;
	while((path = AllPathsCtx_NextPath(ctx))) {
		size_t pathLen = Path_len(path);
		if(longestPath < pathLen) longestPath = pathLen;
	}

	// 0,1,2,3,0
	ASSERT_EQ(longestPath, 5);

	AllPathsCtx_Free(ctx);
	Graph_Free(g);
}

TEST_F(AllPathsTest, UpToThreeLegsPaths) {
	Graph *g = BuildGraph();

	NodeID srcNodeID = 0;
	Node src;
	Graph_GetNode(g, srcNodeID, &src);

	Node *path = NULL;
	unsigned int minLen = 0;
	unsigned int maxLen = 3;
	uint pathsCount = 0;
	int relationships[] = { GRAPH_NO_RELATION };
	AllPathsCtx *ctx = AllPathsCtx_New(&src, g, relationships, 1, GRAPH_EDGE_DIR_OUTGOING, minLen,
									   maxLen);

	/* Connections:
	 * 0 -> 1
	 * 0 -> 2
	 * 1 -> 0
	 * 1 -> 2
	 * 2 -> 1
	 * 2 -> 3
	 * 3 -> 0 */

	// Zero leg paths.
	NodeID p0[3] = {1, 0};

	// One leg paths.
	NodeID p1[3] = {2, 0, 1};
	NodeID p2[3] = {2, 0, 2};

	// Two leg paths.
	NodeID p3[4] = {3, 0, 1, 0};
	NodeID p4[4] = {3, 0, 1, 2};
	NodeID p5[4] = {3, 0, 2, 1};
	NodeID p6[4] = {3, 0, 2, 3};

	// Three leg paths.
	NodeID p7[5] = {4, 0, 1, 2, 1};
	NodeID p8[5] = {4, 0, 1, 2, 3};
	NodeID p9[5] = {4, 0, 2, 1, 0};
	NodeID p10[5] = {4, 0, 2, 1, 2};
	NodeID p11[5] = {4, 0, 2, 3, 0};

	NodeID *expectedPaths[12] = {p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11};

	while((path = AllPathsCtx_NextPath(ctx))) {
		bool expectedPathFound = false;
		assert(pathsCount < 12);

		// Try to match a path.
		for(int i = 0; i < 12; i++) {
			NodeID *expectedPath = expectedPaths[i];
			size_t expectedPathLen = expectedPath[0];
			expectedPath++; // Skip path length.

			if(Path_len(path) != expectedPathLen) continue;

			int j = 0;
			for(; j < expectedPathLen; j++) {
				Node n = path[j];
				if(ENTITY_GET_ID(&n) != expectedPath[j]) break;
			}
			if(j == expectedPathLen) {
				expectedPathFound = true;
				break;
			}
		}
		ASSERT_TRUE(expectedPathFound);
		pathsCount++;
	}
	ASSERT_EQ(pathsCount, 12);

	AllPathsCtx_Free(ctx);
	Graph_Free(g);
}

TEST_F(AllPathsTest, TwoLegPaths) {
	Graph *g = BuildGraph();

	NodeID srcNodeID = 0;
	Node src;
	Node *path = NULL;
	Graph_GetNode(g, srcNodeID, &src);
	unsigned int minLen = 2;
	unsigned int maxLen = 2;
	unsigned int pathsCount = 0;
	int relationships[] = { GRAPH_NO_RELATION };
	AllPathsCtx *ctx = AllPathsCtx_New(&src, g, relationships, 1, GRAPH_EDGE_DIR_OUTGOING, minLen,
									   maxLen);
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

	while((path = AllPathsCtx_NextPath(ctx))) {
		ASSERT_LT(pathsCount, 4);
		ASSERT_EQ(Path_len(path), 3);
		bool expectedPathFound = false;

		for(int i = 0; i < 4; i++) {
			NodeID *expectedPath = expectedPaths[i];
			int j;
			for(j = 0; j < 3; j++) {
				Node n = path[j];
				if(ENTITY_GET_ID(&n) != expectedPath[j]) break;
			}
			expectedPathFound = (j == 3);
			if(expectedPathFound) break;
		}

		ASSERT_TRUE(expectedPathFound);
		pathsCount++;
	}

	ASSERT_EQ(pathsCount, 4);

	AllPathsCtx_Free(ctx);
	Graph_Free(g);
}
