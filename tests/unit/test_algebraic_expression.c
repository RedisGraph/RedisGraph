#include "../../src/value.h"
#include "../../src/graph/graph.h"
#include "../../src/query_executor.h"
#include "../../src/graph/query_graph.h"
#include "../../src/util/simple_timer.h"
#include "../../src/arithmetic/algebraic_expression.h"

#include <assert.h>

// Console text colors
#define KGRN  "\x1B[32m"
#define KRED  "\x1B[31m"
#define KNRM  "\x1B[0m"

/* Get a string representation of algebraic expression. */
// char* AlgebraicExpression_To_String(const AlgebraicExpression* ae);

void _print_matrix(GrB_Matrix mat) {
    GrB_Index ncols, nrows, nvals;
    GrB_Matrix_ncols(&ncols, mat);
    GrB_Matrix_nrows(&nrows, mat);
    GrB_Matrix_nvals(&nvals, mat);
    printf("ncols: %llu, nrows: %llu, nvals: %llu\n", ncols, nrows, nvals);

    GrB_Index I[nvals];     // array for returning row indices of tuples
    GrB_Index J[nvals];     // array for returning col indices of tuples
    bool X[nvals];          // array for returning values of tuples

    GrB_Matrix_extractTuples(I, J, X, &nvals, mat);
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

    GrB_Matrix_extractTuples(aI, aJ, aX, &avals, a);
    GrB_Matrix_extractTuples(bI, bJ, bX, &bvals, b);

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
    char *persons[6] = {"Brian", "Stipe", "Max", "Robert", "Francis", "Daniel"};
    size_t country_count = 5;
    char *countries[5] = {"Israel", "USA", "Japan", "China", "Germany"};
    size_t node_count = person_count + country_count;

    /* Introduce person and country labels. */
    int person_label = Graph_AddLabelMatrix(g);
    int country_label = Graph_AddLabelMatrix(g);
    int labels[11] = {person_label, person_label, person_label, person_label, person_label, person_label,
                      country_label, country_label, country_label, country_label, country_label};

    NodeIterator *it;
    Graph_CreateNodes(g, node_count, labels ,&it);

    // Assign attributes to nodes.
    char *default_property_name = "name";
    for(int i = 0; i < person_count; i++) {
        Node *n = NodeIterator_Next(it);
        SIValue name = SI_StringValC(persons[i]);
        Node_Add_Properties(n, 1, &default_property_name, &name);
    }
    for(int i = 0; i < country_count; i++) {
        Node *n = NodeIterator_Next(it);
        SIValue name = SI_StringValC(countries[i]);
        Node_Add_Properties(n, 1, &default_property_name, &name);
    }

    NodeIterator_Free(it);

    // Creates a relation matrices.
    int friend_relation_id = Graph_AddRelationMatrix(g);
    int visit_relation_id = Graph_AddRelationMatrix(g);
    int war_relation_id = Graph_AddRelationMatrix(g);

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
    GrB_Index relations[81] = {
        0, 1, friend_relation_id,
        0, 3, friend_relation_id,
        0, 5, friend_relation_id,
        1, 2, friend_relation_id,
        1, 4, friend_relation_id,
        2, 3, friend_relation_id,
        2, 4, friend_relation_id,
        2, 5, friend_relation_id,
        3, 0, friend_relation_id,
        3, 5, friend_relation_id,
        4, 2, friend_relation_id,
        4, 3, friend_relation_id,
        5, 0, friend_relation_id,
        5, 4, friend_relation_id,
        0, 0, visit_relation_id,
        0, 1, visit_relation_id,
        1, 2, visit_relation_id,
        1, 4, visit_relation_id,
        2, 0, visit_relation_id,
        2, 1, visit_relation_id,
        2, 3, visit_relation_id,
        3, 4, visit_relation_id,
        4, 3, visit_relation_id,
        5, 4, visit_relation_id,
        0, 3, war_relation_id,
        1, 3, war_relation_id,
        4, 2, war_relation_id};

    Graph_ConnectNodes(g, 81, relations);
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
    pff->mat = g->relations[0];
    fvc->mat = g->relations[1];
    cwe->mat = g->relations[2];

    // Construct query graph
    QueryGraph_AddNode(q, p, "p");
    QueryGraph_AddNode(q, f, "f");
    QueryGraph_AddNode(q, c, "c");
    QueryGraph_AddNode(q, e, "e");
    QueryGraph_ConnectNodes(q, p, f, pff, "ef");
    QueryGraph_ConnectNodes(q, f, c, fvc, "ev");
    QueryGraph_ConnectNodes(q, c, e, cwe, "ew");

    return q;
}

void test_multiple_intermidate_return_nodes(const char *query, const QueryGraph *query_graph) {
    AST_Query *ast = ParseQuery(query, strlen(query), NULL);
    
    size_t exp_count = 0;
    AlgebraicExpression **ae = AlgebraicExpression_From_Query(ast, query_graph, &exp_count);
    assert(exp_count == 3);
    
    // Validate first expression.
    AlgebraicExpression *exp = ae[0];
    assert(exp->op == AL_EXP_MUL);
    assert(exp->operand_count == 1);
    Edge *e;
    e = QueryGraph_GetEdgeByAlias(query_graph, "ef");
    assert(exp->operands[0] == e->mat);

    // Validate second expression.
    exp = ae[1];
    assert(exp->op == AL_EXP_MUL);
    assert(exp->operand_count == 1);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ev");
    assert(exp->operands[0] == e->mat);

    // Validate third expression.
    exp = ae[2];
    assert(exp->op == AL_EXP_MUL);
    assert(exp->operand_count == 1);
    e = QueryGraph_GetEdgeByAlias(query_graph, "ew");
    assert(exp->operands[0] == e->mat);

    // Clean up.
    Free_AST_Query(ast);
    for(int i = 0; i < exp_count; i++) AlgebraicExpression_Free(ae[i]);
    free(ae);

    printf("%stest_multiple_intermidate_return_nodes - PASS!%s\n", KGRN, KNRM);
}

