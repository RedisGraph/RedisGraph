/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
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

    void SetUp() {
        // Initialize GraphBLAS.
        assert(GrB_init(GrB_NONBLOCKING) == GrB_SUCCESS);
        srand(time(NULL));

        // Use the malloc family for allocations
        Alloc_Reset();
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
        GrB_finalize();
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
        size_t person_count = 6;
        const char *persons[6] = {"Brian", "Stipe", "Max", "Robert", "Francis", "Daniel"};
        size_t country_count = 5;
        const char *countries[5] = {"Israel", "USA", "Japan", "China", "Germany"};
        size_t node_count = person_count + country_count;

        /* Introduce person and country labels. */
        int person_label = Graph_AddLabel(g);
        int country_label = Graph_AddLabel(g);
        char *default_property_name = (char *)"name";        
        Graph_AllocateNodes(g, node_count);

        for(int i = 0; i < person_count; i++) {
            Node n;
            Graph_CreateNode(g, person_label, &n);
            SIValue name = SI_StringVal(persons[i]);
            GraphEntity_Add_Properties((GraphEntity*)&n, 1, &default_property_name, &name);
        }

        for(int i = 0; i < country_count; i++) {
            Node n;
            Graph_CreateNode(g, country_label, &n);
            SIValue name = SI_StringVal(countries[i]);
            GraphEntity_Add_Properties((GraphEntity*)&n, 1, &default_property_name, &name);
        }

        // Creates a relation matrices.
        GrB_Index friend_relation_id = Graph_AddRelationType(g);
        GrB_Index visit_relation_id = Graph_AddRelationType(g);
        GrB_Index war_relation_id = Graph_AddRelationType(g);

        // Introduce relations, connect nodes.
        /* friendship matrix 
            0 1 0 1 0 1
            0 0 1 0 1 0
            0 0 0 1 1 1
            1 0 0 0 0 1
            0 0 1 1 0 0
            1 0 0 0 1 0

            visit matrix
            1 1 0 0 0 0
            0 0 1 0 1 0
            1 1 0 1 0 0
            0 0 0 0 1 0
            0 0 0 1 0 0
            0 0 0 0 1 0

            war matrix
            0 0 0 1 0 0
            0 0 0 1 0 0
            0 0 0 0 0 0
            0 0 0 0 0 0
            0 0 1 0 0 0
            0 0 0 0 0 0
        */
        Edge e;
        Graph_ConnectNodes(g, 0, 1, friend_relation_id, &e);
        Graph_ConnectNodes(g, 0, 3, friend_relation_id, &e);
        Graph_ConnectNodes(g, 0, 5, friend_relation_id, &e);
        Graph_ConnectNodes(g, 1, 2, friend_relation_id, &e);
        Graph_ConnectNodes(g, 1, 4, friend_relation_id, &e);
        Graph_ConnectNodes(g, 2, 3, friend_relation_id, &e);
        Graph_ConnectNodes(g, 2, 4, friend_relation_id, &e);
        Graph_ConnectNodes(g, 2, 5, friend_relation_id, &e);
        Graph_ConnectNodes(g, 3, 0, friend_relation_id, &e);
        Graph_ConnectNodes(g, 3, 5, friend_relation_id, &e);
        Graph_ConnectNodes(g, 4, 2, friend_relation_id, &e);
        Graph_ConnectNodes(g, 4, 3, friend_relation_id, &e);
        Graph_ConnectNodes(g, 5, 0, friend_relation_id, &e);
        Graph_ConnectNodes(g, 5, 4, friend_relation_id, &e);
        Graph_ConnectNodes(g, 0, 0, visit_relation_id, &e);
        Graph_ConnectNodes(g, 0, 1, visit_relation_id, &e);
        Graph_ConnectNodes(g, 1, 2, visit_relation_id, &e);
        Graph_ConnectNodes(g, 1, 4, visit_relation_id, &e);
        Graph_ConnectNodes(g, 2, 0, visit_relation_id, &e);
        Graph_ConnectNodes(g, 2, 1, visit_relation_id, &e);
        Graph_ConnectNodes(g, 2, 3, visit_relation_id, &e);
        Graph_ConnectNodes(g, 3, 4, visit_relation_id, &e);
        Graph_ConnectNodes(g, 4, 3, visit_relation_id, &e);
        Graph_ConnectNodes(g, 5, 4, visit_relation_id, &e);
        Graph_ConnectNodes(g, 0, 3, war_relation_id, &e);
        Graph_ConnectNodes(g, 1, 3, war_relation_id, &e);
        Graph_ConnectNodes(g, 4, 2, war_relation_id, &e);
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

        // Create edges
        Edge *pff = Edge_New(p, f, "friend", "pff");
        Edge *fvc = Edge_New(f, c, "visit", "fvc");
        Edge *cwe = Edge_New(c, e, "war", "cwe");
        
        // Set edges matrices according to the order they've been presented
        // during graph construction.
        pff->mat = Graph_GetRelationMatrix(g, 0);
        fvc->mat = Graph_GetRelationMatrix(g, 1);
        cwe->mat = Graph_GetRelationMatrix(g, 2);

        // Construct query graph
        QueryGraph_AddNode(q, p, (char*)"p");
        QueryGraph_AddNode(q, f, (char*)"f");
        QueryGraph_AddNode(q, c, (char*)"c");
        QueryGraph_AddNode(q, e, (char*)"e");
        QueryGraph_ConnectNodes(q, p, f, pff, (char*)"ef");
        QueryGraph_ConnectNodes(q, f, c, fvc, (char*)"ev");
        QueryGraph_ConnectNodes(q, c, e, cwe, (char*)"ew");

        return q;
    }
};

