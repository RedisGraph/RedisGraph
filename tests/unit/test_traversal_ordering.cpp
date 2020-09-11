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
#include "../../src/query_ctx.h"
#include "../../src/arithmetic/funcs.h"
#include "../../src/graph/query_graph.h"
#include "../../src/filter_tree/filter_tree.h"
#include "../../src/ast/ast_build_filter_tree.h"
#include "../../src/arithmetic/algebraic_expression.h"
#include "../../src/execution_plan/optimizations/optimizations.h"

#ifdef __cplusplus
}
#endif

class TraversalOrderingTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {
		// Use the malloc family for allocations
		Alloc_Reset();
		_fake_graph_context();
	}

	static void TearDownTestCase() {
	}

	static void _fake_graph_context() {
		/* Filter tree construction requires access to schemas,
		 * those inturn resides within graph context
		 * accessible via thread local storage, as such we're creating a
		 * fake graph context and placing it within thread local storage. */
		GraphContext *gc = (GraphContext *)calloc(1, sizeof(GraphContext));
		gc->attributes = raxNew();
		pthread_rwlock_init(&gc->_attribute_rwlock, NULL);

		// Prepare thread-local variables
		ASSERT_TRUE(QueryCtx_Init());
		QueryCtx_SetGraphCtx(gc);
		AR_RegisterFuncs();
	}

	AST *_build_ast(const char *query) {
		cypher_parse_result_t *parse_result = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
		AST *ast = AST_Build(parse_result);
		return ast;
	}

	FT_FilterNode *build_filter_tree_from_query(const char *query) {
		AST *ast = _build_ast(query);
		return AST_BuildFilterTree(ast);
	}
};

TEST_F(TraversalOrderingTest, FilterFirst) {
	/* Given the ordered (left to right) set of algebraic expression:
	 * { [AB], [BC], [CD] }
	 * Which represents the traversal:
	 * (A)->(B)->(C)->(D)
	 * And a set of filters:
	 * C.V = X.
	 *
	 * We can reorder this set in such away
	 * that filters are applied as early as possible.
	 *
	 * Here are all of the possible permutations of the set
	 * in which the filter is applied at the earliest step:
	 * { [CB], [BA], [CD] } (D)<-(C)->(B)->(A) (2 transposes)
	 * { [CB], [CD], [BA] } (D)<-(C)->(B)->(A) (2 transposes)
	 * { [CD], [CB], [BA] } (A)<-(B)<-(C)->(D) (2 transposes) */

	FT_FilterNode *filters;
	QGNode *A = QGNode_New("A");
	QGNode *B = QGNode_New("B");
	QGNode *C = QGNode_New("C");
	QGNode *D = QGNode_New("D");

	QGEdge *AB = QGEdge_New("E", "AB");
	QGEdge *BC = QGEdge_New("E", "BC");
	QGEdge *CD = QGEdge_New("E", "CD");

	QueryGraph *qg = QueryGraph_New(4, 3);

	QueryGraph_AddNode(qg, A);
	QueryGraph_AddNode(qg, B);
	QueryGraph_AddNode(qg, C);
	QueryGraph_AddNode(qg, D);
	QueryGraph_ConnectNodes(qg, A, B, AB);
	QueryGraph_ConnectNodes(qg, B, C, BC);
	QueryGraph_ConnectNodes(qg, C, D, CD);

	AlgebraicExpression *set[3];
	AlgebraicExpression *ExpAB = AlgebraicExpression_NewOperand(GrB_NULL, false, "A", "B", NULL, NULL);
	AlgebraicExpression *ExpBC = AlgebraicExpression_NewOperand(GrB_NULL, false, "B", "C", NULL, NULL);
	AlgebraicExpression *ExpCD = AlgebraicExpression_NewOperand(GrB_NULL, false, "C", "D", NULL, NULL);

	// { [AB], [BC], [CD] }
	set[0] = ExpAB;
	set[1] = ExpBC;
	set[2] = ExpCD;

	filters = build_filter_tree_from_query(
				  "MATCH (A)-[]->(B)-[]->(C)-[]->(D) WHERE A.val = 1 RETURN *");

	orderExpressions(qg, set, 3, filters, NULL);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBC);
	ASSERT_EQ(set[2], ExpCD);

	FilterTree_Free(filters);

	// { [AB], [BC], [CD] }
	set[0] = ExpAB;
	set[1] = ExpBC;
	set[2] = ExpCD;

	filters = build_filter_tree_from_query("MATCH (A)-[]->(B)-[]->(C)-[]->(D) WHERE B.val = 1 RETURN *");

	orderExpressions(qg, set, 3, filters, NULL);

	ASSERT_STREQ(AlgebraicExpression_Source(set[0]), "B");

	FilterTree_Free(filters);

	// { [AB], [BC], [CD] }
	set[0] = ExpAB;
	set[1] = ExpBC;
	set[2] = ExpCD;

	filters = build_filter_tree_from_query("MATCH (A)-[]->(B)-[]->(C)-[]->(D) WHERE C.val = 1 RETURN *");

	orderExpressions(qg, set, 3, filters, NULL);
	ASSERT_STREQ(AlgebraicExpression_Source(set[0]), "C");

	FilterTree_Free(filters);

	// { [AB], [BC], [CD] }
	set[0] = ExpAB;
	set[1] = ExpBC;
	set[2] = ExpCD;

	filters = build_filter_tree_from_query("MATCH (A)-[]->(B)-[]->(C)-[]->(D) WHERE D.val = 1 RETURN *");

	orderExpressions(qg, set, 3, filters, NULL);

	ASSERT_STREQ(AlgebraicExpression_Source(set[0]), "D");

	// Clean up.
	FilterTree_Free(filters);
	AlgebraicExpression_Free(ExpAB);
	AlgebraicExpression_Free(ExpBC);
	AlgebraicExpression_Free(ExpCD);
	QueryGraph_Free(qg);
}

