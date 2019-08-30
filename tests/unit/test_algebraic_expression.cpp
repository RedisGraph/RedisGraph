/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../../deps/googletest/include/gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "assert.h"
#include "../../src/value.h"
#include "../../src/util/arr.h"
#include "../../src/graph/graph.h"
#include "../../src/graph/query_graph.h"
#include "../../src/graph/graphcontext.h"
#include "../../src/util/simple_timer.h"
#include "../../src/arithmetic/algebraic_expression.h"
#include "../../src/util/rmalloc.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"

extern AR_ExpNode **_BuildReturnExpressions(RecordMap *record_map,
											const cypher_astnode_t *ret_clause);

#ifdef __cplusplus
}
#endif

extern pthread_key_t _tlsGCKey;    // Thread local storage graph context key.
extern pthread_key_t _tlsASTKey;  // Thread local storage AST key.

QueryGraph *qg;

// Matrices.
GrB_Matrix mat_p;
GrB_Matrix mat_ef;
GrB_Matrix mat_f;
GrB_Matrix mat_ev;
GrB_Matrix mat_c;
GrB_Matrix mat_ew;
GrB_Matrix mat_e;

const char *query_no_intermidate_return_nodes =
	"MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN p, e";
const char *query_one_intermidate_return_nodes =
	"MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN p, c, e";
const char *query_multiple_intermidate_return_nodes =
	"MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN p, f, c, e";

const char *query_return_first_edge =
	"MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN ef";
const char *query_return_intermidate_edge =
	"MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN ev";
const char *query_return_last_edge =
	"MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN ew";

class AlgebraicExpressionTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {
		// Use the malloc family for allocations
		Alloc_Reset();

		// Initialize GraphBLAS.
		GrB_init(GrB_NONBLOCKING);
		GxB_Global_Option_set(GxB_FORMAT, GxB_BY_COL); // all matrices in CSC format
		GxB_Global_Option_set(GxB_HYPER, GxB_NEVER_HYPER); // matrices are never hypersparse