TEST_F(AlgebraicExpressionTest, MultipleIntermidateReturnNodes) {
    const char *query = query_multiple_intermidate_return_nodes;

    AST_Query *ast = ParseQuery(query, strlen(query), NULL);
    
    size_t exp_count = 0;
    AlgebraicExpression **ae = AlgebraicExpression_From_Query(ast, ast->matchNode->_mergedPatterns, query_graph, &exp_count);
    EXPECT_EQ(exp_count, 3);
    
    // Validate first expression.
    AlgebraicExpression *exp = ae[0];
    EXPECT_EQ(exp->op, AL_EXP_MUL);
    EXPECT_EQ(exp->operand_count, 1);

    Edge *e;
    e = QueryGraph_GetEdgeByAlias(query_graph, "ef");
    EXPECT_EQ(exp->operands[0].operand, e->mat);

    // Validate second expression.
    exp = ae[1];
    EXPECT_EQ(exp->op, AL_EXP_MUL);
    EXPECT_EQ(exp->operand_count, 1);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ev");
    EXPECT_EQ(exp->operands[0].operand, e->mat);

    // Validate third expression.
    exp = ae[2];
    EXPECT_EQ(exp->op, AL_EXP_MUL);
    EXPECT_EQ(exp->operand_count, 1);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ew");
    EXPECT_EQ(exp->operands[0].operand, e->mat);

    // Clean up.
    Free_AST_Query(ast);
    for(int i = 0; i < exp_count; i++) AlgebraicExpression_Free(ae[i]);
    free(ae);
}

TEST_F(AlgebraicExpressionTest, OneIntermidateReturnNode) {
    const char *query = query_one_intermidate_return_nodes;
    AST_Query *ast = ParseQuery(query, strlen(query), NULL);

    size_t exp_count = 0;
    AlgebraicExpression **ae = AlgebraicExpression_From_Query(ast, ast->matchNode->_mergedPatterns, query_graph, &exp_count);
    EXPECT_EQ(exp_count, 2);

    // Validate first expression.
    AlgebraicExpression *exp = ae[0];

    // Validate AlgebraicExpression structure.
    EXPECT_EQ(exp->op, AL_EXP_MUL);
    EXPECT_EQ(exp->operand_count, 2);
    
    Edge *e;
    e = QueryGraph_GetEdgeByAlias(query_graph, "ef");
    EXPECT_EQ(exp->operands[1].operand, e->mat);

    e = QueryGraph_GetEdgeByAlias(query_graph, "ev");
    EXPECT_EQ(exp->operands[0].operand, e->mat);

    // Validate second expression.
    exp = ae[1];

    // Validate AlgebraicExpression structure.
    EXPECT_EQ(exp->op, AL_EXP_MUL);
    EXPECT_EQ(exp->operand_count, 1);

    e = QueryGraph_GetEdgeByAlias(query_graph, "ew");
    EXPECT_EQ(exp->operands[0].operand, e->mat);

    // Clean up.
    Free_AST_Query(ast);
    for(int i = 0; i < exp_count; i++) AlgebraicExpression_Free(ae[i]);
    free(ae);
}

