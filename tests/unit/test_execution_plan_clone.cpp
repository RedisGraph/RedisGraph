/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include "../../src/util/arr.h"
#include "../../src/query_ctx.h"
#include "../../src/util/rmalloc.h"
#include "../../src/arithmetic/funcs.h"
#include "../../src/arithmetic/agg_funcs.h"
#include "../../src/procedures/procedure.h"
#include "../../src/execution_plan/execution_plan_clone.h"

#ifdef __cplusplus
}
#endif

class ExecutionPlanCloneTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {
		// Use the malloc family for allocations
		Alloc_Reset();
		// Init query context.
		ASSERT_TRUE(QueryCtx_Init());
		// Initialize GraphBLAS.
		GrB_init(GrB_NONBLOCKING);
		GxB_Global_Option_set(GxB_FORMAT, GxB_BY_COL); // all matrices in CSC format
		GxB_Global_Option_set(GxB_HYPER, GxB_NEVER_HYPER); // matrices are never hypersparse
		Proc_Register();         // Register procedures.
		AR_RegisterFuncs();      // Register arithmetic functions.
		Agg_RegisterFuncs();     // Register aggregation functions.

		// Create a graphcontext
		_fake_graph_context();
	}

	static void build_ast_and_plan(const char *query, AST **ast, ExecutionPlan **plan) {
		cypher_parse_result_t *parse_result = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
		*ast = AST_Build(parse_result);
		*plan = NewExecutionPlan();
	}

	static void _fake_graph_context() {
		GraphContext *gc = (GraphContext *)malloc(sizeof(GraphContext));

		gc->g = Graph_New(16, 16);
		gc->index_count = 0;
		gc->graph_name = strdup("G");
		gc->attributes = raxNew();
		pthread_rwlock_init(&gc->_attribute_rwlock, NULL);
		gc->string_mapping = (char **)array_new(char *, 64);
		gc->node_schemas = (Schema **)array_new(Schema *, GRAPH_DEFAULT_LABEL_CAP);
		gc->relation_schemas = (Schema **)array_new(Schema *, GRAPH_DEFAULT_RELATION_TYPE_CAP);
		QueryCtx_SetGraphCtx(gc);
	}

	static void ExecutionPlan_OpsEqual(const ExecutionPlan *plan_a, const ExecutionPlan *plan_b,
									   const OpBase *op_a, const OpBase *op_b) {
		// If both ops are NULL, there is nothing to compare.
		if(op_a == NULL && op_b == NULL) return;
		// In case one of the ops is NULL.
		ASSERT_TRUE(op_a && op_b);
		// In case both ops are not in their respective segment, there is nothing to compare.
		if(op_a->plan != plan_a && op_b->plan != plan_b) return;
		ASSERT_TRUE(op_a->plan == plan_a && op_b->plan == plan_b);
		ASSERT_EQ(op_a->type, op_b->type);
		ASSERT_EQ(op_a->childCount, op_b->childCount);
		for(uint i = 0; i < op_a->childCount; i++) {
			ExecutionPlan_OpsEqual(plan_a, plan_b, op_a->children[i], op_b->children[i]);
		}
	}

	/* Execution plan cloning clones the following:
	 * 1. Plan segments
	 * 2. Plan operations
	 * 3. Plan record mapping
	 * 4. Query graph and connected components.
	 * As query graph cloning and rax cloning are tested and proven, this function
	 * tests only the segments and operations cloning. */
	static void ExecutionPlan_Equal(const ExecutionPlan *plan_a, const ExecutionPlan *plan_b) {

		ASSERT_TRUE(plan_a->is_union == plan_b->is_union);
		uint plan_a_segment_count = array_len(plan_a->segments);
		uint plan_b_segment_count = array_len(plan_b->segments);
		ASSERT_EQ(plan_a_segment_count, plan_b_segment_count);
		for(uint i = 0; i < plan_a_segment_count; i++) {
			ExecutionPlan_Equal(plan_a->segments[i], plan_b->segments[i]);
		}
		ExecutionPlan_OpsEqual(plan_a, plan_b, plan_a->root, plan_b->root);
	}

	static void validate_query_plans_clone(const char **queries) {
		uint query_count = array_len(queries);
		for(uint i = 0; i < query_count; i++) {
			const char *query = queries[i];
			AST *ast = NULL;
			ExecutionPlan *plan = NULL;
			build_ast_and_plan(query, &ast, &plan);
			ASSERT_TRUE(ast);
			ASSERT_TRUE(plan);
			ExecutionPlan *clone = ExecutionPlan_Clone(plan);
			ExecutionPlan_Equal(plan, clone);
			AST_Free(ast);
			ExecutionPlan_Free(clone);
			ExecutionPlan_Free(plan);
		}
	}
};

