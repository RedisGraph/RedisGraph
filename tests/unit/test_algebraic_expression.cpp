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
#include "../../src/graph/graph.h"
#include "../../src/query_executor.h"
#include "../../src/graph/query_graph.h"
#include "../../src/util/simple_timer.h"
#include "../../src/arithmetic/algebraic_expression.h"
#include "../../src/util/rmalloc.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"

#ifdef __cplusplus
}
#endif

class AlgebraicExpressionTest: public ::testing::Test {
    protected:

    Graph *g;
    QueryGraph *query_graph;
    const char *query_no_intermidate_return_nodes;
    const char *query_one_intermidate_return_nodes;
    const char *query_multiple_intermidate_return_nodes;
    const char *query_return_first_edge;
    const char *query_return_intermidate_edge;
    const char *query_return_last_edge;

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

    void SetUp() {
        srand(time(NULL));
        // Create a graph
        g = _build_graph();

        // Create a graph describing the queries which follows
        query_graph = _build_query_graph(g);

        query_no_intermidate_return_nodes = "MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN p, e";
        query_one_intermidate_return_nodes = "MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN p, c, e";
        query_multiple_intermidate_return_nodes = "MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN p, f, c, e";

        query_return_first_edge = "MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN ef";
        query_return_intermidate_edge = "MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN ev";
        query_return_last_edge = "MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN ew";
    }