TEST_F(AlgebraicExpressionTest, NoIntermidateReturnNodes) {
    const char *query = query_no_intermidate_return_nodes;
    AST_Query *ast = ParseQuery(query, strlen(query), NULL);

    size_t exp_count = 0;
    AlgebraicExpression **ae = AlgebraicExpression_From_Query(ast, ast->matchNode->_mergedPatterns, query_graph, &exp_count);
    EXPECT_EQ(exp_count, 1);

    AlgebraicExpression *exp = ae[0];

    // Validate AlgebraicExpression structure.
    EXPECT_EQ(exp->op, AL_EXP_MUL);
    EXPECT_EQ(exp->operand_count, 3);

    Edge *e;
    e = QueryGraph_GetEdgeByAlias(query_graph, "ef");
    EXPECT_EQ(exp->operands[2].operand, e->mat);

    e = QueryGraph_GetEdgeByAlias(query_graph, "ev");
    EXPECT_EQ(exp->operands[1].operand, e->mat);

    e = QueryGraph_GetEdgeByAlias(query_graph, "ew");
    EXPECT_EQ(exp->operands[0].operand, e->mat);

    // Clean up.
    Free_AST_Query(ast);
    AlgebraicExpression_Free(exp);
    free(ae);
}

TEST_F(AlgebraicExpressionTest, OneIntermidateReturnEdge) {
    Edge *e;
    AST_Query *ast;
    size_t exp_count;
    const char *query;
    AlgebraicExpression **ae;
    AlgebraicExpression *exp;

    //==============================================================================
    //=== MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN ef
    //==============================================================================
    query = query_return_first_edge;
    ast = ParseQuery(query, strlen(query), NULL);

    exp_count = 0;
    ae = AlgebraicExpression_From_Query(ast, ast->matchNode->_mergedPatterns, query_graph, &exp_count);
    EXPECT_EQ(exp_count, 2);

    // Validate first expression.
    exp = ae[0];
    EXPECT_EQ(exp->op, AL_EXP_MUL);
    EXPECT_EQ(exp->operand_count, 1);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ef");
    EXPECT_EQ(exp->operands[0].operand, e->mat);
    EXPECT_TRUE(exp->edge != NULL);
    
    // Validate second expression.
    exp = ae[1];
    EXPECT_EQ(exp->op, AL_EXP_MUL);
    EXPECT_EQ(exp->operand_count, 2);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ew");
    EXPECT_EQ(exp->operands[0].operand, e->mat);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ev");
    EXPECT_EQ(exp->operands[1].operand, e->mat);
    EXPECT_TRUE(exp->edge == NULL);

    // Clean up.
    Free_AST_Query(ast);
    for(int i = 0; i < exp_count; i++) AlgebraicExpression_Free(ae[i]);
    free(ae);

    //==============================================================================
    //=== MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN ev
    //==============================================================================
    query = query_return_intermidate_edge;
    ast = ParseQuery(query, strlen(query), NULL);

    exp_count = 0;
    ae = AlgebraicExpression_From_Query(ast, ast->matchNode->_mergedPatterns, query_graph, &exp_count);
    EXPECT_EQ(exp_count, 3);

    // Validate first expression.
    exp = ae[0];
    EXPECT_EQ(exp->op, AL_EXP_MUL);
    EXPECT_EQ(exp->operand_count, 1);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ef");
    EXPECT_EQ(exp->operands[0].operand, e->mat);
    EXPECT_TRUE(exp->edge == NULL);

    // Validate second expression.
    exp = ae[1];
    EXPECT_EQ(exp->op, AL_EXP_MUL);
    EXPECT_EQ(exp->operand_count, 1);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ev");
    EXPECT_EQ(exp->operands[0].operand, e->mat);
    EXPECT_TRUE(exp->edge != NULL);

    // Validate third expression.
    exp = ae[2];
    EXPECT_EQ(exp->op, AL_EXP_MUL);
    EXPECT_EQ(exp->operand_count, 1);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ew");
    EXPECT_EQ(exp->operands[0].operand, e->mat);
    EXPECT_TRUE(exp->edge == NULL);

    // Clean up.
    Free_AST_Query(ast);
    for(int i = 0; i < exp_count; i++) AlgebraicExpression_Free(ae[i]);
    free(ae);

    //==============================================================================
    //=== MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN ew
    //==============================================================================
    query = query_return_last_edge;
    ast = ParseQuery(query, strlen(query), NULL);

    exp_count = 0;
    ae = AlgebraicExpression_From_Query(ast, ast->matchNode->_mergedPatterns, query_graph, &exp_count);
    EXPECT_EQ(exp_count, 2);

    // Validate first expression.
    exp = ae[0];
    EXPECT_EQ(exp->op, AL_EXP_MUL);
    EXPECT_EQ(exp->operand_count, 2);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ev");
    EXPECT_EQ(exp->operands[0].operand, e->mat);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ef");
    EXPECT_EQ(exp->operands[1].operand, e->mat);
    EXPECT_TRUE(exp->edge == NULL);

    // Validate second expression.
    exp = ae[1];
    EXPECT_EQ(exp->op, AL_EXP_MUL);
    EXPECT_EQ(exp->operand_count, 1);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ew");
    EXPECT_EQ(exp->operands[0].operand, e->mat);
    EXPECT_TRUE(exp->edge != NULL);

    // Clean up.
    Free_AST_Query(ast);
    for(int i = 0; i < exp_count; i++) AlgebraicExpression_Free(ae[i]);
    free(ae);
}

