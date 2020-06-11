/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../src/config.h"
#include "../../src/util/arr.h"
#include "../../src/graph/graph.h"
#include "../../src/util/rmalloc.h"
#include "../../src/util/simple_timer.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"
#include "../../src/util/datablock/datablock_iterator.h"

#ifdef __cplusplus
}
#endif

// Console text colors for benchmark printing
#define KGRN "\x1B[32m"
#define KRED "\x1B[31m"
#define KNRM "\x1B[0m"

RG_Config config; // Global module configuration

// Encapsulate the essence of an edge.
typedef struct {
	NodeID srcId;   	// Source node ID.
	NodeID destId;  	// Destination node ID.
	int64_t relationId; // Relation type ID.
} EdgeDesc;

class GraphTest : public ::testing::Test {
  protected:
	static void SetUpTestCase() {
		// Use the malloc family for allocations
		Alloc_Reset();

		// Set global variables
		config.maintain_transposed_matrices = true; // Ensure that transposed matrices are constructed.

		// Initialize GraphBLAS.
		GrB_init(GrB_NONBLOCKING);

		GxB_Global_Option_set(GxB_FORMAT, GxB_BY_ROW); // all matrices in CSR format
		GxB_Global_Option_set(GxB_HYPER, GxB_NEVER_HYPER); // matrices are never hypersparse
		srand(time(NULL));
	}

	static void TearDownTestCase() {
		GrB_finalize();
	}

	void _test_node_creation(Graph *g, size_t node_count) {
		GrB_Index ncols, nrows, nvals;

		// Create nodes.
		Node n;
		for(uint i = 0; i < node_count; i++) Graph_CreateNode(g, GRAPH_NO_LABEL, &n);

		// Validate nodes creation.
		GrB_Matrix adj = Graph_GetAdjacencyMatrix(g);
		ASSERT_EQ(GrB_Matrix_nrows(&nrows, adj), GrB_SUCCESS);
		ASSERT_EQ(GrB_Matrix_ncols(&ncols, adj), GrB_SUCCESS);
		ASSERT_EQ(GrB_Matrix_nvals(&nvals, adj), GrB_SUCCESS);

		ASSERT_EQ(nvals, 0);                  // No connection were formed.
		ASSERT_GE(ncols, Graph_NodeCount(g)); // Graph's adjacency matrix dimensions.
		ASSERT_GE(nrows, Graph_NodeCount(g));
		ASSERT_EQ(Graph_NodeCount(g), node_count);
	}

	void _test_edge_creation(Graph *g, size_t node_count) {
		// Form connections.
		int relationCount = 3;
		size_t edge_count = (node_count - 1);
		EdgeDesc connections[edge_count];

		// Introduce relations types.
		for(int i = 0; i < relationCount; i++)
			Graph_AddRelationType(g);

		// Describe connections;
		// Node I is connected to Node I+1,
		// Connection type is relationships[I%4].
		Edge e;
		for(unsigned int i = 0; i < edge_count; i++) {
			connections[i].srcId = i;                           // Source node id.
			connections[i].destId = i + 1;                      // Destination node id.
			connections[i].relationId = (i % relationCount);    // Relation.
			Graph_ConnectNodes(g, connections[i].srcId, connections[i].destId, connections[i].relationId, &e);
		}

		// Validate edges creation,
		// expecting number of none zero entries to be edge_count.
		GrB_Index nvals;
		GrB_Matrix adj = Graph_GetAdjacencyMatrix(g);
		ASSERT_EQ(GrB_Matrix_nvals(&nvals, adj), GrB_SUCCESS);
		ASSERT_EQ(nvals, edge_count);

		// Inspect graph matrices;
		// Graph's adjacency matrix should include all connections,
		// relation matrices should include edges of a certain relation.
		for(unsigned int i = 0; i < edge_count; i++) {
			int src_id = connections[i].srcId;
			int dest_id = connections[i].destId;
			int r = connections[i].relationId;
			bool v = false;

			// src_id connected to dest_id.
			ASSERT_EQ(GrB_Matrix_extractElement_BOOL(&v, adj, dest_id, src_id), GrB_SUCCESS);
			ASSERT_TRUE(v);

			// Test relation matrix.
			v = false;
			GrB_Matrix mat = Graph_GetRelationMatrix(g, r);
			ASSERT_EQ(GrB_Matrix_extractElement_BOOL(&v, mat, dest_id, src_id), GrB_SUCCESS);
			ASSERT_TRUE(v);
		}
	}

