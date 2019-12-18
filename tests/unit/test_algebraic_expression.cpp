/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "assert.h"
#include "../../src/value.h"
#include "../../src/util/arr.h"
#include "../../src/query_ctx.h"
#include "../../src/graph/graph.h"
#include "../../src/util/rmalloc.h"
#include "../../src/graph/query_graph.h"
#include "../../src/graph/graphcontext.h"
#include "../../src/util/simple_timer.h"
#include "../../src/execution_plan/execution_plan.h"
#include "../../src/arithmetic/algebraic_expression.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"

extern AR_ExpNode **_BuildReturnExpressions(const cypher_astnode_t *ret_clause, AST *ast);

#ifdef __cplusplus
}
#endif

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
	}

	static void TearDownTestCase() {
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

		ASSERT_TRUE(QueryCtx_Init());
		QueryCtx_SetGraphCtx(gc);
	}

	/* Create a graph containing:
	 * Entities: 'people' and 'countries'.
	 * Relations: 'friend', 'visit' and 'war'. */
	static void _build_graph() {
		GraphContext *gc = QueryCtx_GetGraphCtx();

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
		GraphContext *gc = QueryCtx_GetGraphCtx();
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

	AlgebraicExpression **build_algebraic_expression(const char *query, uint *exp_count) {
		GraphContext *gc = QueryCtx_GetGraphCtx();
		cypher_parse_result_t *parse_result = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
		AST *master_ast = AST_Build(parse_result);
		AST *ast = AST_NewSegment(master_ast, 0, cypher_ast_query_nclauses(master_ast->root));
		QueryGraph *qg = BuildQueryGraph(gc, ast);
		AlgebraicExpression **ae = AlgebraicExpression_FromQueryGraph(qg, exp_count);

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

	void _compare_algebraic_operand(AlgebraicExpression *a, AlgebraicExpression *b) {
        ASSERT_TRUE(a->type == AL_OPERAND);
        ASSERT_TRUE(b->type == AL_OPERAND);
		ASSERT_EQ(a->operand.free, b->operand.free);
		ASSERT_EQ(a->operand.matrix, b->operand.matrix);
		ASSERT_EQ(a->operand.diagonal, b->operand.diagonal);
	}

	void compare_algebraic_expression(AlgebraicExpression *a, AlgebraicExpression *b) {
		ASSERT_EQ(a->type, b->type);
		ASSERT_EQ(AlgebraicExpression_ChildCount(a), AlgebraicExpression_ChildCount(b));
		ASSERT_EQ(AlgebraicExpression_OperandCount(a), AlgebraicExpression_OperandCount(b));
		// ASSERT_EQ(AlgebraicExpression_OperationCount(a, AL_EXP_ALL), AlgebraicExpression_OperationCount(b, AL_EXP_ALL));
        if(a->type == AL_OPERAND) _compare_algebraic_operand(a, b);

        uint child_count = AlgebraicExpression_ChildCount(a);
        for(uint i = 0; i < child_count; i++) {
			compare_algebraic_expression(a->operation.children[i], b->operation.children[i]);
		}
	}

	void compare_algebraic_expressions(AlgebraicExpression **actual, AlgebraicExpression **expected,
									   uint count) {
		for(uint i = 0; i < count; i++) {
			compare_algebraic_expression(actual[i], expected[i]);
		}
	}

	void free_algebraic_expressions(AlgebraicExpression **exps, uint count) {
		for(uint i = 0; i < count; i++) {
			AlgebraicExpression *exp = exps[i];
			AlgebraicExpression_Free(exp);
		}
	}

    AlgebraicExpression *algebraic_expression_from_string(const char *exp) {
        char *alias;
        uint exp_len = strlen(exp);
        AlgebraicExpression *root = NULL;
        AlgebraicExpression *op = NULL;
        AlgebraicExpression *rhs = NULL;

        for(uint i = 0; i < exp_len; i++) {
            char c = exp[i];
            switch(c) {
                case '+':
                    op = AlgebraicExpression_NewOperation(AL_EXP_ADD);
                    rhs = algebraic_expression_from_string(exp + (i + 1));
                    AlgebraicExpression_AddChild(op, root);
                    AlgebraicExpression_AddChild(op, rhs);
                    root = op;
                break;
                case '*':
                    op = AlgebraicExpression_NewOperation(AL_EXP_MUL);
                    rhs = algebraic_expression_from_string(exp + (i + 1));
                    AlgebraicExpression_AddChild(op, root);
                    AlgebraicExpression_AddChild(op, rhs);
                    root = op;
                break;
                case 'T':
                case '(':
                    // Wait for closing ')'
                    break;
                case ')':
                    AlgebraicExpression_Transpose(root);
                break;
                default:
                    alias = (char*)malloc(sizeof(char) * 2);
                    alias[0] = c;
                    alias[1] = '\0';
                    root = AlgebraicExpression_NewOperand(GrB_NULL, false, false, alias, alias, NULL);
                break;
            }
        }

        AlgebraicExpression_Optimize(&root);
        return root;
    }
};

TEST_F(AlgebraicExpressionTest, AlgebraicExpression_New) {
    GrB_Matrix matrix = GrB_NULL;
    bool free = false;
    bool diagonal = false;
    const char *src = "src";
    const char *dest = "dest";
    const char *edge = "edge";

    AlgebraicExpression *operand = AlgebraicExpression_NewOperand(matrix, free, diagonal, src, dest, edge);
    ASSERT_EQ(operand->type, AL_OPERAND);
    ASSERT_EQ(operand->operand.matrix, matrix);
    ASSERT_EQ(operand->operand.free, free);
    ASSERT_EQ(operand->operand.diagonal, diagonal);
    ASSERT_EQ(operand->operand.src, src);
    ASSERT_EQ(operand->operand.dest, dest);
    ASSERT_EQ(operand->operand.edge, edge);

    ASSERT_EQ(AlgebraicExpression_Source(operand), src);
    ASSERT_EQ(AlgebraicExpression_Destination(operand), dest);
    ASSERT_EQ(AlgebraicExpression_Edge(operand), edge);
    ASSERT_EQ(AlgebraicExpression_ChildCount(operand), 0);
    ASSERT_EQ(AlgebraicExpression_OperandCount(operand), 1);
    ASSERT_EQ(AlgebraicExpression_Transposed(operand), false);

    AL_EXP_OP op = AL_EXP_ADD;
    AlgebraicExpression *operation = AlgebraicExpression_NewOperation(op);
    AlgebraicExpression_AddChild(operation, operand);

    ASSERT_EQ(operation->type, AL_OPERATION);
    ASSERT_EQ(operation->operation.op, op);
    ASSERT_EQ(array_len(operation->operation.children), 1);

    ASSERT_EQ(AlgebraicExpression_Source(operation), src);
    ASSERT_EQ(AlgebraicExpression_Destination(operation), dest);
    ASSERT_EQ(AlgebraicExpression_Edge(operation), edge);
    ASSERT_EQ(AlgebraicExpression_ChildCount(operation), 1);
    ASSERT_EQ(AlgebraicExpression_OperandCount(operation), 1);
    ASSERT_EQ(AlgebraicExpression_Transposed(operation), false);

    // Will free `operand` aswell.
    AlgebraicExpression_Free(operation);
}

TEST_F(AlgebraicExpressionTest, AlgebraicExpression_Clone) {
    AlgebraicExpression *exp = NULL;
    AlgebraicExpression *clone = NULL;

    const char *expressions[13] = {"A", "A*B", "A*B*C", "A+B", "A+B+C", "A*B+C", "A+B*C",
    "T(A)", "T(A*B)", "T(A*B*C)", "T(A+B)", "T(A+B+C)", "T(T(A))"};

    for(uint i = 0; i < 13; i++) {        
        exp = algebraic_expression_from_string(expressions[i]);
        clone = AlgebraicExpression_Clone(exp);
        compare_algebraic_expression(exp, clone);
        AlgebraicExpression_Free(clone);
        AlgebraicExpression_Free(exp);
    }
}

TEST_F(AlgebraicExpressionTest, AlgebraicExpression_Transpose) {
    AlgebraicExpression *exp = NULL;
    AlgebraicExpression *transposed_exp = NULL;

    const char *expressions[6] = {"A", "A*B", "A*B*C", "A+B", "A+B+C", "T(A)"};
    const char *transposed_expressions[6] = {"T(A)", "T(B)*T(A)", "T(C)*T(B)*T(A)", "T(A)+T(B)", "T(A)+T(B)+T(C)", "A"};

    for(uint i = 0; i < 6; i++) {
        exp = algebraic_expression_from_string(expressions[i]);
        AlgebraicExpression_Transpose(exp);
        transposed_exp = algebraic_expression_from_string(transposed_expressions[i]);
        compare_algebraic_expression(exp, transposed_exp);
        AlgebraicExpression_Free(exp);
        AlgebraicExpression_Free(transposed_exp);
    }
}

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
	AlgebraicExpression *exp = AlgebraicExpression_NewOperand(A, false, false, NULL, NULL, NULL);
	AlgebraicExpression_AddToTheRight(&exp, B);
	AlgebraicExpression_Optimize(&exp);
	AlgebraicExpression_Eval(exp, res);

	// Using the A matrix described above,
	// A + B = C.
	ASSERT_TRUE(_compare_matrices(res, C));

	GrB_Matrix_free(&A);
	GrB_Matrix_free(&B);
	GrB_Matrix_free(&C);
	GrB_Matrix_free(&res);
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
	AlgebraicExpression *exp = AlgebraicExpression_NewOperand(A, false, false, NULL, NULL, NULL);
	AlgebraicExpression_MultiplyToTheRight(&exp, I);
	AlgebraicExpression_Optimize(&exp);
	AlgebraicExpression_Eval(exp, res);

	// Using the A matrix described above,
	// A * I = A.
	ASSERT_TRUE(_compare_matrices(res, A));

	GrB_Matrix_free(&A);
	GrB_Matrix_free(&I);
	GrB_Matrix_free(&res);
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
	AlgebraicExpression *exp = AlgebraicExpression_NewOperand(A, false, false, NULL, NULL, NULL);
	AlgebraicExpression_Transpose(exp);
    AlgebraicExpression_AddToTheLeft(&exp, A);
    AlgebraicExpression_Optimize(&exp);
	AlgebraicExpression_Eval(exp, res);

	// Using the A matrix described above,
	// A + Transpose(A) = B.
	ASSERT_TRUE(_compare_matrices(res, B));

	GrB_Matrix_free(&A);
	GrB_Matrix_free(&B);
	GrB_Matrix_free(&res);
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
	AlgebraicExpression *exp = AlgebraicExpression_NewOperand(A, false, false, NULL, NULL, NULL);
    AlgebraicExpression_Transpose(exp);
    AlgebraicExpression_MultiplyToTheRight(&exp, A);
    AlgebraicExpression_Optimize(&exp);
	AlgebraicExpression_Eval(exp, res);

	// Using the A matrix described above,
	// Transpose(A) * A = B.
	ASSERT_TRUE(_compare_matrices(res, B));

	GrB_Matrix_free(&A);
	GrB_Matrix_free(&B);
	GrB_Matrix_free(&res);
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
	AlgebraicExpression *exp = AlgebraicExpression_NewOperand(B, false, false, NULL, NULL, NULL);
    AlgebraicExpression_AddToTheRight(&exp, C);
    AlgebraicExpression_MultiplyToTheLeft(&exp, A);
    // AB + AC.
	AlgebraicExpression_Optimize(&exp);
	AlgebraicExpression_Eval(exp, res);
	ASSERT_TRUE(_compare_matrices(res, A));

	GrB_Matrix_free(&A);
	GrB_Matrix_free(&B);
	GrB_Matrix_free(&C);
	GrB_Matrix_free(&res);
}

