/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../../src/util/rmalloc.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"
#include "../../src/graph/query_graph.h"
#include "../../src/algorithms/algorithms.h"

#ifdef __cplusplus
}
#endif

class DetectCycleTest: public ::testing::Test {

  protected:
	static void SetUpTestCase() {
        // Initialize GraphBLAS
        GrB_init(GrB_NONBLOCKING);

		// Use the malloc family for allocations
		Alloc_Reset();
	}

	static QueryGraph *AcyclicBuildGraph() {
		// (A)->(B)->(C)
		size_t node_cap = 3;
		size_t edge_cap = 2;

		// Create nodes.
		const char *label = "L";
		const char *relation = "R";

		QGNode *A = QGNode_New(label, "A");
		QGNode *B = QGNode_New(label, "B");
		QGNode *C = QGNode_New(label, "C");

		QGEdge *AB = QGEdge_New(A, B, relation, "AB");
		QGEdge *BC = QGEdge_New(B, C, relation, "BC");

		QueryGraph *g = QueryGraph_New(node_cap, edge_cap);
		QueryGraph_AddNode(g, A);
		QueryGraph_AddNode(g, B);
		QueryGraph_AddNode(g, C);

		QueryGraph_ConnectNodes(g, A, B, AB);
		QueryGraph_ConnectNodes(g, B, C, BC);

		return g;
	}

    static QueryGraph *CyclicBuildGraph() {
		// (A)->(B)->(C)->(D)->(E)->(B)->(F)->(G)->(C)
		size_t node_cap = 7;
		size_t edge_cap = 8;

		// Create nodes.
		const char *label = "L";
		const char *relation = "R";

		QGNode *A = QGNode_New(label, "A");
		QGNode *B = QGNode_New(label, "B");
		QGNode *C = QGNode_New(label, "C");
		QGNode *D = QGNode_New(label, "D");
		QGNode *E = QGNode_New(label, "E");
		QGNode *F = QGNode_New(label, "F");
		QGNode *G = QGNode_New(label, "G");

        // (A)->(B)
		QGEdge *AB = QGEdge_New(A, B, relation, "AB");
        // (B)->(C)
		QGEdge *BC = QGEdge_New(B, C, relation, "BC");
        // (C)->(D)
		QGEdge *CD = QGEdge_New(C, D, relation, "CD");
        // (D)->(E)
		QGEdge *DE = QGEdge_New(D, E, relation, "DE");
        // (E)->(B)
		QGEdge *EB = QGEdge_New(E, B, relation, "EB");
        // (B)->(F)
		QGEdge *BF = QGEdge_New(B, F, relation, "BF");
        // (F)->(G)
		QGEdge *FG = QGEdge_New(F, G, relation, "FG");
        // (G)->(C)
		QGEdge *GC = QGEdge_New(G, C, relation, "GC");

		QueryGraph *g = QueryGraph_New(node_cap, edge_cap);
		QueryGraph_AddNode(g, A);
		QueryGraph_AddNode(g, B);
		QueryGraph_AddNode(g, C);
		QueryGraph_AddNode(g, D);
		QueryGraph_AddNode(g, E);
		QueryGraph_AddNode(g, F);
		QueryGraph_AddNode(g, G);

		QueryGraph_ConnectNodes(g, A, B, AB);
        QueryGraph_ConnectNodes(g, B, C, BC);
        QueryGraph_ConnectNodes(g, C, D, CD);
        QueryGraph_ConnectNodes(g, D, E, DE);
        QueryGraph_ConnectNodes(g, E, B, EB);
        QueryGraph_ConnectNodes(g, B, F, BF);
        QueryGraph_ConnectNodes(g, F, G, FG);
        QueryGraph_ConnectNodes(g, G, C, GC);

		return g;
	}
};

TEST_F(DetectCycleTest, AcyclicGraph) {
    QueryGraph *g = AcyclicBuildGraph();  // Graph traversed.
    ASSERT_TRUE(IsAcyclicGraph(g));
	// Clean up.
	QueryGraph_Free(g);
}

TEST_F(DetectCycleTest, CyclicGraph) {
    QueryGraph *g = CyclicBuildGraph();  // Graph traversed.
    ASSERT_FALSE(IsAcyclicGraph(g));
	// Clean up.
	QueryGraph_Free(g);
}