	void _test_graph_resize(Graph *g) {
		GrB_Index ncols, nrows;
		size_t prev_node_count = Graph_NodeCount(g);
		size_t additional_node_count = prev_node_count * 16;

		Graph_AllocateNodes(g, prev_node_count + additional_node_count);
		for(uint i = 0; i < additional_node_count; i++) {
			Node n;
			Graph_CreateNode(g, 0, &n);
		}

		// Validate nodes creation.
		ASSERT_EQ(Graph_NodeCount(g), prev_node_count + additional_node_count);
		// Graph's adjacency matrix dimensions.
		GrB_Matrix adj = Graph_GetAdjacencyMatrix(g);
		ASSERT_EQ(GrB_Matrix_nrows(&nrows, adj), GrB_SUCCESS);
		ASSERT_EQ(GrB_Matrix_ncols(&ncols, adj), GrB_SUCCESS);
		ASSERT_GE(ncols, Graph_NodeCount(g));
		ASSERT_GE(nrows, Graph_NodeCount(g));

		// Verify number of created nodes.
		Node *n;
		unsigned int new_node_count = 0;
		DataBlockIterator *it = Graph_ScanNodes(g);
		while((n = (Node *)DataBlockIterator_Next(it)) != NULL) new_node_count++;
		ASSERT_EQ(new_node_count, prev_node_count + additional_node_count);
		DataBlockIterator_Free(it);

		// Relation matrices get resize lazily,
		// Try to fetch one of the specific relation matrices and verify its dimenstions.
		uint relation_count = Graph_RelationTypeCount(g);
		ASSERT_GT(relation_count, 0);
		for(unsigned int i = 0; i < relation_count; i++) {
			GrB_Matrix r = Graph_GetRelationMatrix(g, i);
			ASSERT_EQ(GrB_Matrix_nrows(&nrows, r), GrB_SUCCESS);
			ASSERT_EQ(GrB_Matrix_ncols(&ncols, r), GrB_SUCCESS);
			ASSERT_GE(ncols, Graph_NodeCount(g));
			ASSERT_GE(nrows, Graph_NodeCount(g));
		}
	}
};

/* TODO benchmark functions are currently not invoked */
void benchmark_node_creation_with_labels() {
	printf("benchmark_node_creation_with_labels\n");
	double tic[2];
	int samples = 64;
	int label_count = 3;
	double timings[samples];
	int outliers = 0;
	float threshold = 0.0018;
	// size_t n = GRAPH_DEFAULT_NODE_CAP;
	size_t n = 1000000;
	Graph *g = Graph_New(n, n);
	Graph_AcquireWriteLock(g);

	// Introduce labels and relations to graph.
	for(int i = 0; i < label_count; i++) {
		Graph_AddRelationType(g); // Typed relation.
		Graph_AddLabel(g);    // Typed node.
	}

	Node node;

	int labels[n];
	// Create N nodes with labels.
	for(int i = 0; i < samples; i++) {
		// Associate nodes to labels.
		for(unsigned int j = 0; j < n; j++) labels[j] = (rand() % label_count) - 1;

		simple_tic(tic);
		for(unsigned int j = 0; j < n; j++) Graph_CreateNode(g, labels[j], &node);
		timings[i] = simple_toc(tic);

		printf("%zu Nodes created, time: %.6f sec\n", n, timings[i]);
		if(timings[i] > threshold)
			outliers++;
	}

	if(outliers > samples * 0.1) {
		printf("Node creation took too long\n");
		for(int i = 0; i < samples; i++) {
			printf("%zu Nodes created, time: %.6f sec\n", n, timings[i]);
		}
		// assert(false);
	}

	Graph_ReleaseLock(g);
	Graph_Free(g);
}