TEST_F(AlgebraicExpressionTest, ExpTransform_A_Times_B_Plus_C) {
	// Test Mul / Add transformation:
	// A*(B+C) -> A*B + A*C
	GrB_Matrix A;
	GrB_Matrix B;
	GrB_Matrix C;

	GrB_Matrix_new(&A, GrB_BOOL, 2, 2);
	GrB_Matrix_new(&B, GrB_BOOL, 2, 2);
	GrB_Matrix_new(&C, GrB_BOOL, 2, 2);

	AlgebraicExpression *exp = AlgebraicExpression_NewOperand(B, false, false, NULL, NULL, NULL);	

	// A*(B+C)
	AlgebraicExpression_AddToTheRight(&exp, C);
    AlgebraicExpression_MultiplyToTheLeft(&exp, A);
	AlgebraicExpression_Optimize(&exp);

	// Verifications
	// (A*B)+(A*C)
	ASSERT_TRUE(exp->type == AL_OPERATION && exp->operation.op == AL_EXP_ADD);

	AlgebraicExpression *rootLeftChild = exp->operation.children[0];
	AlgebraicExpression *rootRightChild = exp->operation.children[1];
    ASSERT_TRUE(rootLeftChild && rootLeftChild->type == AL_OPERATION &&
				rootLeftChild->operation.op == AL_EXP_MUL);
	ASSERT_TRUE(rootRightChild && rootRightChild->type == AL_OPERATION &&
				rootRightChild->operation.op == AL_EXP_MUL);

    AlgebraicExpression *leftLeft = rootLeftChild->operation.children[0];
	ASSERT_TRUE(leftLeft->type == AL_OPERAND && leftLeft->operand.matrix == A);

	AlgebraicExpression *leftRight = rootLeftChild->operation.children[1];
	ASSERT_TRUE(leftRight->type == AL_OPERAND && leftRight->operand.matrix == B);

	AlgebraicExpression *rightLeft = rootRightChild->operation.children[0];
	ASSERT_TRUE(rightLeft->type == AL_OPERAND && rightLeft->operand.matrix == A);

	AlgebraicExpression *rightRight = rootRightChild->operation.children[1];
	ASSERT_TRUE(rightRight->type == AL_OPERAND && rightRight->operand.matrix == C);

	GrB_Matrix_free(&A);
	GrB_Matrix_free(&B);
	GrB_Matrix_free(&C);
}

