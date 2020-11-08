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
#include "../../src/util/rmalloc.h"
#include "../../src/util/arr.h"
#include "../../src/ast/ast.h"

#ifdef __cplusplus
}
#endif

class TestReferencedEntities: public ::testing::Test {
  protected:
	static void SetUpTestCase() {// Use the malloc family for allocations
		Alloc_Reset();
	}
};

// Parse query to AST.
AST *buildAST(const char *query) {
	cypher_parse_result_t *parse_result = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
	return AST_Build(parse_result);
}

uint *getASTSegmentIndices(AST *ast) {
	// Retrieve the indices of each WITH clause to properly set the bounds of each segment.
	uint *segment_indices = AST_GetClauseIndices(ast, CYPHER_AST_WITH);
	segment_indices = array_append(segment_indices, cypher_ast_query_nclauses(ast->root));
	return segment_indices;
}

TEST_F(TestReferencedEntities, TestMatch) {
	// Test for no inline filters.
	char *q = "MATCH (n)";
	AST *ast = buildAST(q);
	uint *segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));

	AST *astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(0, raxSize(astSegment->referenced_entities));
	AST_Free(astSegment);

	q = "MATCH (n)-[e]->(m)";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(0, raxSize(astSegment->referenced_entities));
	AST_Free(astSegment);

	// Inline node label filter.
	q = "MATCH (n:X)-[e]->(m)";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(0, raxSize(astSegment->referenced_entities));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);

	// Inline node property filter.
	q = "MATCH (n {val:42})-[e]->(m)";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(1, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);

	// Inline edge relationship type.
	q = "MATCH (n)-[e:T]->(m)";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(0, raxSize(astSegment->referenced_entities));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);

	// Inline edge properties.
	q = "MATCH (n)-[e {val:42}]->(m)";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(1, raxSize(astSegment->referenced_entities));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);

	// Explicit filters: EQ, NE, GT, LT, IS NULL, IS NOT NULL.
	q = "MATCH (n)-[e]->(m) WHERE n.v = 1";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(1, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);

	q = "MATCH (n)-[e]->(m) WHERE n.v <> 1";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(1, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);

	q = "MATCH (n)-[e]->(m) WHERE n.v > 1";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(1, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);

	q = "MATCH (n)-[e]->(m) WHERE n.v < 1";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(1, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);

	q = "MATCH (n)-[e]->(m) WHERE n.v IS NULL 1";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(1, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);

	q = "MATCH (n)-[e]->(m) WHERE n.v IS NOT NULL 1";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(1, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);
}

TEST_F(TestReferencedEntities, TestSet) {
	// Simple match and set property.
	char *q = "MATCH (n) SET n.v=1";
	AST *ast = buildAST(q);
	uint *segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));

	AST *astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(1, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	AST_Free(astSegment);

	// Path match and set property.
	q = "MATCH (n)-[e]->(x)-[]->(m) SET n.v=x.v";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(2, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"x", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);

	// Reference from MATCH and SET caluse.
	q = "MATCH (n)-[e:r]->(m) SET n.v=1";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(1, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);
}

TEST_F(TestReferencedEntities, TestMerge) {
	// Simple merge.
	char *q = "MERGE (n)";
	AST *ast = buildAST(q);
	uint *segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));

	AST *astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(0, raxSize(astSegment->referenced_entities));
	AST_Free(astSegment);

	q = "MERGE (n)-[e]->(m)";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(0, raxSize(astSegment->referenced_entities));
	AST_Free(astSegment);

	// Inline node label filter.
	q = "MERGE (n:X)-[e]->(m)";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(0, raxSize(astSegment->referenced_entities));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);

	// Inline node property filter.
	q = "MERGE (n {val:42})-[e]->(m)";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(1, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);

	// Inline edge relationship type.
	q = "MERGE (n)-[e:T]->(m)";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(0, raxSize(astSegment->referenced_entities));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);

	// Inline edge properties.
	q = "MERGE (n)-[e {val:42}]->(m)";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(1, raxSize(astSegment->referenced_entities));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);


	// On match.
	q = "MERGE (n)-[e]->(m) ON MATCH SET n.v=1, m.v=2";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(2, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);

	// On create.
	q = "MERGE (n)-[e]->(m) ON CREATE SET n.v=1, m.v=2";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(2, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);

	// Mixed.
	q = "MERGE (n)-[e]->(m) ON CREATE SET n.v=1, m.v=2 ON MATCH SET e.v = 3";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(3, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);
}

