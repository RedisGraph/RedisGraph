/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../../deps/googletest/include/gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "../../src/util/rmalloc.h"

#include "../../src/parser/ast.h"
#include "../../src/query_executor.h"
#include "../../src/graph/query_graph.h"
#include "../../src/filter_tree/filter_tree.h"
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
    }

    static void TearDownTestCase() {
    }

    AST* _build_ast(const char *query) {
        char *errMsg;
        AST **ast = ParseQuery(query, strlen(query), &errMsg);
        AST_NameAnonymousNodes(ast[0]);
        return ast[0];
    }

    FT_FilterNode* build_filter_tree_from_query(const char *query) {
        AST *ast = _build_ast(query);
        AST_FilterNode *root = ast->whereNode->filters;
        FT_FilterNode *tree = BuildFiltersTree(ast, root);
        return tree;
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

    Node *A = Node_New(NULL, "A");
    Node *B = Node_New(NULL, "B");
    Node *C = Node_New(NULL, "C");
    Node *D = Node_New(NULL, "D");

    Edge *AB = Edge_New(A, B, "E", "AB");
    Edge *BC = Edge_New(B, C, "E", "BC");
    Edge *CD = Edge_New(C, D, "E", "CD");

    QueryGraph *qg = QueryGraph_New(4, 3);

    QueryGraph_AddNode(qg, A);
    QueryGraph_AddNode(qg, B);
    QueryGraph_AddNode(qg, C);
    QueryGraph_AddNode(qg, D);
    QueryGraph_ConnectNodes(qg, A, B, AB);
    QueryGraph_ConnectNodes(qg, B, C, BC);
    QueryGraph_ConnectNodes(qg, C, D, CD);

    AlgebraicExpression *set[3];
    AlgebraicExpression *ExpAB = AlgebraicExpression_Empty();
    AlgebraicExpression *ExpBC = AlgebraicExpression_Empty();
    AlgebraicExpression *ExpCD = AlgebraicExpression_Empty();
    
    AlgebraicExpression_AppendTerm(ExpAB, NULL, false, false, false);
    AlgebraicExpression_AppendTerm(ExpBC, NULL, false, false, false);
    AlgebraicExpression_AppendTerm(ExpCD, NULL, false, false, false);

    ExpAB->src_node = A;
    ExpAB->dest_node = B;
    ExpBC->src_node = B;
    ExpBC->dest_node = C;
    ExpCD->src_node = C;
    ExpCD->dest_node = D;

    // { [CD], [BC], [AB] }
    set[0] = ExpCD;
    set[1] = ExpBC;
    set[2] = ExpAB;

    orderExpressions(set, 3, NULL);
    ASSERT_EQ(set[0], ExpAB);
    ASSERT_EQ(set[1], ExpBC);
    ASSERT_EQ(set[2], ExpCD);

    // { [AB], [BC], [CD] }
    set[0] = ExpAB;
    set[1] = ExpBC;
    set[2] = ExpCD;
    orderExpressions(set, 3, NULL);
    ASSERT_EQ(set[0], ExpAB);
    ASSERT_EQ(set[1], ExpBC);
    ASSERT_EQ(set[2], ExpCD);
    
    // { [AB], [CD], [BC] }
    set[0] = ExpAB;
    set[1] = ExpCD;
    set[2] = ExpBC;
    orderExpressions(set, 3, NULL);
    ASSERT_EQ(set[0], ExpAB);
    ASSERT_EQ(set[1], ExpBC);
    ASSERT_EQ(set[2], ExpCD);
    
    // { [BC], [AB], [CD] }
    set[0] = ExpBC;
    set[1] = ExpAB;
    set[2] = ExpCD;
    orderExpressions(set, 3, NULL);
    ASSERT_EQ(set[0], ExpAB);
    ASSERT_EQ(set[1], ExpBC);
    ASSERT_EQ(set[2], ExpCD);
    
    // { [BC], [CD], [AB] }
    set[0] = ExpBC;
    set[1] = ExpCD;
    set[2] = ExpAB;
    orderExpressions(set, 3, NULL);
    ASSERT_EQ(set[0], ExpAB);
    ASSERT_EQ(set[1], ExpBC);
    ASSERT_EQ(set[2], ExpCD);
    
    // { [CD], [AB], [BC] }
    set[0] = ExpCD;
    set[1] = ExpAB;
    set[2] = ExpBC;
    orderExpressions(set, 3, NULL);
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
    Node *A = Node_New(NULL, "A");
    Node *B = Node_New(NULL, "B");
    Node *C = Node_New(NULL, "C");
    Node *D = Node_New(NULL, "D");

    Edge *AB = Edge_New(A, B, "E", "AB");
    Edge *BC = Edge_New(B, C, "E", "BC");
    Edge *CD = Edge_New(C, D, "E", "CD");

    QueryGraph *qg = QueryGraph_New(4, 3);

    QueryGraph_AddNode(qg, A);
    QueryGraph_AddNode(qg, B);
    QueryGraph_AddNode(qg, C);
    QueryGraph_AddNode(qg, D);
    QueryGraph_ConnectNodes(qg, A, B, AB);
    QueryGraph_ConnectNodes(qg, B, C, BC);
    QueryGraph_ConnectNodes(qg, C, D, CD);

    AlgebraicExpression *set[3];
    AlgebraicExpression *ExpAB = AlgebraicExpression_Empty();
    AlgebraicExpression *ExpBC = AlgebraicExpression_Empty();
    AlgebraicExpression *ExpCD = AlgebraicExpression_Empty();
    
    AlgebraicExpression_AppendTerm(ExpAB, NULL, false, false, false);
    AlgebraicExpression_AppendTerm(ExpBC, NULL, false, false, false);
    AlgebraicExpression_AppendTerm(ExpCD, NULL, false, false, false);

    ExpAB->src_node = A;
    ExpAB->dest_node = B;
    ExpBC->src_node = B;
    ExpBC->dest_node = C;
    ExpCD->src_node = C;
    ExpCD->dest_node = D;

    // { [AB], [BC], [CD] }
    set[0] = ExpAB;
    set[1] = ExpBC;
    set[2] = ExpCD;

    filters = build_filter_tree_from_query("MATCH (A)-[]->(B)-[]->(C)-[]->(D) WHERE A.val = 1 RETURN *");

    orderExpressions(set, 3, filters);
    ASSERT_EQ(set[0], ExpAB);
    ASSERT_EQ(set[1], ExpBC);
    ASSERT_EQ(set[2], ExpCD);

    FilterTree_Free(filters);

    // { [AB], [BC], [CD] }
    set[0] = ExpAB;
    set[1] = ExpBC;
    set[2] = ExpCD;

    filters = build_filter_tree_from_query("MATCH (A)-[]->(B)-[]->(C)-[]->(D) WHERE B.val = 1 RETURN *");

    orderExpressions(set, 3, filters);
    ASSERT_TRUE (set[0] == ExpAB || set[0] == ExpBC);

    FilterTree_Free(filters);

    // { [AB], [BC], [CD] }
    set[0] = ExpAB;
    set[1] = ExpBC;
    set[2] = ExpCD;

    filters = build_filter_tree_from_query("MATCH (A)-[]->(B)-[]->(C)-[]->(D) WHERE C.val = 1 RETURN *");

    orderExpressions(set, 3, filters);
    ASSERT_TRUE(set[0] == ExpBC || set[0] == ExpCD);

    FilterTree_Free(filters);

    // { [AB], [BC], [CD] }
    set[0] = ExpAB;
    set[1] = ExpBC;
    set[2] = ExpCD;

    filters = build_filter_tree_from_query("MATCH (A)-[]->(B)-[]->(C)-[]->(D) WHERE D.val = 1 RETURN *");

    orderExpressions(set, 3, filters);

    ASSERT_EQ(set[0], ExpCD);
    ASSERT_EQ(set[1], ExpBC);
    ASSERT_EQ(set[2], ExpAB);

    // Clean up.
    FilterTree_Free(filters);
    AlgebraicExpression_Free(ExpAB);
    AlgebraicExpression_Free(ExpBC);
    AlgebraicExpression_Free(ExpCD);
    QueryGraph_Free(qg);
}