TEST_F(AlgebraicExpressionTest, ExpTransform_AB_Times_C_Plus_D) {
	// Test Mul / Add transformation:
	// A*B*(C+D) -> A*B*C + A*B*D
	GrB_Matrix A;
	GrB_Matrix B;
	GrB_Matrix C;
	GrB_Matrix D;

	GrB_Matrix_new(&A, GrB_BOOL, 2, 2);
	GrB_Matrix_new(&B, GrB_BOOL, 2, 2);
	GrB_Matrix_new(&C, GrB_BOOL, 2, 2);
	GrB_Matrix_new(&D, GrB_BOOL, 2, 2);

    // A*B*(C+D) -> A*B*C + A*B*D
	AlgebraicExpression *exp = AlgebraicExpression_NewOperand(C, false, false, NULL, NULL, NULL);	

	// A*B*(C+D)
	AlgebraicExpression_AddToTheRight(&exp, D);
    AlgebraicExpression_MultiplyToTheLeft(&exp, B);
    AlgebraicExpression_MultiplyToTheLeft(&exp, A);
	AlgebraicExpression_Optimize(&exp);

	// Verifications
	// (A*B*C)+(A*B*D)
	ASSERT_TRUE(exp->type == AL_OPERATION && exp->operation.op == AL_EXP_ADD);

	AlgebraicExpression *rootLeftChild = exp->operation.children[0];
	ASSERT_TRUE(rootLeftChild && rootLeftChild->type == AL_OPERATION &&
				rootLeftChild->operation.op == AL_EXP_MUL);

	AlgebraicExpression *rootRightChild = exp->operation.children[1];
	ASSERT_TRUE(rootRightChild && rootRightChild->type == AL_OPERATION &&
				rootRightChild->operation.op == AL_EXP_MUL);

	AlgebraicExpression *leftLeft = rootLeftChild->operation.children[0];
	ASSERT_TRUE(leftLeft->type == AL_OPERATION && leftLeft->operation.op == AL_EXP_MUL);

	AlgebraicExpression *leftLeftLeft = leftLeft->operation.children[0];
	ASSERT_TRUE(leftLeftLeft->type == AL_OPERAND && leftLeftLeft->operand.matrix == A);

	AlgebraicExpression *leftLeftRight = leftLeft->operation.children[1];
	ASSERT_TRUE(leftLeftRight->type == AL_OPERAND && leftLeftRight->operand.matrix == B);

	AlgebraicExpression *leftRight = rootLeftChild->operation.children[1];
	ASSERT_TRUE(leftRight->type == AL_OPERAND && leftRight->operand.matrix == C);

	AlgebraicExpression *rightLeft = rootRightChild->operation.children[0];
	ASSERT_TRUE(rightLeft->type == AL_OPERATION && rightLeft->operation.op == AL_EXP_MUL);

	AlgebraicExpression *rightLeftLeft = rightLeft->operation.children[0];
	ASSERT_TRUE(rightLeftLeft->type == AL_OPERAND && rightLeftLeft->operand.matrix == A);

	AlgebraicExpression *rightLeftRight = rightLeft->operation.children[1];
	ASSERT_TRUE(rightLeftRight->type == AL_OPERAND && rightLeftRight->operand.matrix == B);

	AlgebraicExpression *rightRight = rootRightChild->operation.children[1];
	ASSERT_TRUE(rightRight->type == AL_OPERAND && rightRight->operand.matrix == D);

	GrB_Matrix_free(&A);
	GrB_Matrix_free(&B);
	GrB_Matrix_free(&C);
	GrB_Matrix_free(&D);
}