TEST_F(TestReferencedEntities, TestUnwind) {
	char *q = "UNWIND [1,2] as x";
	AST *ast = buildAST(q);
	uint *segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));

	AST *astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(0, raxSize(astSegment->referenced_entities));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"x", 1));
	AST_Free(astSegment);
}

TEST_F(TestReferencedEntities, TestWith) {
	char *q = "MATCH (n),(m) WITH n as x RETURN x";
	AST *ast = buildAST(q);
	uint *segmentIndices = getASTSegmentIndices(ast);
	// Two segments: first is the MATCH clause, the second is the WITH clause.
	ASSERT_EQ(2, array_len(segmentIndices));

	// Only n is projected from the first segment.
	AST *astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(1, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"x", 1));
	AST_Free(astSegment);

	// Only x is projected from the second segment.
	astSegment = AST_NewSegment(ast, segmentIndices[0], segmentIndices[1]);
	ASSERT_EQ(1, raxSize(astSegment->referenced_entities));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"x", 1));
	AST_Free(astSegment);

	q = "MATCH (n),(m) WITH n as x ORDER BY m.value RETURN x";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	// Two segments: first is the MATCH clause, the second is the WITH clause.
	ASSERT_EQ(2, array_len(segmentIndices));

	// Only n and m projected from the first segment.
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(2, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"x", 1));
	AST_Free(astSegment);

	// Only m and x projected from the second segment.
	astSegment = AST_NewSegment(ast, segmentIndices[0], segmentIndices[1]);
	ASSERT_EQ(2, raxSize(astSegment->referenced_entities));
	ASSERT_EQ(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"x", 1));
	AST_Free(astSegment);
}

TEST_F(TestReferencedEntities, TestReturn) {
	char *q = "MATCH (n),(m) RETURN n as x";
	AST *ast = buildAST(q);
	uint *segmentIndices = getASTSegmentIndices(ast);
	// One segment containing MATCH and RETURN clause.
	ASSERT_EQ(1, array_len(segmentIndices));

	// n and x are both accessible in the only segment.
	AST *astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(2, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"x", 1));
	AST_Free(astSegment);

	q = "MATCH (n),(m) RETURN n as x ORDER BY m.value";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	// One segment containing MATCH and RETURN clause.
	ASSERT_EQ(1, array_len(segmentIndices));

	// All variables are accessible in the only segment.
	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(3, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"x", 1));
	AST_Free(astSegment);
}

TEST_F(TestReferencedEntities, TestNamedPath) {
	char *q = "MATCH p=()";
	AST *ast = buildAST(q);
	uint *segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));

	AST *astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(1, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"anon_0", 6));
	AST_Free(astSegment);

	q = "MATCH p =()-[]-()";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));

	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(3, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"anon_0", 6));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"anon_1", 6));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"anon_2", 6));
	AST_Free(astSegment);

	q = "MATCH p =(n)-[e]-(m)";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));

	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(3, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"n", 1));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);

	q = "MATCH p =(:Label1 {value:1})-[e:Rel*]-(m:Label2 {value:2})";
	ast = buildAST(q);
	segmentIndices = getASTSegmentIndices(ast);
	ASSERT_EQ(1, array_len(segmentIndices));

	astSegment = AST_NewSegment(ast, 0, segmentIndices[0]);
	ASSERT_EQ(3, raxSize(astSegment->referenced_entities));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"anon_0", 6));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"e", 1));
	ASSERT_NE(raxNotFound, raxFind(astSegment->referenced_entities, (unsigned char *)"m", 1));
	AST_Free(astSegment);

}