// Test graph creation time.
void benchmark_node_creation_no_labels() {
	printf("benchmark_node_creation_no_labels\n");
	double tic[2];
	int samples = 64;
	double timings[samples];
	int outliers = 0;
	float threshold = 0.000006;
	// size_t n = GRAPH_DEFAULT_NODE_CAP;
	Node node;
	size_t n = 1000000;
	Graph *g = Graph_New(n, n);
	Graph_AcquireWriteLock(g);

	for(int i = 0; i < samples; i++) {
		// Create N nodes, don't use labels.
		simple_tic(tic);
		for(int j = 0; j < n; j++) Graph_CreateNode(g, GRAPH_NO_LABEL, &node);
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

	Graph_ReleaseLock(g);
	Graph_Free(g);
}

void benchmark_edge_creation_with_relationships() {
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
	Node node;
	Edge edge;
	Graph *g = Graph_New(GRAPH_DEFAULT_NODE_CAP, GRAPH_DEFAULT_EDGE_CAP);
	Graph_AcquireWriteLock(g);

	// Introduce relations types.
	for(int i = 0; i < relation_count; i++) Graph_AddRelationType(g);
	for(int i = 0; i < node_count; i++) Graph_CreateNode(g, GRAPH_NO_LABEL, &node);
	for(int i = 0; i < samples; i++) {
		// Describe connections;
		// Node I is connected to Node I+1,
		// Connection type is relationships[I%4].
		for(int j = 0; j < edge_count; j++) {
			connections[j].srcId = rand() % node_count;     // Source node id.
			connections[j].destId = rand() % node_count;    // Destination node id.
			connections[j].relationId = i % relation_count; // Relation.
		}

		simple_tic(tic);
		for(int j = 0; j < edge_count; j++) {
			Graph_ConnectNodes(g, connections[j].srcId, connections[j].destId, connections[j].relationId,
							   &edge);
		}
		timings[i] = simple_toc(tic);
		printf("%d Formed connections, time: %.6f sec\n", edge_count, timings[i]);
		if(timings[i] > threshold)
			outliers++;
	}

	if(outliers > samples * 0.1) {
		printf("Node creation took too long\n");
		for(int i = 0; i < samples; i++) {
			printf("%d Formed connections, time: %.6f sec\n", edge_count, timings[i]);
		}
		// assert(false);
	}

	Graph_ReleaseLock(g);
	Graph_Free(g);
}

void benchmark_graph() {
	benchmark_node_creation_no_labels();
	benchmark_node_creation_with_labels();
	benchmark_edge_creation_with_relationships();
	printf("%sgraph benchmark - PASS!%s\n", KGRN, KNRM);
}

// Validate the creation of a graph,
// Make sure graph's defaults are applied.
TEST_F(GraphTest, NewGraph) {
	GrB_Index ncols, nrows, nvals;
	Graph *g = Graph_New(GRAPH_DEFAULT_NODE_CAP, GRAPH_DEFAULT_EDGE_CAP);
	Graph_AcquireWriteLock(g);
	GrB_Matrix adj_matrix = g->adjacency_matrix->grb_matrix;
	ASSERT_EQ(GrB_Matrix_ncols(&ncols, adj_matrix), GrB_SUCCESS);
	ASSERT_EQ(GrB_Matrix_nrows(&nrows, adj_matrix), GrB_SUCCESS);
	ASSERT_EQ(GrB_Matrix_nvals(&nvals, adj_matrix), GrB_SUCCESS);

	ASSERT_TRUE(g->nodes != NULL);
	ASSERT_TRUE(g->relations != NULL);
	ASSERT_TRUE(g->labels != NULL);
	ASSERT_TRUE(g->adjacency_matrix != NULL);
	ASSERT_EQ(Graph_NodeCount(g), 0);
	ASSERT_EQ(nrows, GRAPH_DEFAULT_NODE_CAP);
	ASSERT_EQ(ncols, GRAPH_DEFAULT_NODE_CAP);
	ASSERT_EQ(nvals, 0);

	Graph_ReleaseLock(g);
	Graph_Free(g);
}

// Tests node and edge creation.
TEST_F(GraphTest, GraphConstruction) {
	size_t node_count = GRAPH_DEFAULT_NODE_CAP / 2;
	Graph *g = Graph_New(node_count, node_count);
	Graph_AcquireWriteLock(g);
	_test_node_creation(g, node_count);
	// _test_edge_creation(g, node_count);

	// Introduce additional nodes which will cause graph to resize.
	// _test_graph_resize(g);

	Graph_ReleaseLock(g);
	Graph_Free(g);
}

TEST_F(GraphTest, RemoveNodes) {
	// Construct graph.
	GrB_Matrix M;
	GrB_Index nnz;
	Graph *g = Graph_New(32, 32);
	Graph_AcquireWriteLock(g);

	// Create 3 nodes.
	Node node;
	Edge edge;

	for(int i = 0; i < 3; i++) Graph_CreateNode(g, GRAPH_NO_LABEL, &node);
	int r = Graph_AddRelationType(g);

	/* Connections:
	 * 0 connected to 1.
	 * 1 connected to 0.
	 * 1 connected to 2. */

	// connect 0 to 1.
	Graph_ConnectNodes(g, 0, 1, r, &edge);

	// Connect 1 to 0.
	Graph_ConnectNodes(g, 1, 0, r, &edge);

	// Connect 1 to 2.
	Graph_ConnectNodes(g, 1, 2, r, &edge);

	// Validate graph creation.
	ASSERT_EQ(Graph_NodeCount(g), 3);
	M = Graph_GetRelationMatrix(g, r);
	GrB_Matrix_nvals(&nnz, M);
	ASSERT_EQ(nnz, 3);

	M = Graph_GetAdjacencyMatrix(g);
	GrB_Matrix_nvals(&nnz, M);
	ASSERT_EQ(nnz, 3);

	Edge *edges = (Edge *)array_new(Edge, 3);
	//==============================================================================
	//=== Delete node 0 ============================================================
	//==============================================================================

	// First node should have 2 edges.
	Graph_GetNode(g, 0, &node);
	Graph_GetNodeEdges(g, &node, GRAPH_EDGE_DIR_BOTH, GRAPH_NO_RELATION, &edges);
	uint edge_count = array_len(edges);
	ASSERT_EQ(edge_count, 2);

	// Delete the edges on the first node manually, as Graph_DeleteNode expects
	// it to be detached.
	// Both 0 -> 1 edge and 1 -> 0 edge should be removed.
	for(uint i = 0; i < edge_count; i ++) Graph_DeleteEdge(g, &edges[i]);

	array_free(edges);

	Graph_GetNode(g, 0, &node);
	Graph_DeleteNode(g, &node);

	ASSERT_EQ(Graph_NodeCount(g), 2);
	ASSERT_EQ(Graph_EdgeCount(g), 1);
}

TEST_F(GraphTest, RemoveMultipleNodes) {
	// Delete two node.
	// One which is the latest node introduced to the graph
	// and the very first node.

	// Expecting ...
	double tic[2];

	Node n;
	Edge e;
	Graph *g = Graph_New(32, 32);
	Graph_AcquireWriteLock(g);
	int relation = Graph_AddRelationType(g);
	for(int i = 0; i < 8; i++) Graph_CreateNode(g, GRAPH_NO_LABEL, &n);

	// First node.
	Graph_ConnectNodes(g, 0, 2, relation, &e);
	Graph_ConnectNodes(g, 0, 6, relation, &e);
	Graph_ConnectNodes(g, 0, 7, relation, &e);

	// Right before last node.
	Graph_ConnectNodes(g, 6, 0, relation, &e);
	Graph_ConnectNodes(g, 6, 1, relation, &e);
	Graph_ConnectNodes(g, 6, 7, relation, &e);

	// Last node.
	Graph_ConnectNodes(g, 7, 0, relation, &e);
	Graph_ConnectNodes(g, 7, 1, relation, &e);
	Graph_ConnectNodes(g, 7, 6, relation, &e);

	// Delete first and last nodes.
	simple_tic(tic);
	Graph_GetNode(g, 0, &n);
	Graph_DeleteNode(g, &n);
	Graph_GetNode(g, 7, &n);
	Graph_DeleteNode(g, &n);

	ASSERT_EQ(Graph_NodeCount(g), 8 - 2);

	GrB_Descriptor desc;
	GrB_Descriptor_new(&desc);
	GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);

	// Validation.
	GrB_Vector row;
	GrB_Vector col;
	GrB_Index rowNvals;
	GrB_Index colNvals;

	GrB_Vector_new(&row, GrB_BOOL, Graph_NodeCount(g));
	GrB_Vector_new(&col, GrB_BOOL, Graph_NodeCount(g));

	// Make sure last and first rows and column are zeroed out.
	GrB_Matrix adj = Graph_GetAdjacencyMatrix(g);

	GrB_Col_extract(row, NULL, NULL, adj, GrB_ALL, Graph_NodeCount(g), 7, desc);
	GrB_Col_extract(col, NULL, NULL, adj, GrB_ALL, Graph_NodeCount(g), 7, NULL);
	GrB_Vector_nvals(&rowNvals, row);
	GrB_Vector_nvals(&colNvals, col);
	ASSERT_EQ(rowNvals, 0);
	ASSERT_EQ(colNvals, 0);

	GrB_Col_extract(row, NULL, NULL, adj, GrB_ALL, Graph_NodeCount(g), 0, desc);
	GrB_Col_extract(col, NULL, NULL, adj, GrB_ALL, Graph_NodeCount(g), 0, NULL);
	GrB_Vector_nvals(&rowNvals, row);
	GrB_Vector_nvals(&colNvals, col);
	ASSERT_EQ(rowNvals, 0);
	ASSERT_EQ(colNvals, 0);

	// Clean up.
	GrB_Descriptor_free(&desc);
	GrB_Vector_free(&row);
	GrB_Vector_free(&col);

	Graph_ReleaseLock(g);
	Graph_Free(g);
}