    void TearDown() {
        Graph_Free(g);
        QueryGraph_Free(query_graph);
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

        if (acols != bcols || arows != brows || avals != bvals) {
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

    /* Create a graph containing:
    * Entities: 'people' and 'countries'.
    * Relations: 'friend', 'visit' and 'war'. */
    Graph *_build_graph() {
        Graph *g = Graph_New(16, 16);
        Graph_AcquireWriteLock(g);
        size_t person_count = 2;
        size_t country_count = 2;
        size_t node_count = person_count + country_count;

        /* Introduce person and country labels. */
        int person_label = Graph_AddLabel(g);
        int country_label = Graph_AddLabel(g);
        Graph_AllocateNodes(g, node_count);

        for(int i = 0; i < person_count; i++) {
            Node n;
            Graph_CreateNode(g, person_label, &n);
        }

        for(int i = 0; i < country_count; i++) {
            Node n;
            Graph_CreateNode(g, country_label, &n);
        }

        // Creates a relation matrices.
        GrB_Index friend_relation_id = Graph_AddRelationType(g);
        GrB_Index visit_relation_id = Graph_AddRelationType(g);
        GrB_Index war_relation_id = Graph_AddRelationType(g);

        // Introduce relations, connect nodes.
        Edge e;
        Graph_ConnectNodes(g, 0, 1, friend_relation_id, &e);
        Graph_ConnectNodes(g, 1, 0, friend_relation_id, &e);

        Graph_ConnectNodes(g, 0, 2, visit_relation_id, &e);
        Graph_ConnectNodes(g, 0, 3, visit_relation_id, &e);
        Graph_ConnectNodes(g, 1, 2, visit_relation_id, &e);        

        Graph_ConnectNodes(g, 2, 3, war_relation_id, &e);
        Graph_ConnectNodes(g, 3, 2, war_relation_id, &e);
        return g;
    }

    QueryGraph *_build_query_graph(Graph *g) {
        /* Query
        * MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) */
        
        QueryGraph *q = QueryGraph_New(1, 1);
        // Create Nodes
        Node *p = Node_New("Person", "p");
        Node *f = Node_New("Person", "f");
        Node *c = Node_New("City", "c");
        Node *e = Node_New("City", "e");

        // Set node matrices according to the order they've been presented
        // during graph construction.
        p->mat = Graph_GetLabelMatrix(g, 0);
        f->mat = Graph_GetLabelMatrix(g, 0);
        c->mat = Graph_GetLabelMatrix(g, 1);
        e->mat = Graph_GetLabelMatrix(g, 1);

        // Create edges
        Edge *ef = Edge_New(p, f, "friend", "ef");
        Edge *ev = Edge_New(f, c, "visit", "ev");
        Edge *ew = Edge_New(c, e, "war", "ew");
        
        // Set edges matrices according to the order they've been presented
        // during graph construction.
        ef->mat = Graph_GetRelationMatrix(g, 0);
        ev->mat = Graph_GetRelationMatrix(g, 1);
        ew->mat = Graph_GetRelationMatrix(g, 2);

        // Construct query graph
        QueryGraph_AddNode(q, p);
        QueryGraph_AddNode(q, f);
        QueryGraph_AddNode(q, c);
        QueryGraph_AddNode(q, e);
        QueryGraph_ConnectNodes(q, p, f, ef);
        QueryGraph_ConnectNodes(q, f, c, ev);
        QueryGraph_ConnectNodes(q, c, e, ew);

        return q;
    }

    AlgebraicExpression **_build_algebraic_expression(const char *query, size_t *exp_count) {
        *exp_count = 0;
        AlgebraicExpression **ae = NULL;

        AST **ast = ParseQuery(query, strlen(query), NULL);
        ae = AlgebraicExpression_From_Query(
            ast[0],
            ast[0]->matchNode->_mergedPatterns,
            query_graph,
            exp_count);
        return ae;

        AST_Free(ast);
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
    ASSERT_TRUE(rootLeftChild && rootLeftChild->type == AL_OPERATION && rootLeftChild->operation.op == AL_EXP_MUL);
    ASSERT_TRUE(rootRightChild && rootRightChild->type == AL_OPERATION && rootRightChild->operation.op == AL_EXP_MUL);

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
    ASSERT_TRUE(rootLeftChild && rootLeftChild->type == AL_OPERATION && rootLeftChild->operation.op == AL_EXP_MUL);

    AlgebraicExpressionNode *rootRightChild = root->operation.r;
    ASSERT_TRUE(rootRightChild && rootRightChild->type == AL_OPERATION && rootRightChild->operation.op == AL_EXP_MUL);

    AlgebraicExpressionNode *leftLeft = rootLeftChild->operation.l;
    ASSERT_TRUE(leftLeft->type == AL_OPERATION && leftLeft->operation.op == AL_EXP_MUL && leftLeft->operation.reusable == true);

    AlgebraicExpressionNode *leftLeftLeft = leftLeft->operation.l;
    ASSERT_TRUE(leftLeftLeft->type == AL_OPERAND && leftLeftLeft->operand == A);

    AlgebraicExpressionNode *leftLeftRight = leftLeft->operation.r;
    ASSERT_TRUE(leftLeftRight->type == AL_OPERAND && leftLeftRight->operand == B);

    AlgebraicExpressionNode *leftRight = rootLeftChild->operation.r;
    ASSERT_TRUE(leftRight->type == AL_OPERAND && leftRight->operand == C);

    AlgebraicExpressionNode *rightLeft = rootRightChild->operation.l;
    ASSERT_TRUE(rightLeft->type == AL_OPERATION && rightLeft->operation.op == AL_EXP_MUL && rightLeft->operation.reusable == true);

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
    Node *n;
    Edge *e;
    size_t exp_count = 0;
    const char *query = query_multiple_intermidate_return_nodes;
    AlgebraicExpression **ae = _build_algebraic_expression(query, &exp_count);
    ASSERT_EQ(exp_count, 3);
    
    // Validate first expression.
    AlgebraicExpression *exp = ae[0];
    ASSERT_EQ(exp->op, AL_EXP_MUL);
    ASSERT_EQ(exp->operand_count, 3);

    n = QueryGraph_GetNodeByAlias(query_graph, "p");
    ASSERT_EQ(exp->operands[0].operand, n->mat);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ef");
    ASSERT_EQ(exp->operands[1].operand, e->mat);
    n = QueryGraph_GetNodeByAlias(query_graph, "f");
    ASSERT_EQ(exp->operands[2].operand, n->mat);

    // Validate second expression.
    exp = ae[1];
    ASSERT_EQ(exp->op, AL_EXP_MUL);
    ASSERT_EQ(exp->operand_count, 2);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ev");
    ASSERT_EQ(exp->operands[0].operand, e->mat);
    n = QueryGraph_GetNodeByAlias(query_graph, "c");
    ASSERT_EQ(exp->operands[1].operand, n->mat);

    // Validate third expression.
    exp = ae[2];
    ASSERT_EQ(exp->op, AL_EXP_MUL);
    ASSERT_EQ(exp->operand_count, 2);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ew");
    ASSERT_EQ(exp->operands[0].operand, e->mat);
    n = QueryGraph_GetNodeByAlias(query_graph, "e");
    ASSERT_EQ(exp->operands[1].operand, n->mat);

    // Clean up.
    for(int i = 0; i < exp_count; i++) AlgebraicExpression_Free(ae[i]);
    free(ae);
}

TEST_F(AlgebraicExpressionTest, OneIntermidateReturnNode) {
    Edge *e;
    Node *n;
    size_t exp_count = 0;
    const char *query = query_one_intermidate_return_nodes;
    AlgebraicExpression **ae = _build_algebraic_expression(query, &exp_count);
    ASSERT_EQ(exp_count, 2);

    // Validate first expression.
    AlgebraicExpression *exp = ae[0];

    // Validate AlgebraicExpression structure.
    ASSERT_EQ(exp->op, AL_EXP_MUL);
    ASSERT_EQ(exp->operand_count, 5);

    n = QueryGraph_GetNodeByAlias(query_graph, "p");
    ASSERT_EQ(exp->operands[0].operand, n->mat);
    
    e = QueryGraph_GetEdgeByAlias(query_graph, "ef");
    ASSERT_EQ(exp->operands[1].operand, e->mat);

    n = QueryGraph_GetNodeByAlias(query_graph, "f");
    ASSERT_EQ(exp->operands[2].operand, n->mat);

    e = QueryGraph_GetEdgeByAlias(query_graph, "ev");
    ASSERT_EQ(exp->operands[3].operand, e->mat);

    n = QueryGraph_GetNodeByAlias(query_graph, "c");
    ASSERT_EQ(exp->operands[4].operand, n->mat);

    // Validate second expression.
    exp = ae[1];

    // Validate AlgebraicExpression structure.
    ASSERT_EQ(exp->op, AL_EXP_MUL);
    ASSERT_EQ(exp->operand_count, 2);

    e = QueryGraph_GetEdgeByAlias(query_graph, "ew");
    ASSERT_EQ(exp->operands[0].operand, e->mat);

    n = QueryGraph_GetNodeByAlias(query_graph, "e");
    ASSERT_EQ(exp->operands[1].operand, n->mat);

    // Clean up.
    for(int i = 0; i < exp_count; i++) AlgebraicExpression_Free(ae[i]);
    free(ae);
}

TEST_F(AlgebraicExpressionTest, NoIntermidateReturnNodes) {
    Node *n;
    Edge *e;
    size_t exp_count = 0;
    const char *query = query_no_intermidate_return_nodes;
    AlgebraicExpression **ae = _build_algebraic_expression(query, &exp_count);
    ASSERT_EQ(exp_count, 1);

    AlgebraicExpression *exp = ae[0];

    // Validate AlgebraicExpression structure.
    ASSERT_EQ(exp->op, AL_EXP_MUL);
    ASSERT_EQ(exp->operand_count, 7);

    n = QueryGraph_GetNodeByAlias(query_graph, "p");
    ASSERT_EQ(exp->operands[0].operand, n->mat);

    e = QueryGraph_GetEdgeByAlias(query_graph, "ef");
    ASSERT_EQ(exp->operands[1].operand, e->mat);

    n = QueryGraph_GetNodeByAlias(query_graph, "f");
    ASSERT_EQ(exp->operands[2].operand, n->mat);

    e = QueryGraph_GetEdgeByAlias(query_graph, "ev");
    ASSERT_EQ(exp->operands[3].operand, e->mat);

    n = QueryGraph_GetNodeByAlias(query_graph, "c");
    ASSERT_EQ(exp->operands[4].operand, n->mat);

    e = QueryGraph_GetEdgeByAlias(query_graph, "ew");
    ASSERT_EQ(exp->operands[5].operand, e->mat);

    n = QueryGraph_GetNodeByAlias(query_graph, "e");
    ASSERT_EQ(exp->operands[6].operand, n->mat);

    // Clean up.
    AlgebraicExpression_Free(exp);
    free(ae);
}

TEST_F(AlgebraicExpressionTest, OneIntermidateReturnEdge) {
    Node *n;
    Edge *e;    
    size_t exp_count;
    const char *query;
    AlgebraicExpression **ae;
    AlgebraicExpression *exp;

    //==============================================================================
    //=== MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN ef
    //==============================================================================
    exp_count = 0;
    query = query_return_first_edge;
    ae = _build_algebraic_expression(query, &exp_count);
    ASSERT_EQ(exp_count, 2);

    // Validate first expression.
    exp = ae[0];
    ASSERT_EQ(exp->op, AL_EXP_MUL);
    ASSERT_EQ(exp->operand_count, 3);
    ASSERT_TRUE(exp->edge != NULL);

    n = QueryGraph_GetNodeByAlias(query_graph, "p");
    ASSERT_EQ(exp->operands[0].operand, n->mat);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ef");
    ASSERT_EQ(exp->operands[1].operand, e->mat);
    n = QueryGraph_GetNodeByAlias(query_graph, "f");
    ASSERT_EQ(exp->operands[2].operand, n->mat);    

    // Validate second expression.
    exp = ae[1];
    ASSERT_EQ(exp->op, AL_EXP_MUL);
    ASSERT_EQ(exp->operand_count, 4);
    ASSERT_TRUE(exp->edge == NULL);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ev");
    ASSERT_EQ(exp->operands[0].operand, e->mat);
    n = QueryGraph_GetNodeByAlias(query_graph, "c");
    ASSERT_EQ(exp->operands[1].operand, n->mat);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ew");
    ASSERT_EQ(exp->operands[2].operand, e->mat);
    n = QueryGraph_GetNodeByAlias(query_graph, "e");
    ASSERT_EQ(exp->operands[3].operand, n->mat);

    // Clean up.
    for(int i = 0; i < exp_count; i++) AlgebraicExpression_Free(ae[i]);
    free(ae);

    //==============================================================================
    //=== MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN ev
    //==============================================================================
    exp_count = 0;
    query = query_return_intermidate_edge;
    ae = _build_algebraic_expression(query, &exp_count);
    ASSERT_EQ(exp_count, 3);

    // Validate first expression.
    exp = ae[0];
    ASSERT_EQ(exp->op, AL_EXP_MUL);
    ASSERT_EQ(exp->operand_count, 3);
    ASSERT_TRUE(exp->edge == NULL);
    n = QueryGraph_GetNodeByAlias(query_graph, "p");
    ASSERT_EQ(exp->operands[0].operand, n->mat);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ef");
    ASSERT_EQ(exp->operands[1].operand, e->mat);
    n = QueryGraph_GetNodeByAlias(query_graph, "f");
    ASSERT_EQ(exp->operands[2].operand, n->mat);

    // Validate second expression.
    exp = ae[1];
    ASSERT_EQ(exp->op, AL_EXP_MUL);
    ASSERT_EQ(exp->operand_count, 2);    
    ASSERT_TRUE(exp->edge != NULL);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ev");
    ASSERT_EQ(exp->operands[0].operand, e->mat);
    n = QueryGraph_GetNodeByAlias(query_graph, "c");
    ASSERT_EQ(exp->operands[1].operand, n->mat);
    

    // Validate third expression.
    exp = ae[2];
    ASSERT_EQ(exp->op, AL_EXP_MUL);
    ASSERT_EQ(exp->operand_count, 2);
    ASSERT_TRUE(exp->edge == NULL);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ew");
    ASSERT_EQ(exp->operands[0].operand, e->mat);
    n = QueryGraph_GetNodeByAlias(query_graph, "e");
    ASSERT_EQ(exp->operands[1].operand, n->mat);
    

    // Clean up.
    for(int i = 0; i < exp_count; i++) AlgebraicExpression_Free(ae[i]);
    free(ae);

    //==============================================================================
    //=== MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN ew
    //==============================================================================
    exp_count = 0;
    query = query_return_last_edge;
    ae = _build_algebraic_expression(query, &exp_count);
    ASSERT_EQ(exp_count, 2);

    // Validate first expression.
    exp = ae[0];
    ASSERT_EQ(exp->op, AL_EXP_MUL);
    ASSERT_EQ(exp->operand_count, 5);
    ASSERT_TRUE(exp->edge == NULL);
    n = QueryGraph_GetNodeByAlias(query_graph, "p");
    ASSERT_EQ(exp->operands[0].operand, n->mat);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ef");
    ASSERT_EQ(exp->operands[1].operand, e->mat);
    n = QueryGraph_GetNodeByAlias(query_graph, "f");
    ASSERT_EQ(exp->operands[2].operand, n->mat);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ev");
    ASSERT_EQ(exp->operands[3].operand, e->mat);
    n = QueryGraph_GetNodeByAlias(query_graph, "c");
    ASSERT_EQ(exp->operands[4].operand, n->mat);

    // Validate second expression.
    exp = ae[1];
    ASSERT_EQ(exp->op, AL_EXP_MUL);
    ASSERT_EQ(exp->operand_count, 2);
    ASSERT_TRUE(exp->edge != NULL);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ew");
    ASSERT_EQ(exp->operands[0].operand, e->mat);
    n = QueryGraph_GetNodeByAlias(query_graph, "e");
    ASSERT_EQ(exp->operands[1].operand, n->mat);
   

    // Clean up.
    for(int i = 0; i < exp_count; i++) AlgebraicExpression_Free(ae[i]);
    free(ae);
}

TEST_F(AlgebraicExpressionTest, ExpressionExecute) {
    size_t exp_count = 0;
    const char *query = query_no_intermidate_return_nodes;
    AlgebraicExpression **ae = _build_algebraic_expression(query, &exp_count);

    GrB_Matrix res;
    GrB_Matrix_new(&res, GrB_BOOL, Graph_RequiredMatrixDim(g), Graph_RequiredMatrixDim(g));    

    AlgebraicExpression *exp = ae[0];
    AlgebraicExpression_Execute(exp, res);

    Node *src = QueryGraph_GetNodeByAlias(query_graph, "p");
    Node *dest = QueryGraph_GetNodeByAlias(query_graph, "e");
    assert(exp->src_node == src);
    assert(exp->dest_node == dest);
    
    // Validate result matrix.
    GrB_Index ncols, nrows;
    GrB_Matrix_ncols(&ncols, res);
    GrB_Matrix_nrows(&nrows, res);    
    assert(ncols == Graph_RequiredMatrixDim(g));
    assert(nrows == Graph_RequiredMatrixDim(g));

    GrB_Index expected_entries[6] = {1,2, 0,3, 1,3};
    GrB_Matrix expected = NULL;

    GrB_Matrix_dup(&expected, res);
    GrB_Matrix_clear(expected);
    for(int i = 0; i < 6; i+=2) {
        GrB_Matrix_setElement_BOOL(expected, true, expected_entries[i], expected_entries[i+1]);
    }

    assert(_compare_matrices(res, expected));

    // Clean up
    AlgebraicExpression_Free(ae[0]);
    free(ae);
    GrB_Matrix_free(&expected);
    GrB_Matrix_free(&res);
}
