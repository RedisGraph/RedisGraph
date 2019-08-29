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

  public:
	static QGNode *A;
	static QGNode *B;
	static QGNode *C;
	static QGNode *D;
	static QGEdge *AB;
	static QGEdge *BC;
	static QGEdge *CD;

  protected:
	static void SetUpTestCase() {
		// Use the malloc family for allocations
		Alloc_Reset();
	}

	static QueryGraph *BuildGraph() {
		// (A)->(B)
		// (B)->(C)
		// (C)->(D)
		size_t node_cap = 4;
		size_t edge_cap = 3;

		// Create nodes.
		const char *label = "L";
		const char *relation = "R";

		uint id = 0;
		A = QGNode_New(label, "A", id++);
		B = QGNode_New(label, "B", id++);
		C = QGNode_New(label, "C", id++);
		D = QGNode_New(label, "D", id++);

		AB = QGEdge_New(A, B, relation, "AB", id++);
		BC = QGEdge_New(B, C, relation, "BC", id++);
		CD = QGEdge_New(C, D, relation, "CD", id++);

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
	QGNode *S;        // DFS starts here.
	QGEdge **path;    // Path reached by DFS.
	QueryGraph *g;  // Graph traversed.

	g = BuildGraph();
	S = QueryGraph_GetNodeByID(g, A->id);

	QGEdge *expected_level_0[0] = {};
	QGEdge *expected_level_1[1] = {AB};
	QGEdge *expected_level_2[2] = {AB, BC};
	QGEdge *expected_level_3[3] = {AB, BC, CD};
	QGEdge *expected_level_4[0] = {};

	QGEdge **expected[5] = {
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
		QGEdge **expectation = expected[level];

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

// Static function declarations
QGNode *DFSTest::A;
QGNode *DFSTest::B;
QGNode *DFSTest::C;
QGNode *DFSTest::D;

QGEdge *DFSTest::AB;
QGEdge *DFSTest::BC;
QGEdge *DFSTest::CD;