		// Create a graph
		_fake_graph_context();
		_build_graph();
		_bind_matrices();
		int error = pthread_key_create(&_tlsASTKey, NULL);
		ASSERT_EQ(error, 0);
	}

	static void TearDownTestCase() {
		_free_fake_graph_context();
		GrB_finalize();
	}

	void SetUp() {
		qg = QueryGraph_New(16, 16);
	}

	void TearDown() {
		QueryGraph_Free(qg);
	}

	static void _fake_graph_context() {
		GraphContext *gc = (GraphContext *)malloc(sizeof(GraphContext));

		gc->g = Graph_New(16, 16);
		gc->index_count = 0;
		gc->graph_name = strdup("G");
		gc->attributes = raxNew();
		gc->string_mapping = (char **)array_new(char *, 64);
		gc->node_schemas = (Schema **)array_new(Schema *, GRAPH_DEFAULT_LABEL_CAP);
		gc->relation_schemas = (Schema **)array_new(Schema *, GRAPH_DEFAULT_RELATION_TYPE_CAP);

		GraphContext_AddSchema(gc, "Person", SCHEMA_NODE);
		GraphContext_AddSchema(gc, "City", SCHEMA_NODE);
		GraphContext_AddSchema(gc, "friend", SCHEMA_EDGE);
		GraphContext_AddSchema(gc, "visit", SCHEMA_EDGE);
		GraphContext_AddSchema(gc, "war", SCHEMA_EDGE);

		int error = pthread_key_create(&_tlsGCKey, NULL);
		ASSERT_EQ(error, 0);
		pthread_setspecific(_tlsGCKey, gc);
	}

	/* Create a graph containing:
	 * Entities: 'people' and 'countries'.
	 * Relations: 'friend', 'visit' and 'war'. */
	static void _build_graph() {
		GraphContext *gc = GraphContext_GetFromTLS();

		Node n;
		Graph *g = gc->g;
		Graph_AcquireWriteLock(g);

		size_t city_count = 2;
		size_t person_count = 2;
		size_t node_count = person_count + city_count;

		// Introduce person and country labels.
		int city_label = GraphContext_GetSchema(gc, "City", SCHEMA_NODE)->id;
		int person_label = GraphContext_GetSchema(gc, "Person", SCHEMA_NODE)->id;
		Graph_AllocateNodes(g, node_count);

		for(int i = 0; i < person_count; i++) {
			Graph_CreateNode(g, person_label, &n);
		}

		for(int i = 0; i < city_count; i++) {
			Graph_CreateNode(g, city_label, &n);
		}

		// Creates a relation matrices.
		GrB_Index war_relation_id = GraphContext_GetSchema(gc, "war", SCHEMA_EDGE)->id;
		GrB_Index visit_relation_id = GraphContext_GetSchema(gc, "visit", SCHEMA_EDGE)->id;
		GrB_Index friend_relation_id = GraphContext_GetSchema(gc, "friend", SCHEMA_EDGE)->id;

		// Introduce relations, connect nodes.
		Edge e;
		Graph_ConnectNodes(g, 0, 1, friend_relation_id, &e);
		Graph_ConnectNodes(g, 1, 0, friend_relation_id, &e);

		Graph_ConnectNodes(g, 0, 2, visit_relation_id, &e);
		Graph_ConnectNodes(g, 0, 3, visit_relation_id, &e);
		Graph_ConnectNodes(g, 1, 2, visit_relation_id, &e);

		Graph_ConnectNodes(g, 2, 3, war_relation_id, &e);
		Graph_ConnectNodes(g, 3, 2, war_relation_id, &e);

		Graph_ReleaseLock(g);
	}

	static void _bind_matrices() {
		int mat_id;
		GraphContext *gc = GraphContext_GetFromTLS();
		Graph *g = gc->g;

		mat_id = GraphContext_GetSchema(gc, "Person", SCHEMA_NODE)->id;
		mat_p = Graph_GetLabelMatrix(g, mat_id);
		mat_f = Graph_GetLabelMatrix(g, mat_id);

		mat_id = GraphContext_GetSchema(gc, "City", SCHEMA_NODE)->id;
		mat_c = Graph_GetLabelMatrix(g, mat_id);
		mat_e = Graph_GetLabelMatrix(g, mat_id);

		mat_id = GraphContext_GetSchema(gc, "friend", SCHEMA_EDGE)->id;
		mat_ef = Graph_GetRelationMatrix(g, mat_id);

		mat_id = GraphContext_GetSchema(gc, "visit", SCHEMA_EDGE)->id;
		mat_ev = Graph_GetRelationMatrix(g, mat_id);

		mat_id = GraphContext_GetSchema(gc, "war", SCHEMA_EDGE)->id;
		mat_ew = Graph_GetRelationMatrix(g, mat_id);
	}

	static void _free_fake_graph_context() {
		GraphContext *gc = GraphContext_GetFromTLS();
		GraphContext_Free(gc);
	}

	AlgebraicExpression **build_algebraic_expression(const char *query, size_t *exp_count) {
		GraphContext *gc = GraphContext_GetFromTLS();
		cypher_parse_result_t *parse_result = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
		AST *ast = AST_Build(parse_result);
		QueryGraph *qg = BuildQueryGraph(gc, ast);
		RecordMap *map = RecordMap_New();
		_BuildReturnExpressions(map, AST_GetClause(ast, CYPHER_AST_RETURN));
		AlgebraicExpression **ae = AlgebraicExpression_FromQueryGraph(qg, map, exp_count);

		return ae;
	}

	void _print_matrix(GrB_Matrix mat) {
		GrB_Index ncols, nrows, nvals;
		GrB_Matrix_ncols(&ncols, mat);
		GrB_Matrix_nrows(&nrows, mat);
		GrB_Matrix_nvals(&nvals, mat);
		printf("ncols: %llu, nrows: %llu, nvals: %llu\n", ncols, nrows, nvals);

		GrB_Index I[nvals];     // array for returning row indices of tuples
		GrB_Index J[nvals];     // array for returning col indices of tuples
		bool X[nvals];          // array for returning values of tuples

		GrB_Matrix_extractTuples_BOOL(I, J, X, &nvals, mat);
		for(int i = 0; i < nvals; i++) {
			printf("[%llu,%llu,%d]\n", I[i], J[i], X[i]);
		}
	}

	bool _compare_matrices(GrB_Matrix a, GrB_Matrix b) {
		GrB_Index acols, arows, avals;
		GrB_Index bcols, brows, bvals;

		GrB_Matrix_ncols(&acols, a);
		GrB_Matrix_nrows(&arows, a);
		GrB_Matrix_nvals(&avals, a);
		GrB_Matrix_ncols(&bcols, b);
		GrB_Matrix_nrows(&brows, b);
		GrB_Matrix_nvals(&bvals, b);

		if(acols != bcols || arows != brows || avals != bvals) {
			printf("acols: %llu bcols: %llu", acols, bcols);
			printf("arows: %llu brows: %llu", arows, brows);
			printf("avals: %llu bvals: %llu", avals, bvals);
			return false;
		}

		GrB_Index aI[avals];     // array for returning row indices of tuples
		GrB_Index aJ[avals];     // array for returning col indices of tuples
		bool aX[avals];          // array for returning values of tuples
		GrB_Index bI[bvals];     // array for returning row indices of tuples
		GrB_Index bJ[bvals];     // array for returning col indices of tuples
		bool bX[bvals];          // array for returning values of tuples

		GrB_Matrix_extractTuples_BOOL(aI, aJ, aX, &avals, a);
		GrB_Matrix_extractTuples_BOOL(bI, bJ, bX, &bvals, b);

		for(int i = 0; i < avals; i++) {
			if(aI[i] != bI[i] || aJ[i] != bJ[i] || aX[i] != bX[i]) {
				printf("Matrix A \n");
				_print_matrix(a);
				printf("\n\n");
				printf("Matrix B \n");
				_print_matrix(b);
				printf("\n\n");
				return false;
			}
		}

		return true;
	}

	void _compare_algebraic_operand(AlgebraicExpressionOperand *a, AlgebraicExpressionOperand *b) {
		ASSERT_EQ(a->free, b->free);
		ASSERT_EQ(a->operand, b->operand);
		ASSERT_EQ(a->diagonal, b->diagonal);
		ASSERT_EQ(a->transpose, b->transpose);
	}

	void _compare_algebraic_expression(AlgebraicExpression *a, AlgebraicExpression *b) {
		ASSERT_EQ(a->operand_count, b->operand_count);
		for(uint i = 0; i < a->operand_count; i++) {
			_compare_algebraic_operand(a->operands + i, b->operands + i);
		}
	}

	void compare_algebraic_expressions(AlgebraicExpression **actual, AlgebraicExpression **expected,
									   uint count) {
		for(uint i = 0; i < count; i++) {
			_compare_algebraic_expression(actual[i], expected[i]);
		}
	}

	void free_algebraic_expressions(AlgebraicExpression **exps, uint count) {
		for(uint i = 0; i < count; i++) {
			AlgebraicExpression *exp = exps[i];
			AlgebraicExpression_Free(exp);
		}
	}
};