TEST_F(ExecutionPlanCloneTest, TestCreateClause) {
	const char **queries = array_new(const char *, 12);
	// Anonymous nodes create clauses.
	queries = array_append(queries, "CREATE ()");
	queries = array_append(queries, "CREATE (:N)");
	queries = array_append(queries, "CREATE (:N {val:1})");
	// Referenced nodes create clauses.
	queries = array_append(queries, "CREATE (n) RETURN n");
	queries = array_append(queries, "CREATE (n:N) RETURN n");
	queries = array_append(queries, "CREATE (n:N {val:1}) RETURN n");

	// Anonymous edges create clauses.
	queries = array_append(queries, "CREATE ()-[]->()");
	queries = array_append(queries, "CREATE ()-[:E]->()");
	queries = array_append(queries, "CREATE ()-[:E {val:1}]->()");
	// Referenced edges create clauses.
	queries = array_append(queries, "CREATE ()-[e]->() RETURN e");
	queries = array_append(queries, "CREATE ()-[e:E]->() RETURN e");
	queries = array_append(queries, "CREATE ()-[e:E {val:1}]->()");
	validate_query_plans_clone(queries);
	array_free(queries);
}

TEST_F(ExecutionPlanCloneTest, TestMatchClause) {
	const char **queries = array_new(const char *, 9);
	queries = array_append(queries, "MATCH (n) RETURN n");  // All node scan
	queries = array_append(queries, "MATCH (n:N) RETURN n");    // Label scan
	queries = array_append(queries, "MATCH (n) WHERE id(n) = 0 RETURN n");  // ID Scan
	queries = array_append(queries,
						   "MATCH (n)-[]->() RETURN n");    // Conditional traverse, referenced src node.
	queries = array_append(queries,
						   "MATCH (n)-[e]->() RETURN n");   // Conditional traverse, referenced src node and edge.
	queries = array_append(queries,
						   "MATCH p = ()-[]->() RETURN p"); // Named path, conditional traverse
	queries = array_append(queries,
						   "MATCH (n)-[*]->() RETURN n");   // Variable length traverse.
	queries = array_append(queries,
						   "MATCH p = ()-[*]->() return p");    // Named path, variable length traverse.
	queries = array_append(queries,
						   "MATCH (n) WHERE (n)-[:R]->() AND NOT (n)-[:R2)->() RETURN n");   // Apply ops.

	validate_query_plans_clone(queries);
	array_free(queries);
}

TEST_F(ExecutionPlanCloneTest, TestUpdateClause) {
	const char **queries = array_new(const char *, 2);
	queries = array_append(queries, "MATCH (n) SET n.v = 1");
	queries = array_append(queries, "MATCH ()-[e]->() SET e.v = 1");

	validate_query_plans_clone(queries);
	array_free(queries);
}

TEST_F(ExecutionPlanCloneTest, TestDeleteClause) {
	const char **queries = array_new(const char *, 2);
	queries = array_append(queries, "MATCH (n) DELETE n");
	queries = array_append(queries, "MATCH ()-[e]->() DELETE e");

	validate_query_plans_clone(queries);
	array_free(queries);
}