// TEST_F(AlgebraicExpressionTest, MultipleIntermidateReturnNodes) {
// 	uint exp_count = 0;
// 	const char *q = query_multiple_intermidate_return_nodes;
// 	AlgebraicExpression **actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 3);

// 	AlgebraicExpression **expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 3);
// 	AlgebraicExpression *exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
// 	expected[0] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ev, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
// 	expected[1] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
// 	expected[2] = exp;

// 	compare_algebraic_expressions(actual, expected, 3);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);
// }

// TEST_F(AlgebraicExpressionTest, OneIntermidateReturnNode) {
// 	uint exp_count = 0;
// 	const char *q = query_one_intermidate_return_nodes;
// 	AlgebraicExpression **actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 2);

// 	AlgebraicExpression **expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 2);
// 	AlgebraicExpression *exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ev, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
// 	expected[0] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
// 	expected[1] = exp;

// 	compare_algebraic_expressions(actual, expected, 2);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);
// }

// TEST_F(AlgebraicExpressionTest, NoIntermidateReturnNodes) {
// 	uint exp_count = 0;
// 	const char *q = query_no_intermidate_return_nodes;
// 	AlgebraicExpression **actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 1);

// 	AlgebraicExpression **expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 1);
// 	AlgebraicExpression *exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ev, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
// 	expected[0] = exp;