TEST_F(AlgebraicExpressionTest, Exp_OP_ADD) {
	// Exp = A + B
	GrB_Matrix A;
	GrB_Matrix B;
	GrB_Matrix C;
	GrB_Matrix res;

	// A
	// 1 1
	// 0 0
	GrB_Matrix_new(&A, GrB_BOOL, 2, 2);
	GrB_Matrix_setElement_BOOL(A, true, 0, 0);
	GrB_Matrix_setElement_BOOL(A, true, 0, 1);

	// B
	// 0 1
	// 1 1
	GrB_Matrix_new(&B, GrB_BOOL, 2, 2);
	GrB_Matrix_setElement_BOOL(B, true, 0, 1);
	GrB_Matrix_setElement_BOOL(B, true, 1, 0);
	GrB_Matrix_setElement_BOOL(B, true, 1, 1);

	// C
	// 1 1
	// 1 1
	GrB_Matrix_new(&C, GrB_BOOL, 2, 2);
	GrB_Matrix_setElement_BOOL(C, true, 0, 0);
	GrB_Matrix_setElement_BOOL(C, true, 0, 1);
	GrB_Matrix_setElement_BOOL(C, true, 1, 0);
	GrB_Matrix_setElement_BOOL(C, true, 1, 1);

	// Matrix used for intermidate computations of AlgebraicExpression_Eval
	// but also contains the result of expression evaluation.
	GrB_Matrix_new(&res, GrB_BOOL, 2, 2);

	// A + B
	AlgebraicExpressionNode *exp = AlgebraicExpressionNode_NewOperationNode(AL_EXP_ADD);
	AlgebraicExpressionNode *a = AlgebraicExpressionNode_NewOperandNode(A);
	AlgebraicExpressionNode *b = AlgebraicExpressionNode_NewOperandNode(B);
	AlgebraicExpressionNode_AppendLeftChild(exp, a);
	AlgebraicExpressionNode_AppendRightChild(exp, b);

	AlgebraicExpression_SumOfMul(&exp);
	AlgebraicExpression_Eval(exp, res);

	// Using the A matrix described above,
	// A + B = C.
	ASSERT_TRUE(_compare_matrices(res, C));

	GrB_Matrix_free(&A);
	GrB_Matrix_free(&B);
	GrB_Matrix_free(&C);
	GrB_Matrix_free(&res);
	AlgebraicExpressionNode_Free(exp);
}

TEST_F(AlgebraicExpressionTest, Exp_OP_MUL) {
	// Exp = A * I
	GrB_Matrix A;
	GrB_Matrix I;

	// A
	// 1 1
	// 0 0
	GrB_Matrix_new(&A, GrB_BOOL, 2, 2);
	GrB_Matrix_setElement_BOOL(A, true, 0, 0);
	GrB_Matrix_setElement_BOOL(A, true, 0, 1);

	// I
	// 1 0
	// 0 1
	GrB_Matrix_new(&I, GrB_BOOL, 2, 2);
	GrB_Matrix_setElement_BOOL(I, true, 0, 0);
	GrB_Matrix_setElement_BOOL(I, true, 1, 1);
	GrB_Matrix res;

	// Matrix used for intermidate computations of AlgebraicExpression_Eval
	// but also contains the result of expression evaluation.
	GrB_Matrix_new(&res, GrB_BOOL, 2, 2);

	// A * I
	AlgebraicExpressionNode *exp = AlgebraicExpressionNode_NewOperationNode(AL_EXP_MUL);
	AlgebraicExpressionNode *a = AlgebraicExpressionNode_NewOperandNode(A);
	AlgebraicExpressionNode *i = AlgebraicExpressionNode_NewOperandNode(I);
	AlgebraicExpressionNode_AppendLeftChild(exp, a);
	AlgebraicExpressionNode_AppendRightChild(exp, i);

	AlgebraicExpression_SumOfMul(&exp);
	AlgebraicExpression_Eval(exp, res);

	// Using the A matrix described above,
	// A * I = A.
	ASSERT_TRUE(_compare_matrices(res, A));

	GrB_Matrix_free(&A);
	GrB_Matrix_free(&I);
	GrB_Matrix_free(&res);
	AlgebraicExpressionNode_Free(exp);
}

