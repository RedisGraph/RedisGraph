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
#include "../../src/execution_plan/optimizations/traverse_order.h"
#include "../../src/execution_plan/optimizations/traverse_order_utils.h"
#ifdef __cplusplus
}
#endif

class TraversalOrderingTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {
		// Use the malloc family for allocations
		Alloc_Reset();

		// Initialize GraphBLAS.
		GrB_init(GrB_NONBLOCKING);
		GxB_Global_Option_set(GxB_FORMAT, GxB_BY_COL); // all matrices in CSC format
		GxB_Global_Option_set(GxB_HYPER, GxB_NEVER_HYPER); // matrices are never hypersparse

		// Create a GraphContext.
		_fake_graph_context();
	}

	static void TearDownTestCase() {
	}

	static void _fake_graph_context() {
		GraphContext *gc = (GraphContext *)malloc(sizeof(GraphContext));

		gc->g = Graph_New(16, 16);
		gc->index_count = 0;
		gc->graph_name = strdup("G");
		gc->attributes = raxNew();
		pthread_rwlock_init(&gc->_attribute_rwlock, NULL);
		gc->string_mapping = (char **)array_new(char *, 64);
		gc->node_schemas = (Schema **)array_new(Schema *, GRAPH_DEFAULT_LABEL_CAP);
		gc->relation_schemas = (Schema **)array_new(Schema *, GRAPH_DEFAULT_RELATION_TYPE_CAP);

		GraphContext_AddSchema(gc, "L", SCHEMA_NODE);

		ASSERT_TRUE(QueryCtx_Init());
		QueryCtx_SetGraphCtx(gc);
        AR_RegisterFuncs();
	}

	/* Build a truth table as a 1-dimensional boolean array.
	 * Every sequence of n values constitutes a row,
	 * and altogether the rows demonstrate every permutation
	 * of true/false for each input. */
	static void _populate_combination_table(bool *table, int n) {
		int idx = 0; // Index into the overall table
		for(int i = 0; i < 1 << n; i ++) {          // For every value in 2^n
			for(int j = 0; j < n; j ++) {           // For each of the n bits in the value.
				bool x = (i & (1 << j));            // See if the bit at position i is set.
				table[idx++] = x;                   // If it is, this input will be true in this row.
			}
		}
	}

	AST *_build_ast(const char *query) {
		cypher_parse_result_t *parse_result = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
		AST *master_ast = AST_Build(parse_result);
		AST *ast = AST_NewSegment(master_ast, 0, cypher_ast_query_nclauses(master_ast->root));
		return ast;
	}

	FT_FilterNode *build_filter_tree_from_query(const char *query) {
		AST *ast = _build_ast(query);
		return AST_BuildFilterTree(ast);
	}

	QueryGraph *build_query_graph_from_query(const char *query) {
		AST *ast = _build_ast(query);
		return BuildQueryGraph(ast);
	}

	bool *build_combination_table(int node_count) {
		/* Build a table to test every combination of labeled nodes.
		 * If we have 4 nodes, this will be a 16x4 table,
		 * going from:
		 * 0000
		 * 0001
		 * [...]
		 * 1111 */
		int combination_count = 1 << node_count; // Calculate 2^n (16 for 4)
		int array_size = combination_count * node_count; // 16 (rows) * 4 (cols)
		bool *combination_table = (bool *)malloc(array_size);
		_populate_combination_table(combination_table, node_count);
		return combination_table;
	}

	bool _compare_algebraic_operand(AlgebraicExpression *a, AlgebraicExpression *b) {
		if(a->type != AL_OPERAND) return false;;
		if(b->type != AL_OPERAND) return false;
		return a->operand.matrix == b->operand.matrix;
	}

	bool compare_algebraic_expression(AlgebraicExpression *a, AlgebraicExpression *b) {
		if(a->type != b->type) return false;;
		if(AlgebraicExpression_ChildCount(a) !=  AlgebraicExpression_ChildCount(b)) return false;
		if(AlgebraicExpression_OperandCount(a) != AlgebraicExpression_OperandCount(b)) return false;
		// ASSERT_EQ(AlgebraicExpression_OperationCount(a, AL_EXP_ALL), AlgebraicExpression_OperationCount(b, AL_EXP_ALL));
		if(a->type == AL_OPERAND) return _compare_algebraic_operand(a, b);

		uint child_count = AlgebraicExpression_ChildCount(a);
		for(uint i = 0; i < child_count; i++) {
			if(!compare_algebraic_expression(a->operation.children[i], b->operation.children[i])) return false;
		}
		return true;
	}

	bool compare_algebraic_expressions(AlgebraicExpression **actual, AlgebraicExpression **expected,
									   uint count) {
		for(uint i = 0; i < count; i++) {
			if(!compare_algebraic_expression(actual[i], expected[i])) return false;
		}
		return true;
	}

	void assert_valid_permutation(AlgebraicExpression **actual_permutation,
								  AlgebraicExpression **expected_permutations, uint permutation_length, uint permutations_count) {
		bool res = false;
		for(uint i = 0; i < permutations_count; i++) {
			res |= compare_algebraic_expressions(actual_permutation, expected_permutations +i*permutation_length,
												 permutation_length);
		}
		ASSERT_TRUE(res);
	}

	void free_algebraic_expressions(AlgebraicExpression **exps, uint count) {
		for(uint i = 0; i < count; i++) {
			AlgebraicExpression *exp = exps[i];
			AlgebraicExpression_Free(exp);
		}
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

TEST_F(TraversalOrderingTest, TwoOptimalArrangements) {
	/* Given the set of algebraic expressions:
	 * { [AB], [BC], [BD] }
	 * We represent the traversal:
	 * (A:L {v: 1})->(B)->(C), (B)->(D:L {v: 1})
	 * The optimal order of traversals should always be:
	 * { 
     *      {[AB], [BC], [BD]}
     *      {[BC]', [AB]', [BD]}
     *  }
	 * Validate this for all input permutations.
	 */

	FT_FilterNode *filters;
	char *query = "MATCH (A:L {v: 1})-->(B)-->(C), (B)-->(D:L {v: 1}) RETURN 1";
	AST *ast = _build_ast(query);
	QueryGraph *qg = BuildQueryGraph(ast);

	AlgebraicExpression *ExpAB = AlgebraicExpression_NewOperand(GrB_NULL, false, "A", "B", NULL, "L");

	AlgebraicExpression *TAB = AlgebraicExpression_NewOperand(GrB_NULL, false, "A", "B", NULL, "L");
	AlgebraicExpression_Transpose(&TAB);
	AlgebraicExpression *ExpBC = AlgebraicExpression_NewOperand(GrB_NULL, false, "B", "C", NULL, NULL);
	AlgebraicExpression *TBC = AlgebraicExpression_NewOperand(GrB_NULL, false, "B", "C", NULL, NULL);
	AlgebraicExpression_Transpose(&TBC);
	AlgebraicExpression *ExpBD = AlgebraicExpression_NewOperand(GrB_NULL, false, "B", "D", NULL, "L");

	filters = build_filter_tree_from_query(
				  "MATCH (A:L {v: 1})-[]->(B)-[]->(C), (B)-[]->(D:L {v: 1})) RETURN *");

	AlgebraicExpression *expected[] = {
		ExpAB, ExpBC, ExpBD,
		TBC, TAB, ExpBD
	};

	AlgebraicExpression *set[3] = {ExpAB, ExpBC, ExpBD};
	std::sort(set, set + 3);
	// Test every permutation of the set.
	do {
		AlgebraicExpression *tmp[3] = {AlgebraicExpression_Clone(set[0]), AlgebraicExpression_Clone(set[1]), AlgebraicExpression_Clone(set[2])};
		orderExpressions(qg, tmp, 3, filters, NULL);
		assert_valid_permutation(tmp, expected, 3, 2);
		free_algebraic_expressions(tmp, 3);
	} while(std::next_permutation(set, set + 3));

	// Clean up.
	FilterTree_Free(filters);
	AlgebraicExpression_Free(TAB);
	AlgebraicExpression_Free(TBC);
	AlgebraicExpression_Free(ExpBD);
	QueryGraph_Free(qg);
}

TEST_F(TraversalOrderingTest, ValidateLabelScoring) {
	/* Given the graph with the structure:
	 * (A)->(B)->(C)->(D), (D)->(A), (B)->(D)
	 * Test all permutations of labeled nodes and
	 * validate that all produce an optimal scoring. */
	uint node_count = 4;

	char *query = "MATCH (A)-->(B)-->(C)-->(D), (D)-->(A), (B)-->(D) RETURN 1";
	AST *ast = _build_ast(query);
	QueryGraph *qg = BuildQueryGraph(ast);
	QGNode **nodes = qg->nodes;

	// Generate the set of AlgebraicExpressions.
	AlgebraicExpression **orig_set = AlgebraicExpression_FromQueryGraph(qg);
	uint exp_count = array_len(orig_set);

	int combination_count = 1 << node_count; // Calculate 2^4 (16)
	bool *combination_table = build_combination_table(node_count);

	OrderScoreCtx score_ctx = {.qg = qg,
							   .bound_vars = NULL,
							   .filtered_entities = NULL,
							   .independent_entities = NULL,
							   .best_arrangement = (AlgebraicExpression **)rm_malloc(exp_count * sizeof(AlgebraicExpression *)),
							   .max_score = INT_MIN
							  };
	int ctr = 0;
	for(int to_label = 0; to_label < combination_count; to_label ++) {
		// Label the appropriate nodes in the sequence.
		for(int i = 0; i < node_count; i ++) {
			if(combination_table[to_label * node_count + i]) nodes[i]->label = "L";
			else nodes[i]->label = NULL;
		}
		// Copy the initial set.
		AlgebraicExpression *set[exp_count];
		memcpy(set, orig_set, exp_count * sizeof(AlgebraicExpression *));

		// Order the expressions and obtain the score.
		orderExpressions(qg, set, exp_count, NULL, NULL);
		int first_score = score_arrangement(&score_ctx, set, exp_count);
		memcpy(set, orig_set, exp_count * sizeof(AlgebraicExpression *));

		// Test every permutation of the set.
		std::sort(set, set + exp_count);
		do {
			AlgebraicExpression *tmp[exp_count];
			memcpy(tmp, set, exp_count * sizeof(AlgebraicExpression *));
			ctr ++;
			orderExpressions(qg, tmp, exp_count, NULL, NULL);
			int score = score_arrangement(&score_ctx, tmp, exp_count);
			// Every sequence of expressions should achieve the same score.
			ASSERT_EQ(score, first_score);
		} while(std::next_permutation(set, set + exp_count));
	}
	// Clean up.
	QueryGraph_Free(qg);
}

TEST_F(TraversalOrderingTest, ValidateFilterAndLabelScoring) {
	/* Given the graph with the structure:
	 * (A)->(B)->(C)->(D), (D)->(A), (B)->(D)
	 * Test all permutations of labeled and filtered nodes and
	 * validate that all produce an optimal scoring. */

	char *query = "MATCH (A)-->(B)-->(C)-->(D), (D)-->(A), (B)-->(D) RETURN 1";
	QueryGraph *qg = build_query_graph_from_query(query); // Build the QueryGraph.
	QGNode **nodes = qg->nodes;
	uint node_count = array_len(nodes);

	// Build the truth table for making label and filter combinations.
	int combination_count = 1 << node_count; // Calculate 2^4 (16)
	bool *combination_table = build_combination_table(combination_count);

	// Set up the filter string
	int len = 1000;
	char filter_str[len];
	int base_offset = snprintf(filter_str, len,
							   "MATCH (A)-->(B)-->(C)-->(D), (D)-->(A), (B)-->(D) WHERE ");
	int ctr = 0;
	// For every label combination
	for(int to_label = 0; to_label < combination_count; to_label ++) {
		// Label the appropriate nodes in the sequence.
		for(int i = 0; i < node_count; i ++) {
			if(combination_table[to_label * 4 + i]) nodes[i]->label = "L";
			else nodes[i]->label = NULL;
		}
		AlgebraicExpression **orig_set = AlgebraicExpression_FromQueryGraph(qg);
		uint exp_count = array_len(orig_set);
		// Populate the initial set.
		AlgebraicExpression *set[exp_count];
		memcpy(set, orig_set, exp_count * sizeof(AlgebraicExpression *));
		AlgebraicExpression *tmp[exp_count];
		OrderScoreCtx score_ctx = {.qg = qg,
								   .bound_vars = NULL,
								   .filtered_entities = NULL,
								   .independent_entities = NULL,
								   .best_arrangement = (AlgebraicExpression **)rm_malloc(exp_count * sizeof(AlgebraicExpression *)),
								   .max_score = INT_MIN
								  };
		// For every filter combination
		for(int to_filter = 1; to_filter < combination_count; to_filter ++) {
			// Reset the filter string to its base section.
			int offset = base_offset;
			// Filter the appropriate nodes in the sequence.
			for(int j = 0; j < node_count; j ++) {
				if(combination_table[to_filter * 4 + j]) {
					// If this isn't the first filter, precede it with "AND ".
					if(offset != base_offset) offset += snprintf(filter_str + offset, len - offset, "AND ");
					// Add a filter for the current node's alias.
					offset += snprintf(filter_str + offset, len - offset, "%s.v = 1 ", nodes[j]->alias);
				}
			}
			// Finalize the query
			offset += snprintf(filter_str + offset, len - offset, "RETURN *");
			// Build the filter tree to represent the query.
			FT_FilterNode *filters = build_filter_tree_from_query(filter_str);
			rax *filters_rax = FilterTree_CollectModified(filters);
			score_ctx.filtered_entities = filters_rax;
			if(to_label == 1 && to_filter == 2) {
				// printf("%s\n", filter_str);
			}
			// Order the expressions and obtain the score.
			orderExpressions(qg, set, exp_count, filters, NULL);
			int first_score = score_arrangement(&score_ctx, set, exp_count);
			if(to_label == 5 && to_filter == 5) {
				printf("%s\n", filter_str);
				for(uint j = 0; j < node_count; j ++) {
					printf("Node %d:%s, ", j, nodes[j]->label ? "L" : "");
				}
				printf("\n");
				for(uint j = 0; j < exp_count; j ++) {
					printf("Expression %d:\n", j);
					AlgebraicExpression_PrintTree(set[j]);
				}
			}
			memcpy(set, orig_set, exp_count * sizeof(AlgebraicExpression *));

			// Test every permutation of the set.
			std::sort(set, set + exp_count);
			do {
				// Check if the current 'set' arrangement is valid.
				bool valid = true;
				for(int i = 0; i < exp_count; i ++) {
					if(valid_position(set, i, set[i], qg)) continue;
					valid = false;
					break;
				}
				// No valid arrangement should have a higher score than the first computed one.
				if(valid) {
					ctr++;
					if(ctr == 6507) {
						printf("i ");
					}
					int score = score_arrangement(&score_ctx, set, exp_count);
					if(score > first_score) {
						printf("aah\n");
						for(uint j = 0; j < exp_count; j ++) {
							printf("Expression %d:\n", j);
							AlgebraicExpression_PrintTree(set[j]);
						}
					}
					ASSERT_LE(score, first_score);
				}

				// Copy the current sequence into the tmp array.
				memcpy(tmp, set, exp_count * sizeof(AlgebraicExpression *));
				// Reorder the expressions.
				orderExpressions(qg, tmp, exp_count, filters, NULL);
				// Each order should produce the optimal result.
				int score = score_arrangement(&score_ctx, tmp, exp_count);
				if(score != first_score) {
					for(uint j = 0; j < exp_count; j ++) {
						printf("Expression %d:\n", j);
						AlgebraicExpression_PrintTree(set[j]);
					}
					printf("Scored %d, first score was %d\n", score, first_score);
				}
				// Every sequence of expressions should achieve the same score.
				ASSERT_EQ(score, first_score);
			} while(std::next_permutation(set, set + exp_count));


			FilterTree_Free(filters);
			raxFree(filters_rax);
		}
	}

	// Clean up.
	QueryGraph_Free(qg);
}

TEST_F(TraversalOrderingTest, SuboptimalOrder) {
	char *query =
		"MATCH (A)-->(B)-->(C:L {v: 1})-->(F), (A)-->(B)-->(D {v: 1})-->(E {v: 1})-->(F) RETURN *";
	QueryGraph *qg = build_query_graph_from_query(query); // Build the QueryGraph.

	// Build the AlgebraicExpressions.
	AlgebraicExpression **orig_set = AlgebraicExpression_FromQueryGraph(qg);
	uint exp_count = array_len(orig_set);

	// Build the filter tree to represent the query.
	FT_FilterNode *filters = build_filter_tree_from_query(query);
	rax *filters_rax = FilterTree_CollectModified(filters);
	rax *indpendent_filters = raxNew();
	FilterTree_CollectIndependentEntities(filters, indpendent_filters);

	// Populate the initial set.
	AlgebraicExpression *set[exp_count];
	memcpy(set, orig_set, exp_count * sizeof(AlgebraicExpression *));

	// Set A as a bound variable so that it will always start the sequence.
	rax *bound_vars = raxNew();
	raxInsert(bound_vars, (unsigned char *)"A", strlen("A"), NULL, NULL);

	OrderScoreCtx score_ctx = {.qg = qg,
							   .bound_vars = bound_vars,
							   .filtered_entities = filters_rax,
							   .independent_entities = indpendent_filters,
							   .best_arrangement = (AlgebraicExpression **)rm_malloc(exp_count * sizeof(AlgebraicExpression *)),
							   .max_score = INT_MIN
							  };
	// Order the expressions and obtain the score.
	orderExpressions(qg, set, exp_count, filters, bound_vars);
	int first_score = score_arrangement(&score_ctx, set, exp_count);
	memcpy(set, orig_set, exp_count * sizeof(AlgebraicExpression *));

	// Test every permutation of the set.
	std::sort(set, set + exp_count);
	do {
		// Check if the current 'set' arrangement is valid.
		bool valid = true;
		for(int i = 0; i < exp_count; i ++) {
			if(valid_position(set, i, set[i], qg)) continue;
			valid = false;
			break;
		}
		// No valid arrangement should have a higher score than the first computed one.
		if(valid) {
			int score = score_arrangement(&score_ctx, set, exp_count);
			ASSERT_LE(score, first_score); // TODO TODO should fail!
		}

		// Copy the current sequence into the tmp array.
		AlgebraicExpression *tmp[exp_count];
		memcpy(tmp, set, exp_count * sizeof(AlgebraicExpression *));
		// Reorder the expressions.
		orderExpressions(qg, tmp, exp_count, filters, bound_vars);
		// Each order should produce the optimal result.
		int score = score_arrangement(&score_ctx, tmp, exp_count);
		if(score != first_score) {
			printf("Scored %d, first score was %d\n", score, first_score);
		}
		// Every sequence of expressions should achieve the same score.
		ASSERT_EQ(score, first_score);
	} while(std::next_permutation(set, set + exp_count));


	// Clean up.
	FilterTree_Free(filters);
	raxFree(filters_rax);
	QueryGraph_Free(qg);
}

