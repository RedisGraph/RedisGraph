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
#include "../../src/graph/graph.h"
#include "../../src/index/index.h"
#include "../../src/util/rmalloc.h"
#include "../../src/graph/graphcontext.h"

#ifdef __cplusplus
}
#endif

extern pthread_key_t _tlsGCKey;    // Thread local storage graph context key.

class IndexTest: public ::testing::Test {
  protected:
    static void SetUpTestCase() {
      // Use the malloc family for allocations
      Alloc_Reset();
      ASSERT_EQ(GrB_init(GrB_NONBLOCKING), GrB_SUCCESS);
      GxB_Global_Option_set(GxB_FORMAT, GxB_BY_ROW); // all matrices in CSR format
      GxB_Global_Option_set(GxB_HYPER, GxB_NEVER_HYPER); // matrices are never hypersparse

      _fake_graph_context();
    }

    static void TearDownTestCase() {
        _free_fake_graph_context();
        GrB_finalize();
    }

    static void _fake_graph_context() {
        GraphContext *gc = (GraphContext*)malloc(sizeof(GraphContext));

        gc->g = _build_test_graph();
        gc->index_count = 0;
        gc->graph_name = strdup("G");
        gc->attributes = NewTrieMap();
        gc->string_mapping = (char**)array_new(char*, 64);
        gc->node_schemas = (Schema**)array_new(Schema*, GRAPH_DEFAULT_LABEL_CAP);
        gc->relation_schemas = (Schema**)array_new(Schema*, GRAPH_DEFAULT_RELATION_TYPE_CAP);

        GraphContext_AddSchema(gc, "Person", SCHEMA_NODE);

        int error = pthread_key_create(&_tlsGCKey, NULL);
        ASSERT_EQ(error, 0);
        pthread_setspecific(_tlsGCKey, gc);
    }

    static void _free_fake_graph_context() {
        GraphContext *gc = GraphContext_GetFromTLS();
        GraphContext_Free(gc);
    }

    static Graph* _build_test_graph() {
        // Allocate graph
        size_t n = 16;
        Graph *g = Graph_New(n, n);
        Graph_AcquireWriteLock(g);

        // Build a label matrix and add to graph
        // (this does not need to be associated with an actual label string)
        Graph_AddLabel(g);

        Graph_ReleaseLock(g);
        return g;
    }
};

TEST_F(IndexTest, Index_New) {
    const char *l = "Person";
    Index *idx = Index_New(l, IDX_EXACT_MATCH);

    // Return indexed label.
    char *label = Index_GetLabel(idx);
    ASSERT_STREQ(label, l);

    const char *field = "name";
    ASSERT_FALSE(Index_ContainsField(idx, field));
    Index_AddField(idx, field);
    Index_AddField(idx, field);
    ASSERT_TRUE(Index_ContainsField(idx, field));

    field = "age";
    ASSERT_FALSE(Index_ContainsField(idx, field));
    Index_AddField(idx, field);
    Index_AddField(idx, field);
    ASSERT_TRUE(Index_ContainsField(idx, field));

    // Returns number of fields indexed.
    uint field_count = Index_FieldsCount(idx);
    ASSERT_EQ(field_count, 2);

    // Returns indexed fields.
    const char **fields = Index_GetFields(idx);
    ASSERT_STREQ(fields[0], "name");
    ASSERT_STREQ(fields[1], "age");

    Index_RemoveField(idx, "age");
    Index_RemoveField(idx, "age");
    ASSERT_FALSE(Index_ContainsField(idx, "age"));
    ASSERT_TRUE(Index_ContainsField(idx, "name"));

    Index_RemoveField(idx, "name");
    Index_RemoveField(idx, "name");
    ASSERT_FALSE(Index_ContainsField(idx, "age"));
    ASSERT_FALSE(Index_ContainsField(idx, "name"));

    Index_Free(idx);
}