TEST_F(AlgebraicExpressionTest, Exp_OP_ADD_Transpose) {
	// Exp = A + Transpose(A)
	GrB_Matrix A;
	GrB_Matrix B;
	GrB_Matrix res;

	// A
	// 1 1
	// 0 0
	GrB_Matrix_new(&A, GrB_BOOL, 2, 2);
	GrB_Matrix_setElement_BOOL(A, true, 0, 0);
	GrB_Matrix_setElement_BOOL(A, true, 0, 1);

	// B
	// 1 0
	// 1 0
	GrB_Matrix_new(&B, GrB_BOOL, 2, 2);
	GrB_Matrix_setElement_BOOL(B, true, 0, 0);
	GrB_Matrix_setElement_BOOL(B, true, 1, 0);

	// Matrix used for intermidate computations of AlgebraicExpression_Eval
	// but also contains the result of expression evaluation.
	GrB_Matrix_new(&res, GrB_BOOL, 2, 2);

	// A + Transpose(A)
	AlgebraicExpressionNode *exp = AlgebraicExpressionNode_NewOperationNode(AL_EXP_ADD);
	AlgebraicExpressionNode *transpose = AlgebraicExpressionNode_NewOperationNode(AL_EXP_TRANSPOSE);
	AlgebraicExpressionNode *a = AlgebraicExpressionNode_NewOperandNode(A);
	AlgebraicExpressionNode *b = AlgebraicExpressionNode_NewOperandNode(B);

	AlgebraicExpressionNode_AppendLeftChild(exp, transpose);
	AlgebraicExpressionNode_AppendLeftChild(transpose, a);
	AlgebraicExpressionNode_AppendRightChild(exp, b);

	AlgebraicExpression_SumOfMul(&exp);
	AlgebraicExpression_Eval(exp, res);

	// Using the A matrix described above,
	// A + Transpose(A) = B.
	ASSERT_TRUE(_compare_matrices(res, B));

	GrB_Matrix_free(&A);
	GrB_Matrix_free(&B);
	GrB_Matrix_free(&res);
	AlgebraicExpressionNode_Free(exp);
}

TEST_F(AlgebraicExpressionTest, Exp_OP_MUL_Transpose) {
	// Exp = Transpose(A) * A
	GrB_Matrix A;
	GrB_Matrix B;
	GrB_Matrix res;

	// A
	// 1 1
	// 0 0
	GrB_Matrix_new(&A, GrB_BOOL, 2, 2);
	GrB_Matrix_setElement_BOOL(A, true, 0, 0);
	GrB_Matrix_setElement_BOOL(A, true, 0, 1);

	// B
	// 1 1
	// 1 1
	GrB_Matrix_new(&B, GrB_BOOL, 2, 2);
	GrB_Matrix_setElement_BOOL(B, true, 0, 0);
	GrB_Matrix_setElement_BOOL(B, true, 0, 1);
	GrB_Matrix_setElement_BOOL(B, true, 1, 0);
	GrB_Matrix_setElement_BOOL(B, true, 1, 1);

	// Matrix used for intermidate computations of AlgebraicExpression_Eval
	// but also contains the result of expression evaluation.
	GrB_Matrix_new(&res, GrB_BOOL, 2, 2);

	// Transpose(A) * A
	AlgebraicExpressionNode *exp = AlgebraicExpressionNode_NewOperationNode(AL_EXP_MUL);
	AlgebraicExpressionNode *transpose = AlgebraicExpressionNode_NewOperationNode(AL_EXP_TRANSPOSE);
	AlgebraicExpressionNode *a = AlgebraicExpressionNode_NewOperandNode(A);

	AlgebraicExpressionNode_AppendLeftChild(exp, transpose);
	AlgebraicExpressionNode_AppendRightChild(transpose, a);
	AlgebraicExpressionNode_AppendRightChild(exp, a);

	AlgebraicExpression_SumOfMul(&exp);
	AlgebraicExpression_Eval(exp, res);

	// Using the A matrix described above,
	// Transpose(A) * A = B.
	ASSERT_TRUE(_compare_matrices(res, B));

	GrB_Matrix_free(&A);
	GrB_Matrix_free(&B);
	GrB_Matrix_free(&res);
	AlgebraicExpressionNode_Free(exp);
}

TEST_F(AlgebraicExpressionTest, Exp_OP_A_MUL_B_Plus_C) {
	// Exp = A*(B+C)
	GrB_Matrix A;
	GrB_Matrix B;
	GrB_Matrix C;
	GrB_Matrix res;

	// A
	// 1 1
	// 0 0
	GrB_Matrix_new(&A, GrB_BOOL, 2, 2);
	GrB_Matrix_setElement_BOOL(A, true, 0, 0);
	GrB_Matrix_setElement_BOOL(A, true, 0, 1);

	// B
	// 1 0
	// 0 0
	GrB_Matrix_new(&B, GrB_BOOL, 2, 2);
	GrB_Matrix_setElement_BOOL(B, true, 0, 0);

	// C
	// 0 0
	// 0 1
	GrB_Matrix_new(&C, GrB_BOOL, 2, 2);
	GrB_Matrix_setElement_BOOL(C, true, 1, 1);

	// Matrix used for intermidate computations of AlgebraicExpression_Eval
	// but also contains the result of expression evaluation.
	GrB_Matrix_new(&res, GrB_BOOL, 2, 2);

	// A * (B+C) = A.
	AlgebraicExpressionNode *exp = AlgebraicExpressionNode_NewOperationNode(AL_EXP_MUL);
	AlgebraicExpressionNode *add = AlgebraicExpressionNode_NewOperationNode(AL_EXP_ADD);
	AlgebraicExpressionNode *a = AlgebraicExpressionNode_NewOperandNode(A);
	AlgebraicExpressionNode *b = AlgebraicExpressionNode_NewOperandNode(B);
	AlgebraicExpressionNode *c = AlgebraicExpressionNode_NewOperandNode(C);

	AlgebraicExpressionNode_AppendLeftChild(exp, a);
	AlgebraicExpressionNode_AppendRightChild(exp, add);
	AlgebraicExpressionNode_AppendLeftChild(add, b);
	AlgebraicExpressionNode_AppendRightChild(add, c);

	// AB + AC.
	AlgebraicExpression_SumOfMul(&exp);
	AlgebraicExpression_Eval(exp, res);
	ASSERT_TRUE(_compare_matrices(res, A));

	GrB_Matrix_free(&A);
	GrB_Matrix_free(&B);
	GrB_Matrix_free(&C);
	GrB_Matrix_free(&res);
	AlgebraicExpressionNode_Free(exp);
}