// 	compare_algebraic_expressions(actual, expected, 1);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);
// }

// TEST_F(AlgebraicExpressionTest, OneIntermidateReturnEdge) {
// 	uint exp_count;
// 	const char *q;
// 	AlgebraicExpression **actual;

// 	//==============================================================================================
// 	//=== MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN ef
// 	//==============================================================================================

// 	q = query_return_first_edge;
// 	actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 2);

// 	AlgebraicExpression **expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 2);
// 	AlgebraicExpression *exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
// 	expected[0] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ev, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
// 	expected[1] = exp;

// 	compare_algebraic_expressions(actual, expected, 2);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);

// 	//==============================================================================================
// 	//=== MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN ev
// 	//==============================================================================================
// 	q = query_return_intermidate_edge;
// 	actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 3);

// 	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 3);
// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
// 	expected[0] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ev, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
// 	expected[1] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
// 	expected[2] = exp;

// 	compare_algebraic_expressions(actual, expected, 3);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);

// 	//==============================================================================================
// 	//=== MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN ew
// 	//==============================================================================================
// 	q = query_return_last_edge;
// 	actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 2);

// 	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 2);
// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ev, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
// 	expected[0] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
// 	expected[1] = exp;

// 	compare_algebraic_expressions(actual, expected, 2);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);
// }

