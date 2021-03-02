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

TEST_F(TraversalOrderingTest, TransposeFree) {
	/* Given the ordered (left to right) set of algebraic expression:
	 * { [CD], [BC], [AB] }
	 * Which represents the traversal:
	 * (A)->(B)->(C)->(D)
	 * If we choose to start at C
	 * we can continue to D.
	 * Retract from C to B
	 * Retract from B to A.
	 * Overall we've performed 2 transposes:
	 * (A)<-(B) and (B)<-(C).
	 *
	 * We can reorder this set in such away
	 * that we won't perform any transposes.
	 *
	 * Here are all of the possible permutations of the set:
	 * { [AB], [BC], [CD] } (A)->(B)->(C)->(D)
	 * { [AB], [CD], [BC] } Invalid arrangement.
	 * { [BC], [AB], [CD] } (A)<-(B)->(C)->(D)
	 * { [BC], [CD], [AB] } (A)<-(B)->(C)->(D)
	 * { [CD], [AB], [BC] } Invalid arrangement.
	 * { [CD], [BC], [AB] } (A)<-(B)<-(C)->(D)
	 *
	 * Arrangement { [AB], [BC], [CD] }
	 * Is the only one that doesn't requires any transposes. */

	QGNode *A = QGNode_New("A");
	QGNode *B = QGNode_New("B");
	QGNode *C = QGNode_New("C");
	QGNode *D = QGNode_New("D");

	QGEdge *AB = QGEdge_NewRelationPattern(A, B, "E", "AB");
	QGEdge *BC = QGEdge_NewRelationPattern(B, C, "E", "BC");
	QGEdge *CD = QGEdge_NewRelationPattern(C, D, "E", "CD");

	QueryGraph *qg = QueryGraph_New(4, 3);

	QueryGraph_AddNode(qg, A);
	QueryGraph_AddNode(qg, B);
	QueryGraph_AddNode(qg, C);
	QueryGraph_AddNode(qg, D);
	QueryGraph_ConnectNodes(qg, A, B, AB);
	QueryGraph_ConnectNodes(qg, B, C, BC);
	QueryGraph_ConnectNodes(qg, C, D, CD);

	AlgebraicExpression *set[3];
	AlgebraicExpression *ExpAB = AlgebraicExpression_NewOperand(GrB_NULL, false, "A", "B", NULL, NULL, AlgExpReference_NewEmpty());
	AlgebraicExpression *ExpBC = AlgebraicExpression_NewOperand(GrB_NULL, false, "B", "C", NULL, NULL, AlgExpReference_NewEmpty());
	AlgebraicExpression *ExpCD = AlgebraicExpression_NewOperand(GrB_NULL, false, "C", "D", NULL, NULL, AlgExpReference_NewEmpty());

	// { [CD], [BC], [AB] }
	set[0] = ExpCD;
	set[1] = ExpBC;
	set[2] = ExpAB;

	orderExpressions(qg, set, 3, NULL, NULL);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBC);
	ASSERT_EQ(set[2], ExpCD);

	// { [AB], [BC], [CD] }
	set[0] = ExpAB;
	set[1] = ExpBC;
	set[2] = ExpCD;
	orderExpressions(qg, set, 3, NULL, NULL);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBC);
	ASSERT_EQ(set[2], ExpCD);

	// { [AB], [CD], [BC] }
	set[0] = ExpAB;
	set[1] = ExpCD;
	set[2] = ExpBC;
	orderExpressions(qg, set, 3, NULL, NULL);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBC);
	ASSERT_EQ(set[2], ExpCD);

	// { [BC], [AB], [CD] }
	set[0] = ExpBC;
	set[1] = ExpAB;
	set[2] = ExpCD;
	orderExpressions(qg, set, 3, NULL, NULL);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBC);
	ASSERT_EQ(set[2], ExpCD);

	// { [BC], [CD], [AB] }
	set[0] = ExpBC;
	set[1] = ExpCD;
	set[2] = ExpAB;
	orderExpressions(qg, set, 3, NULL, NULL);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBC);
	ASSERT_EQ(set[2], ExpCD);

	// { [CD], [AB], [BC] }
	set[0] = ExpCD;
	set[1] = ExpAB;
	set[2] = ExpBC;
	orderExpressions(qg, set, 3, NULL, NULL);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBC);
	ASSERT_EQ(set[2], ExpCD);

	// Clean up.
	AlgebraicExpression_Free(ExpAB);
	AlgebraicExpression_Free(ExpBC);
	AlgebraicExpression_Free(ExpCD);
	QueryGraph_Free(qg);
}

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

	QGEdge *AB = QGEdge_NewRelationPattern(A, B, "E", "AB");
	QGEdge *BC = QGEdge_NewRelationPattern(B, C, "E", "BC");
	QGEdge *CD = QGEdge_NewRelationPattern(C, D, "E", "CD");

	QueryGraph *qg = QueryGraph_New(4, 3);

	QueryGraph_AddNode(qg, A);
	QueryGraph_AddNode(qg, B);
	QueryGraph_AddNode(qg, C);
	QueryGraph_AddNode(qg, D);
	QueryGraph_ConnectNodes(qg, A, B, AB);
	QueryGraph_ConnectNodes(qg, B, C, BC);
	QueryGraph_ConnectNodes(qg, C, D, CD);

	AlgebraicExpression *set[3];
	AlgebraicExpression *ExpAB = AlgebraicExpression_NewOperand(GrB_NULL, false, "A", "B", NULL, NULL, AlgExpReference_NewEmpty());
	AlgebraicExpression *ExpBC = AlgebraicExpression_NewOperand(GrB_NULL, false, "B", "C", NULL, NULL, AlgExpReference_NewEmpty());
	AlgebraicExpression *ExpCD = AlgebraicExpression_NewOperand(GrB_NULL, false, "C", "D", NULL, NULL, AlgExpReference_NewEmpty());

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