TEST_F(AlgebraicExpressionTest, ExpTransform_A_Times_B_Plus_C) {
	// Test Mul / Add transformation:
	// A*(B+C) -> A*B + A*C
	AlgebraicExpressionNode *root = AlgebraicExpressionNode_NewOperationNode(AL_EXP_MUL);
	AlgebraicExpressionNode *add = AlgebraicExpressionNode_NewOperationNode(AL_EXP_ADD);

	GrB_Matrix A;
	GrB_Matrix B;
	GrB_Matrix C;

	GrB_Matrix_new(&A, GrB_BOOL, 2, 2);
	GrB_Matrix_new(&B, GrB_BOOL, 2, 2);
	GrB_Matrix_new(&C, GrB_BOOL, 2, 2);

	AlgebraicExpressionNode *aExp = AlgebraicExpressionNode_NewOperandNode(A);
	AlgebraicExpressionNode *bExp = AlgebraicExpressionNode_NewOperandNode(B);
	AlgebraicExpressionNode *cExp = AlgebraicExpressionNode_NewOperandNode(C);

	// A*(B+C)
	AlgebraicExpressionNode_AppendLeftChild(root, aExp);
	AlgebraicExpressionNode_AppendRightChild(root, add);
	AlgebraicExpressionNode_AppendLeftChild(add, bExp);
	AlgebraicExpressionNode_AppendRightChild(add, cExp);

	AlgebraicExpression_SumOfMul(&root);

	// Verifications
	// (A*B)+(A*C)
	ASSERT_TRUE(root->type == AL_OPERATION && root->operation.op == AL_EXP_ADD);

	AlgebraicExpressionNode *rootLeftChild = root->operation.l;
	AlgebraicExpressionNode *rootRightChild = root->operation.r;
	ASSERT_TRUE(rootLeftChild && rootLeftChild->type == AL_OPERATION &&
				rootLeftChild->operation.op == AL_EXP_MUL);
	ASSERT_TRUE(rootRightChild && rootRightChild->type == AL_OPERATION &&
				rootRightChild->operation.op == AL_EXP_MUL);

	AlgebraicExpressionNode *leftLeft = rootLeftChild->operation.l;
	ASSERT_TRUE(leftLeft->type == AL_OPERAND && leftLeft->operand == A);

	AlgebraicExpressionNode *leftRight = rootLeftChild->operation.r;
	ASSERT_TRUE(leftRight->type == AL_OPERAND && leftRight->operand == B);

	AlgebraicExpressionNode *rightLeft = rootRightChild->operation.l;
	ASSERT_TRUE(rightLeft->type == AL_OPERAND && rightLeft->operand == A);

	AlgebraicExpressionNode *rightRight = rootRightChild->operation.r;
	ASSERT_TRUE(rightRight->type == AL_OPERAND && rightRight->operand == C);

	GrB_Matrix_free(&A);
	GrB_Matrix_free(&B);
	GrB_Matrix_free(&C);
	AlgebraicExpressionNode_Free(root);
}

