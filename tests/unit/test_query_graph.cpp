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
#include "../../src/graph/query_graph.h"
#ifdef __cplusplus
}
#endif

class QueryGraphTest: public ::testing::Test {
    protected:
        static void SetUpTestCase() {
            // Use the malloc family for allocations
            Alloc_Reset();
    }

    void compare_nodes(const Node *a, const Node *b) {
        ASSERT_STREQ(a->alias, b->alias);
        ASSERT_STREQ(a->label, b->label);
        ASSERT_EQ(a->labelID, b->labelID);
        ASSERT_EQ(Vector_Size(a->incoming_edges), Vector_Size(b->incoming_edges));
        ASSERT_EQ(Vector_Size(a->outgoing_edges), Vector_Size(b->outgoing_edges));
    }

    void compare_edges(const Edge *a, const Edge *b) {
        ASSERT_STREQ(a->alias, b->alias);
        ASSERT_STREQ(a->relationship, b->relationship);
        ASSERT_EQ(a->relationID, b->relationID);

        compare_nodes(a->src, b->src);
        compare_nodes(a->dest, b->dest);
    }
};

TEST_F(QueryGraphTest, QueryGraphClone) {
    // Create a triangle graph
    // (A)->(B)->(C)->(A)
    size_t node_cap = 3;
    size_t edge_cap = 3;

    // Create nodes.
    const char *label = "L";
    const char *relation = "R";

    Node *A = Node_New(label, "A");
    Node *B = Node_New(label, "B");
    Node *C = Node_New(label, "C");

    Edge *AB = Edge_New(A, B, relation, "AB");
    Edge *BC = Edge_New(B, C, relation, "BC");
    Edge *CA = Edge_New(C, A, relation, "CA");

    QueryGraph *g = QueryGraph_New(node_cap, edge_cap);
    QueryGraph_AddNode(g, A);
    QueryGraph_AddNode(g, B);
    QueryGraph_AddNode(g, C);

    QueryGraph_ConnectNodes(g, A, B, AB);
    QueryGraph_ConnectNodes(g, B, C, BC);
    QueryGraph_ConnectNodes(g, C, A, CA);

    QueryGraph *clone = QueryGraph_Clone(g);

    // Validations.
    ASSERT_EQ(g->node_count, clone->node_count);
    ASSERT_EQ(g->edge_count, clone->edge_count);

    // Validate aliases.
    for(int i = 0; i < g->node_count; i++) {
        ASSERT_STREQ(g->node_aliases[i], clone->node_aliases[i]);
    }
    for(int i = 0; i < g->edge_count; i++) {
        ASSERT_STREQ(g->edge_aliases[i], clone->edge_aliases[i]);
    }

    // Validate nodes.
    for(int i = 0; i < g->node_count; i++) {
        Node *a = g->nodes[i];
        Node *b = clone->nodes[i];
        compare_nodes(a, b);
    }

    // Validate edges.
    for(int i = 0; i < g->edge_count; i++) {
        Edge *a = g->edges[i];
        Edge *b = clone->edges[i];
        compare_edges(a, b);  
    }

    // Clean up.
    QueryGraph_Free(g);
    QueryGraph_Free(clone);
}