// TEST_F(AlgebraicExpressionTest, BothDirections) {
// 	uint exp_count = 0;
// 	const char *q =
// 		"MATCH (p:Person)-[ef:friend]->(f:Person)<-[ev:visit]-(c:City)-[ew:war]->(e:City) RETURN p,e";
// 	AlgebraicExpression **actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 1);

// 	AlgebraicExpression **expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 1);
// 	AlgebraicExpression *exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ev, true, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
// 	expected[0] = exp;

// 	compare_algebraic_expressions(actual, expected, 1);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);
// }

// TEST_F(AlgebraicExpressionTest, SingleNode) {
// 	uint exp_count = 0;
// 	const char *q = "MATCH (p:Person) RETURN p";
// 	AlgebraicExpression **actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 0);

// 	AlgebraicExpression **expected = NULL;

// 	compare_algebraic_expressions(actual, expected, 0);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free(actual);
// }

// TEST_F(AlgebraicExpressionTest, ShareableEntity) {
// 	uint exp_count = 0;
// 	const char *q =
// 		"MATCH (p:Person)-[ef:friend]->(f:Person) MATCH (f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN p,e";
// 	AlgebraicExpression **actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 1);

// 	AlgebraicExpression **expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 1);
// 	AlgebraicExpression *exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ev, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
// 	expected[0] = exp;

// 	compare_algebraic_expressions(actual, expected, 1);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);

// 	exp_count = 0;
// 	q = "MATCH (p:Person)-[ef:friend]->(f:Person) MATCH (f:Person)<-[ev:visit]-(c:City)<-[ew:war]-(e:City) RETURN p,e";
// 	actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 1);

// 	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 1);
// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ev, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, true, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);

// 	expected[0] = exp;

// 	compare_algebraic_expressions(actual, expected, 1);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);

// 	exp_count = 0;
// 	q = "MATCH (p:Person)-[ef:friend]->(f:Person) MATCH (f:Person)-[ev:visit]->(c:City) MATCH (c:City)-[ew:war]->(e:City) RETURN p,e";
// 	actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 1);

// 	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 1);
// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ev, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
// 	expected[0] = exp;

// 	compare_algebraic_expressions(actual, expected, 1);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);

// 	exp_count = 0;
// 	q = "MATCH (a:Person)-[:friend]->(f:Person) MATCH (b:Person)-[:friend]->(f:Person) RETURN a,b";
// 	actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 1);

// 	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 1);
// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, true, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	expected[0] = exp;

// 	compare_algebraic_expressions(actual, expected, 1);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);

// 	// High incoming degree.
// 	exp_count = 0;
// 	q = "MATCH (a:Person)-[:friend]->(d:Person) MATCH (b:Person)-[:friend]->(d:Person) MATCH (c:Person)-[:friend]->(d:Person) RETURN a";
// 	actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 3);

// 	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 3);
// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	expected[0] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, true, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	expected[1] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	expected[2] = exp;

// 	compare_algebraic_expressions(actual, expected, 3);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);

// 	// High outgoing degree.
// 	exp_count = 0;
// 	q = "MATCH (a:Person)-[:friend]->(b:Person) MATCH (a:Person)-[:friend]->(c:Person) MATCH (a:Person)-[:friend]->(d:Person) RETURN a";
// 	actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 3);

// 	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 3);
// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, true, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	expected[0] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	expected[1] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	expected[2] = exp;

// 	compare_algebraic_expressions(actual, expected, 3);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);

// 	// Cycle.
// 	/* TODO: The algebraic expression here can be improved
// 	 * reducing from 2 expression into a single one
// 	 * mat_p * mat_ef * mat_p * mat_ef * mat_p
// 	 * see comment in AlgebraicExpression_FromQueryGraph regarding cycles. */
// 	exp_count = 0;
// 	q = "MATCH (a:Person)-[:friend]->(b:Person)-[:friend]->(a:Person) RETURN a";
// 	actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 1);

// 	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 1);
// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	expected[0] = exp;

// 	compare_algebraic_expressions(actual, expected, 1);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);

