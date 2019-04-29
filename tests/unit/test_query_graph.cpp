/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../../deps/googletest/include/gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "../../src/util/arr.h"
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
        ASSERT_EQ(array_len(a->incoming_edges), array_len(b->incoming_edges));
        ASSERT_EQ(array_len(a->outgoing_edges), array_len(b->outgoing_edges));
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


TEST_F(QueryGraphTest, QueryGraphRemoveEntities) {
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

    // Remove an edge.
    ASSERT_TRUE(QueryGraph_ContainsEdge(g, AB));
    ASSERT_TRUE(QueryGraph_GetEdgeByAlias(g, AB->alias) != NULL);
    
    QueryGraph_RemoveEdge(g, AB);

    ASSERT_FALSE(QueryGraph_ContainsEdge(g, AB));
    ASSERT_FALSE(QueryGraph_GetEdgeByAlias(g, AB->alias) != NULL);
    
    // Remove node.
    ASSERT_TRUE(QueryGraph_ContainsNode(g, C));
    ASSERT_TRUE(QueryGraph_GetNodeByAlias(g, C->alias) != NULL);

    QueryGraph_RemoveNode(g, C);
    
    ASSERT_FALSE(QueryGraph_ContainsNode(g, C));
    ASSERT_FALSE(QueryGraph_GetNodeByAlias(g, C->alias) != NULL);

    // Both CA BC edges should be removed.
    ASSERT_FALSE(QueryGraph_ContainsEdge(g, CA));
    ASSERT_FALSE(QueryGraph_GetEdgeByAlias(g, CA->alias) != NULL);
    
    ASSERT_FALSE(QueryGraph_ContainsEdge(g, BC));
    ASSERT_FALSE(QueryGraph_GetEdgeByAlias(g, BC->alias) != NULL);

    /* Assert entity count:
     * Nodes - A was removed.
     * Edges - AB explicitly removed, BC and CA implicitly removed. */
    ASSERT_EQ(g->node_count, 2);    
    ASSERT_EQ(g->edge_count, 0);

    // Assert remaining entities, 
    ASSERT_TRUE(QueryGraph_ContainsNode(g, A));
    ASSERT_TRUE(QueryGraph_GetNodeByAlias(g, A->alias) != NULL);
    ASSERT_TRUE(QueryGraph_ContainsNode(g, B));
    ASSERT_TRUE(QueryGraph_GetNodeByAlias(g, B->alias) != NULL);

    // Clean up.
    QueryGraph_Free(g);
}