TEST_F(TraversalOrderingTest, OptimalStartingPoint) {
	/* Given the single algebraic expression that represents the traversal:
	 * (A)->(B:L)->(C:L)
	 * And a set of filters:
	 * A.V = X, C.V = Y
	 *
	 * The starting point of the traversal should be C,
	 * as it is both labeled and filtered. */

	FT_FilterNode *filters;
	QGNode *A  = QGNode_New("A");
	QGNode *B  = QGNode_New("B");
	QGNode *C  = QGNode_New("C");
	QGEdge *AB = QGEdge_NewRelationPattern(A, B, "E", "AB");
	QGEdge *BC = QGEdge_NewRelationPattern(B, C, "E", "BC");

	B->label = "L";
	C->label = "L";

	QueryGraph *qg = QueryGraph_New(3, 2);

	QueryGraph_AddNode(qg, A);
	QueryGraph_AddNode(qg, B);
	QueryGraph_AddNode(qg, C);
	QueryGraph_ConnectNodes(qg, A, B, AB);
	QueryGraph_ConnectNodes(qg, B, C, BC);

	AlgebraicExpression *root = AlgebraicExpression_NewOperation(AL_EXP_MUL);
	AlgebraicExpression *ExpAB = AlgebraicExpression_NewOperand(GrB_NULL, false, "A", "B", NULL, NULL, AlgExpReference_NewEmpty());
	AlgebraicExpression *ExpBC = AlgebraicExpression_NewOperand(GrB_NULL, false, "B", "C", NULL, NULL, AlgExpReference_NewEmpty());
	AlgebraicExpression_AddChild(root, ExpAB);
	AlgebraicExpression_AddChild(root, ExpBC);

	filters = build_filter_tree_from_query(
				  "MATCH (A {val: 'v1'})-[]-(B:L)-[]->(C:L {val: 'v3'}) RETURN A");

	ASSERT_STRNE(AlgebraicExpression_Source(root), "C");
	orderExpressions(qg, &root, 1, filters, NULL);
	ASSERT_STREQ(AlgebraicExpression_Source(root), "C");

	// Clean up.
	FilterTree_Free(filters);
	AlgebraicExpression_Free(ExpAB);
	AlgebraicExpression_Free(ExpBC);
	QueryGraph_Free(qg);
}