TEST_F(GraphTest, RemoveEdges) {
	// Construct graph.
	Edge e;
	Node n;
	GrB_Matrix M;
	GrB_Index nnz;
	Graph *g = Graph_New(32, 32);
	Graph_AcquireWriteLock(g);
	for(int i = 0; i < 3; i++) Graph_CreateNode(g, GRAPH_NO_LABEL, &n);
	int r = Graph_AddRelationType(g);

	/* Connections:
	 * 0 connected to 1.
	 * 0 connected to 2.
	 * 1 connected to 2. */

	// connect 0 to 1.
	Graph_ConnectNodes(g, 0, 1, r, &e);
	// Connect 0 to 2.
	Graph_ConnectNodes(g, 0, 2, r, &e);
	// Connect 1 to 2.
	Graph_ConnectNodes(g, 1, 2, r, &e);

	// Validate graph creation.
	ASSERT_EQ(Graph_EdgeCount(g), 3);

	M = Graph_GetRelationMatrix(g, r);
	GrB_Matrix_nvals(&nnz, M);
	ASSERT_EQ(nnz, 3);

	M = Graph_GetAdjacencyMatrix(g);
	GrB_Matrix_nvals(&nnz, M);
	ASSERT_EQ(nnz, 3);

	//==============================================================================
	//=== Delete first edge ========================================================
	//==============================================================================
	Edge *edges = (Edge *)array_new(Edge, 3);
	Graph_GetEdgesConnectingNodes(g, 0, 1, GRAPH_NO_RELATION, &edges);
	ASSERT_EQ(array_len(edges), 1);

	e = edges[0];
	NodeID srcID = Edge_GetSrcNodeID(&e);
	NodeID destID = Edge_GetDestNodeID(&e);

	// Delete edge.
	Graph_DeleteEdge(g, &e);

	// Validate edge deletion.
	ASSERT_EQ(Graph_EdgeCount(g), 2);
	array_clear(edges);
	Graph_GetEdgesConnectingNodes(g, 0, 1, GRAPH_NO_RELATION, &edges);
	ASSERT_EQ(array_len(edges), 0);

	// Validate matrix update.
	M = Graph_GetRelationMatrix(g, r);
	// Relation matrix at [destID, srcID] should be empty.
	bool x = false;
	GrB_Matrix_extractElement_BOOL(&x, M, destID, srcID);
	ASSERT_FALSE(x);
	GrB_Matrix_nvals(&nnz, M);
	ASSERT_EQ(nnz, 2);

	// Adjacency matrix at [destID, srcID] should be empty.
	x = false;
	M = Graph_GetAdjacencyMatrix(g);
	GrB_Matrix_extractElement_BOOL(&x, M, destID, srcID);
	ASSERT_FALSE(x);
	GrB_Matrix_nvals(&nnz, M);
	ASSERT_EQ(nnz, 2);

	//==============================================================================
	//=== Delete second edge =======================================================
	//==============================================================================

	array_clear(edges);
	Graph_GetNode(g, 2, &n);
	Graph_GetNodeEdges(g, &n, GRAPH_EDGE_DIR_BOTH, GRAPH_NO_RELATION, &edges);

	ASSERT_EQ(array_len(edges), 2);

	// Delete edge.
	e = edges[0];
	srcID = Edge_GetSrcNodeID(&e);
	destID = Edge_GetDestNodeID(&e);
	Graph_DeleteEdge(g, &e);

	// Validate edge deletion.
	ASSERT_EQ(Graph_EdgeCount(g), 1);

	// Validate matrix update.
	M = Graph_GetRelationMatrix(g, r);
	// Relation matrix at [destID, srcID] should be empty.
	x = false;
	GrB_Matrix_extractElement_BOOL(&x, M, destID, srcID);
	ASSERT_FALSE(x);
	GrB_Matrix_nvals(&nnz, M);
	ASSERT_EQ(nnz, 1);

	// Adjacency matrix at [destID, srcID] should be empty.
	x = false;
	M = Graph_GetAdjacencyMatrix(g);
	GrB_Matrix_extractElement_BOOL(&x, M, destID, srcID);
	ASSERT_FALSE(x);
	GrB_Matrix_nvals(&nnz, M);
	ASSERT_EQ(nnz, 1);

	//==============================================================================
	//=== Delete third edge ========================================================
	//==============================================================================

	// Delete edge.
	e = edges[1];
	srcID = Edge_GetSrcNodeID(&e);
	destID = Edge_GetDestNodeID(&e);
	Graph_DeleteEdge(g, &e);

	// Validate edge deletion.
	ASSERT_EQ(Graph_EdgeCount(g), 0);

	// Validate matrix update.
	M = Graph_GetRelationMatrix(g, r);
	// Relation matrix at [destID, srcID] should be empty.
	x = false;
	GrB_Matrix_extractElement_BOOL(&x, M, destID, srcID);
	ASSERT_FALSE(x);
	GrB_Matrix_nvals(&nnz, M);
	ASSERT_EQ(nnz, 0);

	// Adjacency matrix at [destID, srcID] should be empty.
	x = false;
	M = Graph_GetAdjacencyMatrix(g);
	GrB_Matrix_extractElement_BOOL(&x, M, destID, srcID);
	ASSERT_FALSE(x);
	GrB_Matrix_nvals(&nnz, M);
	ASSERT_EQ(nnz, 0);

	// Cleanup.
	Graph_ReleaseLock(g);
	Graph_Free(g);
}