// 	// Longer cycle.
// 	exp_count = 0;
// 	q = "MATCH (a:Person)-[:friend]->(b:Person)-[:friend]->(c:Person)-[:friend]->(a:Person) RETURN a";
// 	actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 1);

// 	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 2);
// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	expected[0] = exp;

// 	compare_algebraic_expressions(actual, expected, 1);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);

// 	// Self pointing node.
// 	exp_count = 0;
// 	q = "MATCH (a:Person)-[:friend]->(a) RETURN a";
// 	actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 1);

// 	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 1);
// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	expected[0] = exp;

// 	compare_algebraic_expressions(actual, expected, 1);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);

// 	//(p1)-[]->(p2)-[]->(p3)-[]->(p2)-[]->(p4)-[]->(p5) RETURN p1
// 	exp_count = 0;
// 	q = "MATCH (p1)-[:friend]->(p2)-[:friend]->(p3)-[:friend]->(p2)-[:friend]->(p4)-[:friend]->(p5) RETURN p1";
// 	actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 3);

// 	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 3);
// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[0] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[1] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[2] = exp;

// 	compare_algebraic_expressions(actual, expected, 3);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);

// 	// (p1)-[]->(p2)-[]->(p3)-[]->(p2)-[]->(p4)-[]->(p5) RETURN p1,p2,p3,p4,p5
// 	exp_count = 0;
// 	q = "MATCH (p1)-[:friend]->(p2)-[:friend]->(p3)-[:friend]->(p2)-[:friend]->(p4)-[:friend]->(p5) RETURN p1,p2,p3,p4,p5";
// 	actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 5);

// 	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 5);
// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[0] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[1] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[2] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[3] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[4] = exp;

// 	compare_algebraic_expressions(actual, expected, 5);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);

// 	// MATCH (p1)-[]->(p2)-[]->(p3)-[]->(p4)-[]->(p5)-[]->(p2)-[]->(p6)-[]->(p7)-[]->(p3) RETURN p1
// 	exp_count = 0;
// 	q = "MATCH (p1)-[:friend]->(p2)-[:friend]->(p3)-[:friend]->(p4)-[:friend]->(p5)-[:friend]->(p2)-[:friend]->(p6)-[:friend]->(p7)-[:friend]->(p3) RETURN p1";
// 	actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 4);

// 	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 4);
// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[0] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[1] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[2] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[3] = exp;

// 	compare_algebraic_expressions(actual, expected, 4);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);

// 	// MATCH (p1)-[]->(p2)-[]->(p3)-[]->(p4)-[]->(p5)-[]->(p2)-[]->(p6)-[]->(p7)-[]->(p3) RETURN p1,p2,p3,p4,p5,p6,p7
// 	exp_count = 0;
// 	q = "MATCH (p1)-[:friend]->(p2)-[:friend]->(p3)-[:friend]->(p4)-[:friend]->(p5)-[:friend]->(p2)-[:friend]->(p6)-[:friend]->(p7)-[:friend]->(p3) RETURN p1,p2,p3,p4,p5,p6,p7";
// 	actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 8);

// 	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 8);
// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[0] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[1] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[2] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[3] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[4] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[5] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[6] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[7] = exp;

// 	compare_algebraic_expressions(actual, expected, 8);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);

// 	// MATCH (p1)-[]->(p2)-[]->(p3)-[]->(p4)-[]->(p1)-[]->(p4),(p4)-[]->(p5) RETURN p1
// 	exp_count = 0;
// 	q = "MATCH (p1)-[:friend]->(p2)-[:friend]->(p3)-[:friend]->(p4)-[:friend]->(p1)-[:friend]->(p4)-[:friend]->(p5) RETURN p1";
// 	actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 4);

// 	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 4);
// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, true, false, false);
// 	expected[0] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[1] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[2] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[3] = exp;

// 	compare_algebraic_expressions(actual, expected, 4);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);