TEST_F(AlgebraicExpressionTest, ExpTransform_AB_Times_C_Plus_D) {
	// Test Mul / Add transformation:
	// A*B*(C+D) -> A*B*C + A*B*D
	AlgebraicExpressionNode *root = AlgebraicExpressionNode_NewOperationNode(AL_EXP_MUL);
	AlgebraicExpressionNode *mul = AlgebraicExpressionNode_NewOperationNode(AL_EXP_MUL);
	AlgebraicExpressionNode *add = AlgebraicExpressionNode_NewOperationNode(AL_EXP_ADD);

	GrB_Matrix A;
	GrB_Matrix B;
	GrB_Matrix C;
	GrB_Matrix D;

	GrB_Matrix_new(&A, GrB_BOOL, 2, 2);
	GrB_Matrix_new(&B, GrB_BOOL, 2, 2);
	GrB_Matrix_new(&C, GrB_BOOL, 2, 2);
	GrB_Matrix_new(&D, GrB_BOOL, 2, 2);

	AlgebraicExpressionNode *aExp = AlgebraicExpressionNode_NewOperandNode(A);
	AlgebraicExpressionNode *bExp = AlgebraicExpressionNode_NewOperandNode(B);
	AlgebraicExpressionNode *cExp = AlgebraicExpressionNode_NewOperandNode(C);
	AlgebraicExpressionNode *dExp = AlgebraicExpressionNode_NewOperandNode(D);

	// A*B*(C+D)
	AlgebraicExpressionNode_AppendLeftChild(root, mul);
	AlgebraicExpressionNode_AppendRightChild(root, add);

	AlgebraicExpressionNode_AppendLeftChild(mul, aExp);
	AlgebraicExpressionNode_AppendRightChild(mul, bExp);

	AlgebraicExpressionNode_AppendLeftChild(add, cExp);
	AlgebraicExpressionNode_AppendRightChild(add, dExp);

	AlgebraicExpression_SumOfMul(&root);

	// Verifications
	// (A*B*C)+(A*B*D)
	ASSERT_TRUE(root->type == AL_OPERATION && root->operation.op == AL_EXP_ADD);

	AlgebraicExpressionNode *rootLeftChild = root->operation.l;
	ASSERT_TRUE(rootLeftChild && rootLeftChild->type == AL_OPERATION &&
				rootLeftChild->operation.op == AL_EXP_MUL);

	AlgebraicExpressionNode *rootRightChild = root->operation.r;
	ASSERT_TRUE(rootRightChild && rootRightChild->type == AL_OPERATION &&
				rootRightChild->operation.op == AL_EXP_MUL);

	AlgebraicExpressionNode *leftLeft = rootLeftChild->operation.l;
	ASSERT_TRUE(leftLeft->type == AL_OPERATION && leftLeft->operation.op == AL_EXP_MUL &&
				leftLeft->operation.reusable == true);

	AlgebraicExpressionNode *leftLeftLeft = leftLeft->operation.l;
	ASSERT_TRUE(leftLeftLeft->type == AL_OPERAND && leftLeftLeft->operand == A);

	AlgebraicExpressionNode *leftLeftRight = leftLeft->operation.r;
	ASSERT_TRUE(leftLeftRight->type == AL_OPERAND && leftLeftRight->operand == B);

	AlgebraicExpressionNode *leftRight = rootLeftChild->operation.r;
	ASSERT_TRUE(leftRight->type == AL_OPERAND && leftRight->operand == C);

	AlgebraicExpressionNode *rightLeft = rootRightChild->operation.l;
	ASSERT_TRUE(rightLeft->type == AL_OPERATION && rightLeft->operation.op == AL_EXP_MUL &&
				rightLeft->operation.reusable == true);

	AlgebraicExpressionNode *rightLeftLeft = rightLeft->operation.l;
	ASSERT_TRUE(rightLeftLeft->type == AL_OPERAND && rightLeftLeft->operand == A);

	AlgebraicExpressionNode *rightLeftRight = rightLeft->operation.r;
	ASSERT_TRUE(rightLeftRight->type == AL_OPERAND && rightLeftRight->operand == B);

	AlgebraicExpressionNode *rightRight = rootRightChild->operation.r;
	ASSERT_TRUE(rightRight->type == AL_OPERAND && rightRight->operand == D);

	GrB_Matrix_free(&A);
	GrB_Matrix_free(&B);
	GrB_Matrix_free(&C);
	GrB_Matrix_free(&D);
	AlgebraicExpressionNode_Free(root);
}

TEST_F(AlgebraicExpressionTest, MultipleIntermidateReturnNodes) {
	size_t exp_count = 0;
	const char *q = query_multiple_intermidate_return_nodes;
	AlgebraicExpression **actual = build_algebraic_expression(q, &exp_count);
	ASSERT_EQ(exp_count, 3);

	AlgebraicExpression **expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 3);
	AlgebraicExpression *exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
	expected[0] = exp;

	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_ev, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
	expected[1] = exp;

	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
	expected[2] = exp;

	compare_algebraic_expressions(actual, expected, 3);

	// Clean up.
	free_algebraic_expressions(actual, exp_count);
	free_algebraic_expressions(expected, exp_count);
	free(expected);
	free(actual);
}

TEST_F(AlgebraicExpressionTest, OneIntermidateReturnNode) {
	size_t exp_count = 0;
	const char *q = query_one_intermidate_return_nodes;
	AlgebraicExpression **actual = build_algebraic_expression(q, &exp_count);
	ASSERT_EQ(exp_count, 2);

	AlgebraicExpression **expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 2);
	AlgebraicExpression *exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ev, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
	expected[0] = exp;

	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
	expected[1] = exp;

	compare_algebraic_expressions(actual, expected, 2);

	// Clean up.
	free_algebraic_expressions(actual, exp_count);
	free_algebraic_expressions(expected, exp_count);
	free(expected);
	free(actual);
}

TEST_F(AlgebraicExpressionTest, NoIntermidateReturnNodes) {
	size_t exp_count = 0;
	const char *q = query_no_intermidate_return_nodes;
	AlgebraicExpression **actual = build_algebraic_expression(q, &exp_count);
	ASSERT_EQ(exp_count, 1);

	AlgebraicExpression **expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 1);
	AlgebraicExpression *exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ev, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
	expected[0] = exp;

	compare_algebraic_expressions(actual, expected, 1);

	// Clean up.
	free_algebraic_expressions(actual, exp_count);
	free_algebraic_expressions(expected, exp_count);
	free(expected);
	free(actual);
}

TEST_F(AlgebraicExpressionTest, OneIntermidateReturnEdge) {
	size_t exp_count;
	const char *q;
	AlgebraicExpression **actual;

	//==============================================================================================
	//=== MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN ef
	//==============================================================================================

	q = query_return_first_edge;
	actual = build_algebraic_expression(q, &exp_count);
	ASSERT_EQ(exp_count, 2);

	AlgebraicExpression **expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 2);
	AlgebraicExpression *exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
	expected[0] = exp;

	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_ev, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
	expected[1] = exp;

	compare_algebraic_expressions(actual, expected, 2);

	// Clean up.
	free_algebraic_expressions(actual, exp_count);
	free_algebraic_expressions(expected, exp_count);
	free(expected);
	free(actual);

	//==============================================================================================
	//=== MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN ev
	//==============================================================================================
	q = query_return_intermidate_edge;
	actual = build_algebraic_expression(q, &exp_count);
	ASSERT_EQ(exp_count, 3);

	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 3);
	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
	expected[0] = exp;

	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_ev, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
	expected[1] = exp;

	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
	expected[2] = exp;

	compare_algebraic_expressions(actual, expected, 3);

	// Clean up.
	free_algebraic_expressions(actual, exp_count);
	free_algebraic_expressions(expected, exp_count);
	free(expected);
	free(actual);

	//==============================================================================================
	//=== MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN ew
	//==============================================================================================
	q = query_return_last_edge;
	actual = build_algebraic_expression(q, &exp_count);
	ASSERT_EQ(exp_count, 2);

	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 2);
	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ev, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
	expected[0] = exp;

	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
	expected[1] = exp;

	compare_algebraic_expressions(actual, expected, 2);

	// Clean up.
	free_algebraic_expressions(actual, exp_count);
	free_algebraic_expressions(expected, exp_count);
	free(expected);
	free(actual);
}