TEST_F(GraphTest, GetNode) {
	/* Create a graph with nodeCount nodes,
	 * Make sure node retrival works as expected:
	 * 1. try to get nodes (0 - nodeCount)
	 * 2. try to get node with ID >= nodeCount. */

	Node n;
	size_t nodeCount = 16;
	Graph *g = Graph_New(nodeCount, nodeCount);
	Graph_AcquireWriteLock(g);
	for(int i = 0 ; i < nodeCount; i++) Graph_CreateNode(g, GRAPH_NO_LABEL, &n);

	// Get nodes 0 - nodeCount.
	NodeID i = 0;
	for(; i < nodeCount; i++) {
		Graph_GetNode(g, i, &n);
		ASSERT_TRUE(n.entity != NULL);
	}

	// Get a none existing node.
	ASSERT_EQ(Graph_GetNode(g, i, &n), 0);
	ASSERT_TRUE(n.entity == NULL);

	Graph_ReleaseLock(g);
	Graph_Free(g);
}

TEST_F(GraphTest, GetEdge) {
	/* Create a graph with both nodes and edges.
	 * Make sure edge retrival works as expected:
	 * 1. try to get edges by ID
	 * 2. try to get edges connecting source to destination node
	 * 3. try to get none existing edges. */

	Node n;
	Edge e;
	int relationCount = 4;
	size_t nodeCount = 5;
	size_t edgeCount = 5;
	int relations[relationCount];

	Graph *g = Graph_New(nodeCount, nodeCount);
	Graph_AcquireWriteLock(g);
	for(int i = 0; i < nodeCount; i++) Graph_CreateNode(g, GRAPH_NO_LABEL, &n);
	for(int i = 0; i < relationCount; i++) relations[i] = Graph_AddRelationType(g);

	/* Connect nodes:
	 * 1. nodes (0-1) will be connected by relation 0.
	 * 2. nodes (0-1) will be connected by relation 1.
	 * 3. nodes (1-2) will be connected by relation 1.
	 * 4. nodes (2-3) will be connected by relation 2.
	 * 5. nodes (3-4) will be connected by relation 3. */

	Graph_ConnectNodes(g, 0, 1, relations[0], &e);
	Graph_ConnectNodes(g, 0, 1, relations[1], &e);
	Graph_ConnectNodes(g, 1, 2, relations[1], &e);
	Graph_ConnectNodes(g, 2, 3, relations[2], &e);
	Graph_ConnectNodes(g, 3, 4, relations[3], &e);

	// Validations
	// Try to get edges by ID
	for(EdgeID i = 0; i < edgeCount; i++) {
		Graph_GetEdge(g, i, &e);
		ASSERT_TRUE(e.entity != NULL);
		ASSERT_EQ(e.entity->id, i);
	}

	// Try to get edges connecting source to destination node.
	NodeID src;
	NodeID dest;
	int relation;
	Edge *edges = (Edge *)array_new(Edge, 4);

	/* Get all edges connecting node 0 to node 1,
	 * expecting 2 edges. */
	src = 0;
	dest = 1;
	relation = GRAPH_NO_RELATION;   // Relation agnostic.
	Graph_GetEdgesConnectingNodes(g, src, dest, relation, &edges);
	ASSERT_EQ(array_len(edges), 2);
	for(int i = 0; i < 2; i++) {
		e = edges[i];
		relation = relations[i];
		ASSERT_TRUE(e.entity != NULL);
		ASSERT_EQ(Edge_GetSrcNodeID(&e), src);
		ASSERT_EQ(Edge_GetDestNodeID(&e), dest);
		ASSERT_EQ(Edge_GetRelationID(&e), relation);
	}
	array_clear(edges);

	// Try to get none existing edges:

	// Node 0 is not connected to 2.
	src = 0;
	dest = 2;
	relation = GRAPH_NO_RELATION;
	Graph_GetEdgesConnectingNodes(g, src, dest, relation, &edges);
	ASSERT_EQ(array_len(edges), 0);
	array_clear(edges);

	// Node 0 is not connected to 1 via relation 2.
	src = 0;
	dest = 1;
	relation = relations[2];
	Graph_GetEdgesConnectingNodes(g, src, dest, relation, &edges);
	ASSERT_EQ(array_len(edges), 0);
	array_clear(edges);

	// Node 1 is not connected to 0 via relation 0.
	src = 1;
	dest = 0;
	relation = relations[0];
	Graph_GetEdgesConnectingNodes(g, src, dest, relation, &edges);
	ASSERT_EQ(array_len(edges), 0);
	array_clear(edges);

	// No node connects to itself.
	for(NodeID i = 0; i < nodeCount; i++) {
		for(int j = 0; j < relationCount; j++) {
			src = i;
			relation = relations[j];
			Graph_GetEdgesConnectingNodes(g, src, src, relation, &edges);
			ASSERT_EQ(array_len(edges), 0);
			array_clear(edges); // Reset edge count.
		}
		relation = GRAPH_NO_RELATION;
		Graph_GetEdgesConnectingNodes(g, src, src, relation, &edges);
		ASSERT_EQ(array_len(edges), 0);
		array_clear(edges); // Reset edge count.
	}

	array_free(edges);
	Graph_ReleaseLock(g);
	Graph_Free(g);
}