// 	// MATCH (p1)-[]->(p2)-[]->(p3)-[]->(p4)-[]->(p1)-[]->(p4),(p4)-[]->(p5) RETURN p1,p2,p3,p4,p5
// 	exp_count = 0;
// 	q = "MATCH (p1)-[:friend]->(p2)-[:friend]->(p3)-[:friend]->(p4)-[:friend]->(p1)-[:friend]->(p4)-[:friend]->(p5) RETURN p1,p2,p3,p4,p5";
// 	actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 6);

// 	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 6);
// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, true, false, false);
// 	expected[0] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[1] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[2] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[3] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[4] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	expected[5] = exp;

// 	compare_algebraic_expressions(actual, expected, 6);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);
// }

// TEST_F(AlgebraicExpressionTest, VariableLength) {
// 	uint exp_count = 0;
// 	const char *q =
// 		"MATCH (p:Person)-[ef:friend]->(f:Person)-[:visit*1..3]->(c:City)-[ew:war]->(e:City) RETURN p,e";
// 	AlgebraicExpression **actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 3);

// 	AlgebraicExpression **expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 3);
// 	AlgebraicExpression *exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
// 	expected[0] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ev, false, false, false);
// 	expected[1] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
// 	expected[2] = exp;

// 	compare_algebraic_expressions(actual, expected, 3);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);

// 	// Transposed variable length.
// 	exp_count = 0;
// 	q = "MATCH (p:Person)-[ef:friend]->(f:Person)<-[:visit*1..3]-(c:City)-[ew:war]->(e:City) RETURN p,e";
// 	actual = build_algebraic_expression(q, &exp_count);
// 	ASSERT_EQ(exp_count, 3);

// 	expected = (AlgebraicExpression **)malloc(sizeof(AlgebraicExpression *) * 3);
// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_p, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ef, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_f, false, false, true);
// 	expected[0] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_ev, true, false, false);
// 	expected[1] = exp;

// 	exp = AlgebraicExpression_Empty();
// 	AlgebraicExpression_AppendTerm(exp, mat_c, false, false, true);
// 	AlgebraicExpression_AppendTerm(exp, mat_ew, false, false, false);
// 	AlgebraicExpression_AppendTerm(exp, mat_e, false, false, true);
// 	expected[2] = exp;

// 	compare_algebraic_expressions(actual, expected, 3);

// 	// Clean up.
// 	free_algebraic_expressions(actual, exp_count);
// 	free_algebraic_expressions(expected, exp_count);
// 	free(expected);
// 	free(actual);
// }

// TEST_F(AlgebraicExpressionTest, ExpressionExecute) {
// 	uint exp_count = 0;
// 	GraphContext *gc = QueryCtx_GetGraphCtx();
// 	Graph *g = gc->g;

// 	const char *q = query_no_intermidate_return_nodes;
// 	AlgebraicExpression **ae = build_algebraic_expression(q, &exp_count);

// 	GrB_Matrix res;
// 	GrB_Matrix_new(&res, GrB_BOOL, Graph_RequiredMatrixDim(g), Graph_RequiredMatrixDim(g));

// 	AlgebraicExpression *exp = ae[0];
// 	AlgebraicExpression_Execute(exp, res);

// 	ASSERT_STREQ(exp->src, "p");
// 	ASSERT_STREQ(exp->dest, "e");

// 	// Validate result matrix.
// 	GrB_Index ncols, nrows;
// 	GrB_Matrix_ncols(&ncols, res);
// 	GrB_Matrix_nrows(&nrows, res);
// 	assert(ncols == Graph_RequiredMatrixDim(g));
// 	assert(nrows == Graph_RequiredMatrixDim(g));

// 	GrB_Index expected_entries[6] = {1, 2, 0, 3, 1, 3};
// 	GrB_Matrix expected = NULL;

// 	GrB_Matrix_dup(&expected, res);
// 	GrB_Matrix_clear(expected);
// 	for(int i = 0; i < 6; i += 2) {
// 		GrB_Matrix_setElement_BOOL(expected, true, expected_entries[i], expected_entries[i + 1]);
// 	}

// 	assert(_compare_matrices(res, expected));

// 	// Clean up
// 	AlgebraicExpression_Free(ae[0]);
// 	free(ae);
// 	GrB_Matrix_free(&expected);
// 	GrB_Matrix_free(&res);
// }