TEST_F(ExecutionPlanCloneTest, TestMergeClause) {
	const char **queries = array_new(const char *, 9);
	queries = array_append(queries, "MERGE ()");
	queries = array_append(queries, "MERGE (:N)");
	queries = array_append(queries, "MERGE (:N {val:1})");

	queries = array_append(queries, "MERGE (n) ON MATCH SET n.val2 = 2");
	queries = array_append(queries, "MERGE (n:N) ON MATCH SET n.val2 = 2");
	queries = array_append(queries, "MERGE (n:N {val:1}) ON MATCH SET n.val2 = 2");

	queries = array_append(queries, "MERGE (n) ON CREATE SET n.val2 = 2");
	queries = array_append(queries, "MERGE (n:N) ON CREATE SET n.val2 = 2");
	queries = array_append(queries, "MERGE (n:N {val:1}) ON CREATE SET n.val2 = 2");

	validate_query_plans_clone(queries);
	array_free(queries);
}

TEST_F(ExecutionPlanCloneTest, TestCartesianProduct) {
	const char **queries = array_new(const char *, 1);
	queries = array_append(queries, "MATCH (a), (b) RETURN a, b");

	validate_query_plans_clone(queries);
	array_free(queries);
}

TEST_F(ExecutionPlanCloneTest, TestSkipLimitSort) {
	const char **queries = array_new(const char *, 5);
	queries = array_append(queries, "MATCH (n) RETURN n SKIP 5");
	queries = array_append(queries, "MATCH (n) RETURN n LIMIT 5");
	queries = array_append(queries, "MATCH (n) RETURN n SKIP 5 LIMIT 5");
	queries = array_append(queries, "MATCH (n) RETURN n ORDER BY n.val");
	queries = array_append(queries, "MATCH (n) RETURN n ORDER BY n.val SKIP 5 LIMIT 5");

	validate_query_plans_clone(queries);
	array_free(queries);
}

TEST_F(ExecutionPlanCloneTest, TestOptionalMatch) {
	const char **queries = array_new(const char *, 3);
	queries = array_append(queries, "OPTIONAL MATCH (n) RETURN n");
	queries = array_append(queries, "MATCH (a) OPTIONAL MATCH (b) RETURN a, b");
	queries = array_append(queries, "MATCH (a) OPTIONAL MATCH (a)-[e]->(b) RETURN a, e, b");

	validate_query_plans_clone(queries);
	array_free(queries);
}

TEST_F(ExecutionPlanCloneTest, TestProcCall) {
	const char **queries = array_new(const char *, 1);
	queries = array_append(queries,
						   "CALL db.idx.fulltext.queryNodes('fruit', 'Orange*') YIELD node RETURN node");

	validate_query_plans_clone(queries);
	array_free(queries);
}

TEST_F(ExecutionPlanCloneTest, TestUnwind) {
	const char **queries = array_new(const char *, 1);
	queries = array_append(queries,
						   "UNWIND [1,2,3] as x RETURN x");

	validate_query_plans_clone(queries);
	array_free(queries);
}

TEST_F(ExecutionPlanCloneTest, TestWith) {
	const char **queries = array_new(const char *, 6);
	queries = array_append(queries,
						   "MATCH (n) WITH n RETURN n");
	queries = array_append(queries,
						   "MATCH (n) WITH n AS m RETURN m");
	queries = array_append(queries,
						   "MATCH (n) WITH n AS m SKIP 5 RETURN m");
	queries = array_append(queries,
						   "MATCH (n) WITH n AS m LIMIT 5 RETURN m");
	queries = array_append(queries,
						   "MATCH (n) WITH n AS m ORDER BY n.val RETURN m");
	queries = array_append(queries,
						   "MATCH (n) WITH n AS m WHERE n.val < 5 RETURN m");

	validate_query_plans_clone(queries);
	array_free(queries);
}

TEST_F(ExecutionPlanCloneTest, TestUnion) {
	const char **queries = array_new(const char *, 1);
	queries = array_append(queries,
						   "MATCH (n) RETURN n UNION MATCH (n) RETURN n");

	validate_query_plans_clone(queries);
	array_free(queries);
}