TEST_F(TraversalOrderingTest, SingleOptimalArrangement) {
	/* Given the set of algebraic expressions:
	 * { [AB], [BC], [BD] }
	 * We represent the traversal:
	 * (A:L {v: 1})->(B)->(C), (B)->(D:L {v: 1})
	 * The optimal order of traversals should always be:
	 * { [AB], [BD], [BC] }
	 * Validate this for all input permutations.
	 */

	FT_FilterNode *filters;
	QGNode *A = QGNode_New("A");
	QGNode *B = QGNode_New("B");
	QGNode *C = QGNode_New("C");
	QGNode *D = QGNode_New("D");
	A->label = "L";
	D->label = "L";

	QGEdge *AB = QGEdge_New("E", "AB");
	QGEdge *BC = QGEdge_New("E", "BC");
	QGEdge *BD = QGEdge_New("E", "BD");

	QueryGraph *qg = QueryGraph_New(4, 3);

	QueryGraph_AddNode(qg, A);
	QueryGraph_AddNode(qg, B);
	QueryGraph_AddNode(qg, C);
	QueryGraph_AddNode(qg, D);
	QueryGraph_ConnectNodes(qg, A, B, AB);
	QueryGraph_ConnectNodes(qg, B, C, BC);
	QueryGraph_ConnectNodes(qg, B, D, BD);

	AlgebraicExpression *ExpAB = AlgebraicExpression_NewOperand(GrB_NULL, false, "A", "B", NULL, "L");
	AlgebraicExpression *ExpBC = AlgebraicExpression_NewOperand(GrB_NULL, false, "B", "C", NULL, NULL);
	AlgebraicExpression *ExpBD = AlgebraicExpression_NewOperand(GrB_NULL, false, "B", "D", NULL, "L");

	filters = build_filter_tree_from_query(
				  "MATCH (A:L {v: 1})-[]->(B)-[]->(C), (B)-[]->(D:L {v: 1})) RETURN *");

	AlgebraicExpression *set[3] = {ExpAB, ExpBC, ExpBD};
	std::sort(set, set + 3);
	// Test every permutation of the set.
	do {
		AlgebraicExpression *tmp[3] = {set[0], set[1], set[2]};
		orderExpressions(qg, tmp, 3, filters, NULL);
		ASSERT_EQ(tmp[0], ExpAB);
		ASSERT_EQ(tmp[1], ExpBD);
		ASSERT_EQ(tmp[2], ExpBC);
	} while(std::next_permutation(set, set + 3));

	// Clean up.
	FilterTree_Free(filters);
	AlgebraicExpression_Free(ExpAB);
	AlgebraicExpression_Free(ExpBC);
	AlgebraicExpression_Free(ExpBD);
	QueryGraph_Free(qg);
}