TEST_F(AlgebraicExpressionTest, BothDirections) {
	size_t exp_count = 0;
	const char *q =
		"MATCH (p:Person)-[ef:friend]->(f:Person)<-[ev:visit]-(c:City)-[ew:war]->(e:City) RETURN p,e";
	AlgebraicExpression **actual = build_algebraic_expression(q, &exp_count);
	ASSERT_EQ(exp_count, 1);

	AlgebraicExpression **expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 1);
	AlgebraicExpression *exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ev, true, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
	expected[0] = exp;

	compare_algebraic_expressions(actual, expected, 1);

	// Clean up.
	free_algebraic_expressions(actual, exp_count);
	free_algebraic_expressions(expected, exp_count);
	free(expected);
	free(actual);
}

TEST_F(AlgebraicExpressionTest, SingleNode) {
	size_t exp_count = 0;
	const char *q = "MATCH (p:Person) RETURN p";
	AlgebraicExpression **actual = build_algebraic_expression(q, &exp_count);
	ASSERT_EQ(exp_count, 0);

	AlgebraicExpression **expected = NULL;

	compare_algebraic_expressions(actual, expected, 0);

	// Clean up.
	free_algebraic_expressions(actual, exp_count);
	free(actual);
}

TEST_F(AlgebraicExpressionTest, ShareableEntity) {
	size_t exp_count = 0;
	const char *q =
		"MATCH (p:Person)-[ef:friend]->(f:Person) MATCH (f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN p,e";
	AlgebraicExpression **actual = build_algebraic_expression(q, &exp_count);
	ASSERT_EQ(exp_count, 1);

	AlgebraicExpression **expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 1);
	AlgebraicExpression *exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ev, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
	expected[0] = exp;

	compare_algebraic_expressions(actual, expected, 1);

	// Clean up.
	free_algebraic_expressions(actual, exp_count);
	free_algebraic_expressions(expected, exp_count);
	free(expected);
	free(actual);

	exp_count = 0;
	q = "MATCH (p:Person)-[ef:friend]->(f:Person) MATCH (f:Person)<-[ev:visit]-(c:City)<-[ew:war]-(e:City) RETURN p,e";
	actual = build_algebraic_expression(q, &exp_count);
	ASSERT_EQ(exp_count, 1);

	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 1);
	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ev, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, true, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);

	expected[0] = exp;

	compare_algebraic_expressions(actual, expected, 1);

	// Clean up.
	free_algebraic_expressions(actual, exp_count);
	free_algebraic_expressions(expected, exp_count);
	free(expected);
	free(actual);

	exp_count = 0;
	q = "MATCH (p:Person)-[ef:friend]->(f:Person) MATCH (f:Person)-[ev:visit]->(c:City) MATCH (c:City)-[ew:war]->(e:City) RETURN p,e";
	actual = build_algebraic_expression(q, &exp_count);
	ASSERT_EQ(exp_count, 1);

	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 1);
	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ev, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
	expected[0] = exp;

	compare_algebraic_expressions(actual, expected, 1);

	// Clean up.
	free_algebraic_expressions(actual, exp_count);
	free_algebraic_expressions(expected, exp_count);
	free(expected);
	free(actual);

	exp_count = 0;
	q = "MATCH (a:Person)-[:friend]->(f:Person) MATCH (b:Person)-[:friend]->(f:Person) RETURN a,b";
	actual = build_algebraic_expression(q, &exp_count);
	ASSERT_EQ(exp_count, 1);

	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 1);
	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, true, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	expected[0] = exp;

	compare_algebraic_expressions(actual, expected, 1);

	// Clean up.
	free_algebraic_expressions(actual, exp_count);
	free_algebraic_expressions(expected, exp_count);
	free(expected);
	free(actual);

	// High incoming degree.
	exp_count = 0;
	q = "MATCH (a:Person)-[:friend]->(d:Person) MATCH (b:Person)-[:friend]->(d:Person) MATCH (c:Person)-[:friend]->(d:Person) RETURN a";
	actual = build_algebraic_expression(q, &exp_count);
	ASSERT_EQ(exp_count, 3);

	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 3);
	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	expected[0] = exp;

	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_ef, true, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	expected[1] = exp;

	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	expected[2] = exp;

	compare_algebraic_expressions(actual, expected, 3);

	// Clean up.
	free_algebraic_expressions(actual, exp_count);
	free_algebraic_expressions(expected, exp_count);
	free(expected);
	free(actual);

	// High outgoing degree.
	exp_count = 0;
	q = "MATCH (a:Person)-[:friend]->(b:Person) MATCH (a:Person)-[:friend]->(c:Person) MATCH (a:Person)-[:friend]->(d:Person) RETURN a";
	actual = build_algebraic_expression(q, &exp_count);
	ASSERT_EQ(exp_count, 3);

	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 3);
	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, true, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	expected[0] = exp;

	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	expected[1] = exp;

	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	expected[2] = exp;

	compare_algebraic_expressions(actual, expected, 3);

	// Clean up.
	free_algebraic_expressions(actual, exp_count);
	free_algebraic_expressions(expected, exp_count);
	free(expected);
	free(actual);

	// Cycle.
	exp_count = 0;
	q = "MATCH (a:Person)-[:friend]->(b:Person)-[:friend]->(a:Person) RETURN a";
	actual = build_algebraic_expression(q, &exp_count);
	ASSERT_EQ(exp_count, 2);

	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 2);
	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	expected[0] = exp;

	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	expected[1] = exp;

	compare_algebraic_expressions(actual, expected, 2);

	// Clean up.
	free_algebraic_expressions(actual, exp_count);
	free_algebraic_expressions(expected, exp_count);
	free(expected);
	free(actual);

	// Longer cycle.
	exp_count = 0;
	q = "MATCH (a:Person)-[:friend]->(b:Person)-[:friend]->(c:Person)-[:friend]->(a:Person) RETURN a";
	actual = build_algebraic_expression(q, &exp_count);
	ASSERT_EQ(exp_count, 2);

	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 2);
	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	expected[0] = exp;

	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	expected[1] = exp;

	compare_algebraic_expressions(actual, expected, 2);

	// Clean up.
	free_algebraic_expressions(actual, exp_count);
	free_algebraic_expressions(expected, exp_count);
	free(expected);
	free(actual);

	// Self pointing node.
	exp_count = 0;
	q = "MATCH (a:Person)-[:friend]->(a) RETURN a";
	actual = build_algebraic_expression(q, &exp_count);
	ASSERT_EQ(exp_count, 1);

	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 1);
	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	expected[0] = exp;

	compare_algebraic_expressions(actual, expected, 1);

	// Clean up.
	free_algebraic_expressions(actual, exp_count);
	free_algebraic_expressions(expected, exp_count);
	free(expected);
	free(actual);
}