void test_one_intermidate_return_node(const char *query, const QueryGraph *query_graph) {
    AST_Query *ast = ParseQuery(query, strlen(query), NULL);

    size_t exp_count = 0;
    AlgebraicExpression **ae = AlgebraicExpression_From_Query(ast, query_graph, &exp_count);
    assert(exp_count == 2);

    // Validate first expression.
    AlgebraicExpression *exp = ae[0];

    // Validate AlgebraicExpression structure.
    assert(exp->op == AL_EXP_MUL);
    assert(exp->operand_count == 2);
    
    Edge *e;
    e = QueryGraph_GetEdgeByAlias(query_graph, "ef");
    assert(exp->operands[0] == e->mat);

    e = QueryGraph_GetEdgeByAlias(query_graph, "ev");
    assert(exp->operands[1] == e->mat);

    // Validate second expression.
    exp = ae[1];

    // Validate AlgebraicExpression structure.
    assert(exp->op == AL_EXP_MUL);
    assert(exp->operand_count == 1);

    e = QueryGraph_GetEdgeByAlias(query_graph, "ew");
    assert(exp->operands[0] == e->mat);

    // Clean up.
    Free_AST_Query(ast);
    for(int i = 0; i < exp_count; i++) AlgebraicExpression_Free(ae[i]);
    free(ae);

    printf("%stest_one_intermidate_return_node - PASS!%s\n", KGRN, KNRM);
}

void test_no_intermidate_return_nodes(const char *query, const QueryGraph *query_graph) {
    AST_Query *ast = ParseQuery(query, strlen(query), NULL);

    size_t exp_count = 0;
    AlgebraicExpression **ae = AlgebraicExpression_From_Query(ast, query_graph, &exp_count);
    assert(exp_count == 1);

    AlgebraicExpression *exp = ae[0];

    // Validate AlgebraicExpression structure.
    assert(exp->op == AL_EXP_MUL);
    assert(exp->operand_count == 3);

    Edge *e;
    e = QueryGraph_GetEdgeByAlias(query_graph, "ef");
    assert(exp->operands[0] == e->mat);

    e = QueryGraph_GetEdgeByAlias(query_graph, "ev");
    assert(exp->operands[1] == e->mat);

    e = QueryGraph_GetEdgeByAlias(query_graph, "ew");
    assert(exp->operands[2] == e->mat);

    // Clean up.
    Free_AST_Query(ast);
    AlgebraicExpression_Free(exp);
    free(ae);

    printf("%stest_no_intermidate_return_nodes - PASS!%s\n", KGRN, KNRM);
}

void test_expression_execute(const char *query, Graph *g, QueryGraph *query_graph) {
    AST_Query *ast = ParseQuery(query, strlen(query), NULL);
    size_t exp_count = 0;
    AlgebraicExpression **ae = AlgebraicExpression_From_Query(ast, query_graph, &exp_count);

    double tic [2];
    simple_tic(tic);
    AlgebraicExpressionResult *res = AlgebraicExpression_Execute(ae[0]);

    Node **src = QueryGraph_GetNodeRef(query_graph, QueryGraph_GetNodeByAlias(query_graph, "p"));
    Node **dest = QueryGraph_GetNodeRef(query_graph, QueryGraph_GetNodeByAlias(query_graph, "e"));
    assert(res->src_node == src);
    assert(res->dest_node == dest);

    GrB_Matrix M = res->m;
    double timing = simple_toc(tic);
    printf("AlgebraicExpression_Execute, time: %.6f sec\n", timing);
    
    // Validate result matrix.
    GrB_Index ncols, nrows, nvals;
    GrB_Matrix_ncols(&ncols, M);
    GrB_Matrix_nrows(&nrows, M);    
    assert(ncols == g->node_cap);
    assert(nrows == g->node_cap);

    // Expected result.
    // 0   0   1   0   0   0
    // 0   0   0   1   0   0
    // 0   0   1   0   0   0
    // 0   0   1   1   0   0
    // 0   0   1   1   0   0
    // 0   0   0   1   0   0
    GrB_Index expected_entries[16] = {0,2, 1,3, 2,2, 3,2, 3,3, 4,2, 4,3, 5,3};
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

    printf("%stest_expression_execute - PASS!%s\n", KGRN, KNRM);
}

int main(int argc, char **argv) {
    // Initialize GraphBLAS.
    assert(GrB_init(GrB_NONBLOCKING) == GrB_SUCCESS);
    srand(time(NULL));

    // Create a graph
    Graph *g = _build_graph();
    
    // Create a graph describing the queries which follows
    QueryGraph *query_graph = _build_query_graph(g);
    const char *query_no_intermidate_return_nodes = "MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN p, e";
    const char *query_one_intermidate_return_nodes = "MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN p, c, e";
    const char *query_multiple_intermidate_return_nodes = "MATCH (p:Person)-[ef:friend]->(f:Person)-[ev:visit]->(c:City)-[ew:war]->(e:City) RETURN p, f, c, e";
    
    test_no_intermidate_return_nodes(query_no_intermidate_return_nodes, query_graph);
    test_one_intermidate_return_node(query_one_intermidate_return_nodes, query_graph);
    test_multiple_intermidate_return_nodes(query_multiple_intermidate_return_nodes, query_graph);
    test_expression_execute(query_no_intermidate_return_nodes, g, query_graph);

    // Clean up
    Graph_Free(g);
    QueryGraph_Free(query_graph);
    GrB_finalize();
    return 0;
}
