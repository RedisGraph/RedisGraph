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

    void SetUp() {
        // Initialize GraphBLAS.
        assert(GrB_init(GrB_NONBLOCKING) == GrB_SUCCESS);
        srand(time(NULL));

        // Create a graph
        g = _build_graph();

        // Create a graph describing the queries which follows
        query_graph = _build_query_graph(g);

        query_no_intermidate_return_nodes = "MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN p, e";
        query_one_intermidate_return_nodes = "MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN p, c, e";
        query_multiple_intermidate_return_nodes = "MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN p, f, c, e";
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
        Graph *g = Graph_New(16);
        size_t person_count = 6;
        const char *persons[6] = {"Brian", "Stipe", "Max", "Robert", "Francis", "Daniel"};
        size_t country_count = 5;
        const char *countries[5] = {"Israel", "USA", "Japan", "China", "Germany"};
        size_t node_count = person_count + country_count;

        /* Introduce person and country labels. */
        int person_label = Graph_AddLabelMatrix(g);
        int country_label = Graph_AddLabelMatrix(g);
        int labels[11] = {person_label, person_label, person_label, person_label, person_label, person_label,
                        country_label, country_label, country_label, country_label, country_label};

        DataBlockIterator *it;
        Graph_CreateNodes(g, node_count, labels ,&it);

        // Assign attributes to nodes.
        char *default_property_name = (char *)"name";
        for(int i = 0; i < person_count; i++) {
            Node *n = (Node*)DataBlockIterator_Next(it);
            SIValue name = SI_StringVal(persons[i]);
            Node_Add_Properties(n, 1, &default_property_name, &name);
        }
        for(int i = 0; i < country_count; i++) {
            Node *n = (Node*)DataBlockIterator_Next(it);
            SIValue name = SI_StringVal(countries[i]);
            Node_Add_Properties(n, 1, &default_property_name, &name);
        }

        DataBlockIterator_Free(it);

        // Creates a relation matrices.
        GrB_Index friend_relation_id = Graph_AddRelationMatrix(g);
        GrB_Index visit_relation_id = Graph_AddRelationMatrix(g);
        GrB_Index war_relation_id = Graph_AddRelationMatrix(g);

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
        ConnectionDesc relations[27];        
        relations[0].srcId = 0;
        relations[0].destId = 1;
        relations[0].relationId = friend_relation_id;

        relations[1].srcId = 0;
        relations[1].destId = 3;
        relations[1].relationId = friend_relation_id;

        relations[2].srcId = 0;
        relations[2].destId = 5;
        relations[2].relationId = friend_relation_id;

        relations[3].srcId = 1;
        relations[3].destId = 2;
        relations[3].relationId = friend_relation_id;

        relations[4].srcId = 1;
        relations[4].destId = 4;
        relations[4].relationId = friend_relation_id;

        relations[5].srcId = 2;
        relations[5].destId = 3;
        relations[5].relationId = friend_relation_id;

        relations[6].srcId = 2;
        relations[6].destId = 4;
        relations[6].relationId = friend_relation_id;

        relations[7].srcId = 2;
        relations[7].destId = 5;
        relations[7].relationId = friend_relation_id;

        relations[8].srcId = 3;
        relations[8].destId = 0;
        relations[8].relationId = friend_relation_id;

        relations[9].srcId = 3;
        relations[9].destId = 5;
        relations[9].relationId = friend_relation_id;

        relations[10].srcId = 4;
        relations[10].destId = 2;
        relations[10].relationId = friend_relation_id;

        relations[11].srcId = 4;
        relations[11].destId = 3;
        relations[11].relationId = friend_relation_id;

        relations[12].srcId = 5;
        relations[12].destId = 0;
        relations[12].relationId = friend_relation_id;

        relations[13].srcId = 5;
        relations[13].destId = 4;
        relations[13].relationId = friend_relation_id;

        relations[14].srcId = 0;
        relations[14].destId = 0;
        relations[14].relationId = visit_relation_id;

        relations[15].srcId = 0;
        relations[15].destId = 1;
        relations[15].relationId = visit_relation_id;

        relations[16].srcId = 1;
        relations[16].destId = 2;
        relations[16].relationId = visit_relation_id;

        relations[17].srcId = 1;
        relations[17].destId = 4;
        relations[17].relationId = visit_relation_id;

        relations[18].srcId = 2;
        relations[18].destId = 0;
        relations[18].relationId = visit_relation_id;

        relations[19].srcId = 2;
        relations[19].destId = 1;
        relations[19].relationId = visit_relation_id;

        relations[20].srcId = 2;
        relations[20].destId = 3;
        relations[20].relationId = visit_relation_id;

        relations[21].srcId = 3;
        relations[21].destId = 4;
        relations[21].relationId = visit_relation_id;

        relations[22].srcId = 4;
        relations[22].destId = 3;
        relations[22].relationId = visit_relation_id;

        relations[23].srcId = 5;
        relations[23].destId = 4;
        relations[23].relationId = visit_relation_id;

        relations[24].srcId = 0;
        relations[24].destId = 3;
        relations[24].relationId = war_relation_id;

        relations[25].srcId = 1;
        relations[25].destId = 3;
        relations[25].relationId = war_relation_id;

        relations[26].srcId = 4;
        relations[26].destId = 2;
        relations[26].relationId = war_relation_id;

        Graph_ConnectNodes(g, relations, 27, NULL);
        return g;
    }

    QueryGraph *_build_query_graph(Graph *g) {
        /* Query
        * MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) */
        QueryGraph *q = QueryGraph_New();

        // The following indicies are synced with Graph_AddRelationMatrix call order
        // within _build_graph, this is not ideal, but for now this will do.
        int friend_relation_id = 0;
        int visit_relation_id = 1;
        int war_relation_id = 2;

        // Create Nodes
        Node *p = Node_New(0, "Person");
        Node *f = Node_New(1, "Person");
        Node *c = Node_New(2, "City");
        Node *e = Node_New(3, "City");

        // Create edges
        Edge *pff = Edge_New(friend_relation_id, p, f, "friend");
        Edge *fvc = Edge_New(visit_relation_id, f, c, "visit");
        Edge *cwe = Edge_New(war_relation_id, c, e, "war");
        
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

TEST_F(AlgebraicExpressionTest, ExpressionExecute) {
    const char *query = query_no_intermidate_return_nodes;
    AST_Query *ast = ParseQuery(query, strlen(query), NULL);
    size_t exp_count = 0;
    AlgebraicExpression **ae = AlgebraicExpression_From_Query(ast, ast->matchNode->_mergedPatterns, query_graph, &exp_count);

    double tic [2];
    simple_tic(tic);
    AlgebraicExpressionResult *res = AlgebraicExpression_Execute(ae[0]);

    Node **src = QueryGraph_GetNodeRef(query_graph, QueryGraph_GetNodeByAlias(query_graph, "p"));
    Node **dest = QueryGraph_GetNodeRef(query_graph, QueryGraph_GetNodeByAlias(query_graph, "e"));
    assert(res->src_node == src);
    assert(res->dest_node == dest);

    GrB_Matrix M = res->m;
    // double timing = simple_toc(tic);
    // printf("AlgebraicExpression_Execute, time: %.6f sec\n", timing);
    
    // Validate result matrix.
    GrB_Index ncols, nrows;
    GrB_Matrix_ncols(&ncols, M);
    GrB_Matrix_nrows(&nrows, M);    
    assert(ncols == Graph_NodeCount(g));
    assert(nrows == Graph_NodeCount(g));

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
    GrB_Matrix_free(&M);
}