TEST_F(AlgebraicExpressionTest, VariableLength) {
	size_t exp_count = 0;
	const char *q =
		"MATCH (p:Person)-[ef:friend]->(f:Person)-[:visit*1..3]->(c:City)-[ew:war]->(e:City) RETURN p,e";
	AlgebraicExpression **actual = build_algebraic_expression(q, &exp_count);
	ASSERT_EQ(exp_count, 3);

	AlgebraicExpression **expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 3);
	AlgebraicExpression *exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
	expected[0] = exp;

	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_ev, false, false, false);
	expected[1] = exp;

	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
	expected[2] = exp;

	compare_algebraic_expressions(actual, expected, 3);

	// Clean up.
	free_algebraic_expressions(actual, exp_count);
	free_algebraic_expressions(expected, exp_count);
	free(expected);
	free(actual);

	// Transposed variable length.
	exp_count = 0;
	q = "MATCH (p:Person)-[ef:friend]->(f:Person)<-[:visit*1..3]-(c:City)-[ew:war]->(e:City) RETURN p,e";
	actual = build_algebraic_expression(q, &exp_count);
	ASSERT_EQ(exp_count, 3);

	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 3);
	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
	expected[0] = exp;

	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_ev, true, false, false);
	expected[1] = exp;

	exp = AlgebraicExpression_Empty();
	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
	expected[2] = exp;

	compare_algebraic_expressions(actual, expected, 3);

	// Clean up.
	free_algebraic_expressions(actual, exp_count);
	free_algebraic_expressions(expected, exp_count);
	free(expected);
	free(actual);
}

TEST_F(AlgebraicExpressionTest, ExpressionExecute) {
	size_t exp_count = 0;
	GraphContext *gc = GraphContext_GetFromTLS();
	Graph *g = gc->g;

	const char *q = query_no_intermidate_return_nodes;
	AlgebraicExpression **ae = build_algebraic_expression(q, &exp_count);

	GrB_Matrix res;
	GrB_Matrix_new(&res, GrB_BOOL, Graph_RequiredMatrixDim(g), Graph_RequiredMatrixDim(g));

	AlgebraicExpression *exp = ae[0];
	AlgebraicExpression_Execute(exp, res);

	ASSERT_STREQ(exp->src_node->alias, "p");
	ASSERT_STREQ(exp->dest_node->alias, "e");

	// Validate result matrix.
	GrB_Index ncols, nrows;
	GrB_Matrix_ncols(&ncols, res);
	GrB_Matrix_nrows(&nrows, res);
	assert(ncols == Graph_RequiredMatrixDim(g));
	assert(nrows == Graph_RequiredMatrixDim(g));

	GrB_Index expected_entries[6] = {1, 2, 0, 3, 1, 3};
	GrB_Matrix expected = NULL;

	GrB_Matrix_dup(&expected, res);
	GrB_Matrix_clear(expected);
	for(int i = 0; i < 6; i += 2) {
		GrB_Matrix_setElement_BOOL(expected, true, expected_entries[i], expected_entries[i + 1]);
	}

	assert(_compare_matrices(res, expected));

	// Clean up
	AlgebraicExpression_Free(ae[0]);
	free(ae);
	GrB_Matrix_free(&expected);
	GrB_Matrix_free(&res);
}