/* Build a truth table as a 1-dimensional boolean array.
 * Every sequence of n values constitutes a row,
 * and altogether the rows demonstrate every permutation
 * of true/false for each input. */
static void populate_truth_table(bool *table, int n) {
	int idx = 0; // Index into the overall table
	for(int i = 0; i < 1 << n; i ++) {          // For every value in 2^n
		for(int j = 0; j < sizeof(int); j ++) { // For each of the 4 bits in the value.
			bool x = (i & (1 << j));            // See if the bit at position i is set.
			table[idx++] = x;                   // If it is, this input will be true in this row.
		}
	}
}

TEST_F(TraversalOrderingTest, ValidateLabelScoring) {
	/* Given the graph with the structure:
	 * (A)->(B)->(C)->(D), (D)->(A), (B)->(D)
	 * Test all permutations of labeled nodes and
	 * validate that all produce an optimal scoring. */
	uint exp_count = 5;
	int node_count = 4;
	int edge_count = 5;
	QueryGraph *qg = QueryGraph_New(node_count, edge_count);

	// Build QGNodes
	QGNode *nodes[node_count];
	for(int i = 0; i < node_count + 1; i ++) {
		char *alias;
		asprintf(&alias, "%c", i + 'A');
		nodes[i] = QGNode_New(alias);
		QueryGraph_AddNode(qg, nodes[i]);
	}

	QGEdge *AB = QGEdge_New("E", "AB");
	QueryGraph_ConnectNodes(qg, nodes[0], nodes[1], AB);
	QGEdge *BC = QGEdge_New("E", "BC");
	QueryGraph_ConnectNodes(qg, nodes[1], nodes[2], BC);
	QGEdge *CD = QGEdge_New("E", "CD");
	QueryGraph_ConnectNodes(qg, nodes[2], nodes[3], CD);
	QGEdge *CA = QGEdge_New("E", "CA");
	QueryGraph_ConnectNodes(qg, nodes[3], nodes[0], CA);
	QGEdge *BD = QGEdge_New("E", "BD");
	QueryGraph_ConnectNodes(qg, nodes[1], nodes[3], BD);

	AlgebraicExpression *ExpAB = AlgebraicExpression_NewOperand(GrB_NULL, false, "A", "B", NULL, NULL);
	AlgebraicExpression *ExpBC = AlgebraicExpression_NewOperand(GrB_NULL, false, "B", "C", NULL, NULL);
	AlgebraicExpression *ExpCD = AlgebraicExpression_NewOperand(GrB_NULL, false, "C", "D", NULL, NULL);
	AlgebraicExpression *ExpCA = AlgebraicExpression_NewOperand(GrB_NULL, false, "C", "A", NULL, NULL);
	AlgebraicExpression *ExpBD = AlgebraicExpression_NewOperand(GrB_NULL, false, "B", "D", NULL, NULL);

	// Store unmodified versions of operands, as ordering can modify them by introducing transposes.
	AlgebraicExpression *ExpAB_orig = AlgebraicExpression_NewOperand(GrB_NULL, false, "A", "B", NULL,
																	 NULL);
	AlgebraicExpression *ExpBC_orig = AlgebraicExpression_NewOperand(GrB_NULL, false, "B", "C", NULL,
																	 NULL);
	AlgebraicExpression *ExpCD_orig = AlgebraicExpression_NewOperand(GrB_NULL, false, "C", "D", NULL,
																	 NULL);
	AlgebraicExpression *ExpCA_orig = AlgebraicExpression_NewOperand(GrB_NULL, false, "C", "A", NULL,
																	 NULL);
	AlgebraicExpression *ExpBD_orig = AlgebraicExpression_NewOperand(GrB_NULL, false, "B", "D", NULL,
																	 NULL);
	AlgebraicExpression *orig_set[5] = {ExpAB_orig, ExpBC_orig, ExpCD_orig, ExpCA_orig, ExpBD_orig};

	/* Build a truth table to test every combination of labeled nodes.
	 * As we have 4 nodes, this will be a 16x4 table,
	 * going from:
	 * 0000
	 * 1000
	 * [...]
	 * 1111 */
	int combination_count = 1 << node_count; // Calculate 2^4 (16)
	int array_size = combination_count * 4; // 16 (rows) * 4 (cols)
	bool truth_table[array_size];
	populate_truth_table(truth_table, node_count);

	// Populate the initial set.
	AlgebraicExpression *set[5];
	memcpy(set, orig_set, exp_count * sizeof(AlgebraicExpression *));

	for(int to_label = 0; to_label < combination_count; to_label ++) {
		// Label the appropriate nodes in the sequence.
		for(int j = 0; j < node_count; j ++) {
			if(truth_table[to_label * 4 + j]) nodes[j]->label = "L";
			else nodes[j]->label = NULL;
		}

		// Order the expressions and obtain the score.
		orderExpressions(qg, set, exp_count, NULL, NULL);
		int first_score = score_arrangement(set, exp_count, qg, NULL, NULL);
		memcpy(set, orig_set, exp_count * sizeof(AlgebraicExpression *));

		// Test every permutation of the set.
		std::sort(set, set + exp_count);
		do {
			AlgebraicExpression *tmp[exp_count];
			memcpy(tmp, orig_set, exp_count * sizeof(AlgebraicExpression *));
			orderExpressions(qg, tmp, exp_count, NULL, NULL);
			int score = score_arrangement(tmp, exp_count, qg, NULL, NULL);
			/*
			if(score != first_score) {
				printf("Scored %d, first score was %d\n", score, first_score);
			}
			*/
			// Every sequence of expressions should achieve the same score.
			ASSERT_EQ(score, first_score);
		} while(std::next_permutation(set, set + exp_count));
	}
	// Clean up.
	AlgebraicExpression_Free(ExpAB);
	AlgebraicExpression_Free(ExpBC);
	AlgebraicExpression_Free(ExpCD);
	AlgebraicExpression_Free(ExpCA);
	AlgebraicExpression_Free(ExpBD);
	QueryGraph_Free(qg);
}

