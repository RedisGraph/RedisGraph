/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../src/value.h"
#include "../../src/query_ctx.h"
#include "../../src/arithmetic/funcs.h"
#include "../../src/arithmetic/arithmetic_expression.h"
#include "../../src/graph/entities/node.h"
#include "../../src/execution_plan/execution_plan.h"
#include "../../src/util/rmalloc.h"
#include "../../src/util/arr.h"
#include "../../src/datatypes/array.h"
#include "../../src/errors.h"
#include <time.h>
#include <math.h>

// Declaration of function in execution_plan.h
AR_ExpNode **_BuildProjectionExpressions(const cypher_astnode_t *ret_clause, AST *ast);

#ifdef __cplusplus
}
#endif

class ArithmeticTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {
		// Use the malloc family for allocations
		Alloc_Reset();

		ASSERT_TRUE(ErrorCtx_Init());

		// Prepare thread-local variables
		ASSERT_TRUE(QueryCtx_Init());

		// Register functions
		AR_RegisterFuncs();
	}

	static void TearDownTestCase() {
	}
};

void _test_string(const AR_ExpNode *exp, const char *expected) {
	char *str;
	AR_EXP_ToString(exp, &str);
	ASSERT_STREQ(str, expected);
	free(str);
}

void _test_ar_func(AR_ExpNode *root, SIValue expected) {
	SIValue res = AR_EXP_Evaluate(root, NULL);
	if(SI_TYPE(res) == T_NULL && SI_TYPE(expected) == T_NULL) {
		// NULLs implicitly match
		return;
	} else if(SI_TYPE(res) & SI_NUMERIC && SI_TYPE(expected) & SI_NUMERIC) {
		// Compare numerics by internal value
		ASSERT_EQ(SI_GET_NUMERIC(res), SI_GET_NUMERIC(expected));
	} else {
		FAIL() << "Tried to compare disjoint types";
	}
}

AR_ExpNode *_exp_from_query(const char *query) {
	cypher_parse_result_t *parse_result = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
	AST *ast = AST_Build(parse_result);
	ast->referenced_entities = raxNew();

	const cypher_astnode_t *ret_clause = AST_GetClause(ast, CYPHER_AST_RETURN,
													   NULL);
	return _BuildProjectionExpressions(ret_clause, ast)[0];
}

TEST_F(ArithmeticTest, RandomUUID) {
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;
	char v;

	query = "RETURN randomUUID()";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(36, strlen(result.stringval));
	ASSERT_EQ('-', result.stringval[8]);
	ASSERT_EQ('-', result.stringval[13]);
	ASSERT_EQ('4', result.stringval[14]);
	ASSERT_EQ('-', result.stringval[18]);
	v = result.stringval[19];
	ASSERT_TRUE(v == '8' || v == '9' || v == 'a' || v == 'b');
	ASSERT_EQ('-', result.stringval[23]);
	AR_EXP_Free(arExp);
}

TEST_F(ArithmeticTest, ReduceTest) {
	const char *query;
	AR_ExpNode *arExp;

	// Validate reduce to a constant value in applicable case.
	query = "RETURN 1 IN [1,2,3]";
	arExp = _exp_from_query(query);
	ASSERT_EQ(AR_EXP_OPERAND, arExp->type);
	ASSERT_EQ(AR_EXP_CONSTANT, arExp->operand.type);
	ASSERT_EQ(0, SIValue_Compare(SI_BoolVal(true), arExp->operand.constant, NULL));

	// Validate partial reduce of the AR_Exp tree. Reduce only applicable nodes.
	query = "RETURN rand() IN [1,2,3]";
	arExp = _exp_from_query(query);

	ASSERT_EQ(AR_EXP_OP, arExp->type);
	ASSERT_EQ(2, arExp->op.child_count);

	AR_ExpNode *lhs = arExp->op.children[0];
	ASSERT_EQ(AR_EXP_OP, lhs->type);
	ASSERT_EQ(0, strcasecmp("rand", AR_EXP_GetFuncName(lhs)));

	AR_ExpNode *rhs = arExp->op.children[1];
	ASSERT_EQ(AR_EXP_OPERAND, rhs->type);
	ASSERT_EQ(AR_EXP_CONSTANT, rhs->operand.type);
	ASSERT_EQ(T_ARRAY, rhs->operand.constant.type);
}