TEST_F(AlgebraicExpressionTest, ExpressionExecute) {
    const char *query = query_no_intermidate_return_nodes;
    AST_Query *ast = ParseQuery(query, strlen(query), NULL);
    size_t exp_count = 0;
    AlgebraicExpression **ae = AlgebraicExpression_From_Query(ast, ast->matchNode->_mergedPatterns, query_graph, &exp_count);

    double tic [2];
    simple_tic(tic);
    AlgebraicExpressionResult *res = AlgebraicExpression_Execute(ae[0]);

    Node *src = QueryGraph_GetNodeByAlias(query_graph, "p");
    Node *dest = QueryGraph_GetNodeByAlias(query_graph, "e");
    assert(res->src_node == src);
    assert(res->dest_node == dest);

    GrB_Matrix M = res->m;
    // double timing = simple_toc(tic);
    // printf("AlgebraicExpression_Execute, time: %.6f sec\n", timing);
    
    // Validate result matrix.
    GrB_Index ncols, nrows;
    GrB_Matrix_ncols(&ncols, M);
    GrB_Matrix_nrows(&nrows, M);    
    assert(ncols == Graph_RequiredMatrixDim(g));
    assert(nrows == Graph_RequiredMatrixDim(g));

    // Expected result.
    // 0   0   0   0   0   0
    // 0   0   0   0   0   0
    // 1   0   1   1   1   0
    // 0   1   0   1   1   1
    // 0   0   0   0   0   0
    // 0   0   0   0   0   0

    GrB_Index expected_entries[16] = {2,0, 3,1, 2,2, 2,3, 3,3, 2,4, 3,4, 3,5};
    GrB_Matrix expected = NULL;

    GrB_Matrix_dup(&expected, M);
    GrB_Matrix_clear(expected);
    for(int i = 0; i < 16; i+=2) {
        GrB_Matrix_setElement_BOOL(expected, true, expected_entries[i], expected_entries[i+1]);
    }

    assert(_compare_matrices(M, expected));

    // Clean up
    Free_AST_Query(ast);
    AlgebraicExpression_Free(ae[0]);
    free(ae);
    free(res);
    GrB_Matrix_free(&expected);
    GrB_Matrix_free(&M);
}