TEST_F(TraversalOrderingTest, ValidateFilterAndLabelScoring) {
	/* Given the graph with the structure:
	 * (A)->(B)->(C)->(D), (D)->(A), (B)->(D)
	 * Test all permutations of labeled and filtered nodes and
	 * validate that all produce an optimal scoring. */
	uint exp_count = 5;
	int node_count = 4;
	int edge_count = 5;
	QueryGraph *qg = QueryGraph_New(node_count, edge_count);

	// Build QGNodes
	QGNode *nodes[node_count];
	for(int i = 0; i < node_count + 1; i ++) {
		char *alias;
		asprintf(&alias, "%c", i + 'A');
		nodes[i] = QGNode_New(alias);
		QueryGraph_AddNode(qg, nodes[i]);
	}

	QGEdge *AB = QGEdge_New("E", "AB");
	QueryGraph_ConnectNodes(qg, nodes[0], nodes[1], AB);
	QGEdge *BC = QGEdge_New("E", "BC");
	QueryGraph_ConnectNodes(qg, nodes[1], nodes[2], BC);
	QGEdge *CD = QGEdge_New("E", "CD");
	QueryGraph_ConnectNodes(qg, nodes[2], nodes[3], CD);
	QGEdge *CA = QGEdge_New("E", "CA");
	QueryGraph_ConnectNodes(qg, nodes[3], nodes[0], CA);
	QGEdge *BD = QGEdge_New("E", "BD");
	QueryGraph_ConnectNodes(qg, nodes[1], nodes[3], BD);

	AlgebraicExpression *ExpAB = AlgebraicExpression_NewOperand(GrB_NULL, false, "A", "B", NULL, NULL);
	AlgebraicExpression *ExpBC = AlgebraicExpression_NewOperand(GrB_NULL, false, "B", "C", NULL, NULL);
	AlgebraicExpression *ExpCD = AlgebraicExpression_NewOperand(GrB_NULL, false, "C", "D", NULL, NULL);
	AlgebraicExpression *ExpCA = AlgebraicExpression_NewOperand(GrB_NULL, false, "C", "A", NULL, NULL);
	AlgebraicExpression *ExpBD = AlgebraicExpression_NewOperand(GrB_NULL, false, "B", "D", NULL, NULL);

	// Store unmodified versions of operands, as ordering can modify them by introducing transposes.
	AlgebraicExpression *ExpAB_orig = AlgebraicExpression_NewOperand(GrB_NULL, false, "A", "B", NULL,
																	 NULL);
	AlgebraicExpression *ExpBC_orig = AlgebraicExpression_NewOperand(GrB_NULL, false, "B", "C", NULL,
																	 NULL);
	AlgebraicExpression *ExpCD_orig = AlgebraicExpression_NewOperand(GrB_NULL, false, "C", "D", NULL,
																	 NULL);
	AlgebraicExpression *ExpCA_orig = AlgebraicExpression_NewOperand(GrB_NULL, false, "C", "A", NULL,
																	 NULL);
	AlgebraicExpression *ExpBD_orig = AlgebraicExpression_NewOperand(GrB_NULL, false, "B", "D", NULL,
																	 NULL);
	AlgebraicExpression *orig_set[5] = {ExpAB_orig, ExpBC_orig, ExpCD_orig, ExpCA_orig, ExpBD_orig};

	/* Build a truth table to test every combination of labeled nodes.
	 * As we have 4 nodes, this will be a 16x4 table,
	 * going from:
	 * 0000
	 * 1000
	 * [...]
	 * 1111 */
	int combination_count = 1 << node_count; // Calculate 2^4 (16)
	int array_size = combination_count * 4; // 16 (rows) * 4 (cols)
	bool truth_table[array_size];
	populate_truth_table(truth_table, node_count);

	// Populate the initial set.
	AlgebraicExpression *set[5];
	memcpy(set, orig_set, exp_count * sizeof(AlgebraicExpression *));

	FT_FilterNode *filters;
	// Set up the filter string
	int len = 1000;
	char filter_str[len];
	int base_offset = snprintf(filter_str, len,
							   "MATCH (A)-->(B)-->(C)-->(D), (D)-->(A), (B)-->(D) WHERE ");
	// For every label combination
	for(int to_label = 0; to_label < combination_count; to_label ++) {
		// For every filter combination
		for(int to_filter = 1; to_filter < combination_count; to_filter ++) {
			// Label the appropriate nodes in the sequence.
			for(int j = 0; j < node_count; j ++) {
				if(truth_table[to_label * 4 + j]) nodes[j]->label = "L";
				else nodes[j]->label = NULL;
			}

			// Reset the filter string to its base section.
			int offset = base_offset;
			// Filter the appropriate nodes in the sequence.
			for(int j = 0; j < node_count; j ++) {
				if(truth_table[to_filter * 4 + j]) {
					// If this isn't the first filter, precede it with "AND ".
					if(offset != base_offset) offset += snprintf(filter_str + offset, len - offset, "AND ");
					// Add a filter for the current node's alias.
					offset += snprintf(filter_str + offset, len - offset, "%s.v = 1 ", nodes[j]->alias);
				}
			}
			// Finalize the query
			offset += snprintf(filter_str + offset, len - offset, "RETURN 1");
			// Build the filter tree to represent the query.
			filters = build_filter_tree_from_query(filter_str);
			rax *filters_rax = FilterTree_CollectModified(filters);
			// Order the expressions and obtain the score.
			orderExpressions(qg, set, exp_count, filters, NULL);
			int first_score = score_arrangement(set, exp_count, qg, filters_rax, NULL);
			memcpy(set, orig_set, exp_count * sizeof(AlgebraicExpression *));

			// Test every permutation of the set.
			std::sort(set, set + exp_count);
			do {
				AlgebraicExpression *tmp[exp_count];
				memcpy(tmp, orig_set, exp_count * sizeof(AlgebraicExpression *));
				orderExpressions(qg, tmp, exp_count, filters, NULL);
				int score = score_arrangement(tmp, exp_count, qg, filters_rax, NULL);
				/*
				if(score != first_score) {
					printf("Scored %d, first score was %d\n", score, first_score);
				}
				*/
				// Every sequence of expressions should achieve the same score.
				ASSERT_EQ(score, first_score);
			} while(std::next_permutation(set, set + exp_count));


			FilterTree_Free(filters);
			raxFree(filters_rax);
		}
	}

	// Clean up.
	AlgebraicExpression_Free(ExpAB);
	AlgebraicExpression_Free(ExpBC);
	AlgebraicExpression_Free(ExpCD);
	AlgebraicExpression_Free(ExpCA);
	AlgebraicExpression_Free(ExpBD);
	QueryGraph_Free(qg);
}