TEST_F(GraphTest, BulkDelete) {
	// Create graph.
	int node_count = 5;
	int edge_count = 13;
	Node n[node_count];
	Edge e[edge_count];
	Graph *g = Graph_New(16, 16);

	Graph_AcquireWriteLock(g);

	int l = Graph_AddLabel(g);
	int r0 = Graph_AddRelationType(g);
	int r1 = Graph_AddRelationType(g);

	for(int i = 0; i < node_count; i++) Graph_CreateNode(g, l, &n[i]);

	/* Connect nodes:
	 * (0)-[r0]->(1)
	 * (0)-[r0]->(1)
	 * (0)-[r1]->(1)
	 * (0)-[r1]->(1)
	 *
	 * (1)-[r0]->(0)
	 * (1)-[r0]->(0)
	 * (1)-[r1]->(0)
	 *
	 * (2)-[r0]->(0)
	 * (2)-[r1]->(1)
	 * (2)-[r1]->(3)
	 *
	 * (3)-[r1]->(4)
	 * (3)-[r1]->(4)
	 *
	 * (4)-[r0]->(3) */

	// Node 0 is connected to node 1 with multiple edges.
	Graph_ConnectNodes(g, ENTITY_GET_ID(&n[0]), ENTITY_GET_ID(&n[1]), r0, &e[0]);
	Edge_SetRelationID(&e[0], r0);
	Graph_ConnectNodes(g, ENTITY_GET_ID(&n[0]), ENTITY_GET_ID(&n[1]), r0, &e[1]);
	Edge_SetRelationID(&e[1], r0);
	Graph_ConnectNodes(g, ENTITY_GET_ID(&n[0]), ENTITY_GET_ID(&n[1]), r1, &e[2]);
	Edge_SetRelationID(&e[2], r1);
	Graph_ConnectNodes(g, ENTITY_GET_ID(&n[0]), ENTITY_GET_ID(&n[1]), r1, &e[3]);
	Edge_SetRelationID(&e[3], r1);

	// Node 1 is connected to node 0 with multiple edges.
	Graph_ConnectNodes(g, ENTITY_GET_ID(&n[1]), ENTITY_GET_ID(&n[0]), r0, &e[4]);
	Edge_SetRelationID(&e[4], r0);
	Graph_ConnectNodes(g, ENTITY_GET_ID(&n[1]), ENTITY_GET_ID(&n[0]), r0, &e[5]);
	Edge_SetRelationID(&e[5], r0);
	Graph_ConnectNodes(g, ENTITY_GET_ID(&n[1]), ENTITY_GET_ID(&n[0]), r1, &e[6]);
	Edge_SetRelationID(&e[6], r1);

	// Node 2 is connected to nodes 0,1 and 3.
	Graph_ConnectNodes(g, ENTITY_GET_ID(&n[2]), ENTITY_GET_ID(&n[0]), r0, &e[7]);
	Edge_SetRelationID(&e[7], r0);
	Graph_ConnectNodes(g, ENTITY_GET_ID(&n[2]), ENTITY_GET_ID(&n[1]), r1, &e[8]);
	Edge_SetRelationID(&e[8], r1);
	Graph_ConnectNodes(g, ENTITY_GET_ID(&n[2]), ENTITY_GET_ID(&n[3]), r1, &e[9]);
	Edge_SetRelationID(&e[9], r1);

	// Node 3 is connected to node 4 with multiple edges.
	Graph_ConnectNodes(g, ENTITY_GET_ID(&n[3]), ENTITY_GET_ID(&n[4]), r1, &e[10]);
	Edge_SetRelationID(&e[10], r1);
	Graph_ConnectNodes(g, ENTITY_GET_ID(&n[3]), ENTITY_GET_ID(&n[4]), r1, &e[11]);
	Edge_SetRelationID(&e[11], r1);

	// Node 4 is connected to node 3.
	Graph_ConnectNodes(g, ENTITY_GET_ID(&n[4]), ENTITY_GET_ID(&n[3]), r0, &e[12]);
	Edge_SetRelationID(&e[12], r0);

	Graph_ReleaseLock(g);
	/* Delete nodes 0,1.
	 * Implicit deleted edges:
	 * 0 (0)-[r0]->(1)
	 * 1 (0)-[r0]->(1)
	 * 2 (0)-[r1]->(1)
	 * 3 (0)-[r1]->(1)
	 *
	 * 4 (1)-[r0]->(0)
	 * 5 (1)-[r0]->(0)
	 * 6 (1)-[r1]->(0)
	 *
	 * 7 (2)-[r0]->(0)
	 * 8 (2)-[r1]->(1) */

	Node nodes[4] = {n[0], n[1], n[0], n[1]};

	/* Delete edges 1,5 and 11.
	 * 0  (0)-[r0]->(1) - Duplicate
	 * 4  (1)-[r0]->(0) - Duplicate
	 * 10 (3)-[r1]->(4) */
	Edge edges[6] = {e[0], e[0], e[4], e[4], e[10], e[10]};
	uint node_deleted = 0;
	uint edge_deleted = 0;

	Graph_AcquireWriteLock(g);
	Graph_BulkDelete(g, nodes, 4, edges, 6, &node_deleted, &edge_deleted);
	Graph_ReleaseLock(g);

	ASSERT_EQ(node_deleted, 2);
	// Statistics do not count for multi edge deletions.
	// ASSERT_EQ(edge_deleted, 10);

	/* Verification, updated graph:
	 * (2)-[r1]->(3)
	 *
	 * (3)-[r1]->(4)
	 *
	 * (4)-[r0]->(3) */

	ASSERT_EQ(Graph_NodeCount(g), 3);
	ASSERT_EQ(Graph_EdgeCount(g), 3);

	// Clean up.
	Graph_Free(g);
}

