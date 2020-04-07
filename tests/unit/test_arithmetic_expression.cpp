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
#include "../../src/arithmetic/agg_funcs.h"
#include "../../src/execution_plan/record.h"
#include "../../src/execution_plan/execution_plan.h"
#include "../../src/util/rmalloc.h"
#include "../../src/util/arr.h"
#include "../../src/datatypes/array.h"
#include <time.h>

// Declaration of function in execution_plan.h
AR_ExpNode **_BuildReturnExpressions(const cypher_astnode_t *ret_clause, AST *ast);

#ifdef __cplusplus
}
#endif

class ArithmeticTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {
		// Use the malloc family for allocations
		Alloc_Reset();

		// Prepare thread-local variables
		ASSERT_TRUE(QueryCtx_Init());

		// Register functions
		AR_RegisterFuncs();
		Agg_RegisterFuncs();
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

void _test_ar_func(AR_ExpNode *root, SIValue expected, const Record r) {
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

	const cypher_astnode_t *ret_clause = AST_GetClause(ast, CYPHER_AST_RETURN);
	return _BuildReturnExpressions(ret_clause, ast)[0];
}

TEST_F(ArithmeticTest, ExpressionTest) {
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;

	/* muchacho */
	query = "RETURN 'muchacho'";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_STREQ(result.stringval, "muchacho");
	AR_EXP_Free(arExp);

	/* 1 */
	query = "RETURN 1";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(result.longval, 1);
	AR_EXP_Free(arExp);

	/* 1+2*3 */
	query = "RETURN 1+2*3";
	arExp = _exp_from_query(query);
	// Entire expression should be reduce to a single constant.
	ASSERT_EQ(arExp->type, AR_EXP_OPERAND);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(result.longval, 7);
	AR_EXP_Free(arExp);

	/* 1 + 1 + 1 + 1 + 1 + 1 */
	query = "RETURN 1 + 1 + 1 + 1 + 1 + 1";
	arExp = _exp_from_query(query);
	ASSERT_EQ(arExp->type, AR_EXP_OPERAND);

	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(result.longval, 6);
	AR_EXP_Free(arExp);

	/* ABS(-5 + 2 * 1) */
	query = "RETURN ABS(-5 + 2 * 1)";
	arExp = _exp_from_query(query);
	ASSERT_EQ(arExp->type, AR_EXP_OPERAND);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(result.longval, 3);
	AR_EXP_Free(arExp);

	/* 'a' + 'b' */
	query = "RETURN 'a' + 'b'";
	arExp = _exp_from_query(query);
	ASSERT_EQ(arExp->type, AR_EXP_OPERAND);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_TRUE(strcmp(result.stringval, "ab") == 0);
	AR_EXP_Free(arExp);

	/* 1 + 2 + 'a' + 2 + 1 */
	query = "RETURN 1 + 2 + 'a' + 2 + 1";
	arExp = _exp_from_query(query);
	ASSERT_EQ(arExp->type, AR_EXP_OPERAND);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_TRUE(strcmp(result.stringval, "3a21") == 0);
	AR_EXP_Free(arExp);

	/* 2 * 2 + 'a' + 3 * 3 */
	query = "RETURN 2 * 2 + 'a' + 3 * 3";
	arExp = _exp_from_query(query);
	ASSERT_EQ(arExp->type, AR_EXP_OPERAND);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_TRUE(strcmp(result.stringval, "4a9") == 0);
	AR_EXP_Free(arExp);

	query = "RETURN 9 % 5";
	arExp = _exp_from_query(query);
	ASSERT_EQ(arExp->type, AR_EXP_OPERAND);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(result.longval, 4);
	AR_EXP_Free(arExp);

	query = "RETURN 9 % 5 % 3";
	arExp = _exp_from_query(query);
	ASSERT_EQ(arExp->type, AR_EXP_OPERAND);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(result.longval, 1);
	AR_EXP_Free(arExp);

}

TEST_F(ArithmeticTest, NullArithmetic) {
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;

	/* null + 1 */
	query = "RETURN null + 1";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_TRUE(SIValue_IsNull(result));
	AR_EXP_Free(arExp);

	/* 1 + null */
	query = "RETURN 1 + null";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_TRUE(SIValue_IsNull(result));
	AR_EXP_Free(arExp);

	/* null - 1 */
	query = "RETURN null - 1";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_TRUE(SIValue_IsNull(result));
	AR_EXP_Free(arExp);

	/* 1 - null */
	query = "RETURN 1 - null";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_TRUE(SIValue_IsNull(result));
	AR_EXP_Free(arExp);

	/* null * 1 */
	query = "RETURN null * 1";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_TRUE(SIValue_IsNull(result));
	AR_EXP_Free(arExp);

	/* 1 * null */
	query = "RETURN 1 * null";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_TRUE(SIValue_IsNull(result));
	AR_EXP_Free(arExp);

	/* null / 1 */
	query = "RETURN null / 1";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_TRUE(SIValue_IsNull(result));
	AR_EXP_Free(arExp);

	/* 1 / null */
	query = "RETURN 1 / null";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_TRUE(SIValue_IsNull(result));
	AR_EXP_Free(arExp);

	query = "RETURN 5 % null";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_TRUE(SIValue_IsNull(result));
	AR_EXP_Free(arExp);

	query = "RETURN null % 5";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_TRUE(SIValue_IsNull(result));
	AR_EXP_Free(arExp);

}

TEST_F(ArithmeticTest, AggregateTest) {
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;

	/* SUM(1) */
	query = "RETURN SUM(1)";
	arExp = _exp_from_query(query);

	AR_EXP_Aggregate(arExp, NULL);
	AR_EXP_Aggregate(arExp, NULL);
	AR_EXP_Aggregate(arExp, NULL);
	AR_EXP_Reduce(arExp);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(result.doubleval, 3);
	AR_EXP_Free(arExp);

	/* 2+SUM(1) */
	query = "RETURN 2+SUM(1)";
	arExp = _exp_from_query(query);
	AR_EXP_Aggregate(arExp, NULL);
	AR_EXP_Aggregate(arExp, NULL);
	AR_EXP_Aggregate(arExp, NULL);
	AR_EXP_Reduce(arExp);
	/* Just for the kick of it, call reduce more than once.*/
	AR_EXP_Reduce(arExp);

	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(result.doubleval, 5);
	AR_EXP_Free(arExp);
}

TEST_F(ArithmeticTest, AbsTest) {
	const char *query;
	AR_ExpNode *arExp;

	/* ABS(1) */
	query = "RETURN ABS(1)";
	arExp = _exp_from_query(query);
	SIValue expected = SI_LongVal(1);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* ABS(-1) */
	query = "RETURN ABS(-1)";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(1);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* ABS(0) */
	query = "RETURN ABS(0)";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(0);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* ABS() */
	query = "RETURN ABS(NULL)";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);
}

TEST_F(ArithmeticTest, CeilTest) {
	SIValue expected;
	const char *query;
	AR_ExpNode *arExp;

	/* CEIL(0.5) */
	query = "RETURN CEIL(0.5)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(1);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* CEIL(1) */
	query = "RETURN CEIL(1)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(1);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* CEIL(0.1) */
	query = "RETURN CEIL(0.1)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(1);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* CEIL() */
	query = "RETURN CEIL(NULL)";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);
}

TEST_F(ArithmeticTest, FloorTest) {
	SIValue expected;
	const char *query;
	AR_ExpNode *arExp;

	/* FLOOR(0.5) */
	query = "RETURN FLOOR(0.5)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(0);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* FLOOR(1) */
	query = "RETURN FLOOR(1)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(1);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* FLOOR(0.1) */
	query = "RETURN FLOOR(0.1)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(0);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* FLOOR() */
	query = "RETURN FLOOR(NULL)";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);
}

TEST_F(ArithmeticTest, RoundTest) {
	SIValue expected;
	const char *query;
	AR_ExpNode *arExp;

	/* ROUND(0) */
	query = "RETURN ROUND(0)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(0);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* ROUND(0.49) */
	query = "RETURN ROUND(0.49)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(0);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* ROUND(0.5) */
	query = "RETURN ROUND(0.5)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(1);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* ROUND(1) */
	query = "RETURN ROUND(1)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(1);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* ROUND() */
	query = "RETURN ROUND(NULL)";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);
}

TEST_F(ArithmeticTest, SignTest) {
	SIValue expected;
	const char *query;
	AR_ExpNode *arExp;

	/* SIGN(0) */
	query = "RETURN SIGN(0)";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(0);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* SIGN(-1) */
	query = "RETURN SIGN(-1)";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(-1);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* SIGN(1) */
	query = "RETURN SIGN(1)";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(1);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* SIGN() */
	query = "RETURN SIGN(NULL)";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);
}

TEST_F(ArithmeticTest, ToIntegerTest) {
	SIValue expected;
	const char *query;
	AR_ExpNode *arExp;

	/* toInteger(1) */
	query = "RETURN toInteger(1)";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(1);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* toInteger(1.1) */
	query = "RETURN toInteger(1.1)";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(1);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* toInteger(1.9) */
	query = "RETURN toInteger(1.9)";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(1);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* toInteger('1') */
	query = "RETURN toInteger('1')";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(1);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* toInteger('1.1') */
	query = "RETURN toInteger('1.1')";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(1);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* toInteger('1.9') */
	query = "RETURN toInteger('1.9')";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(1);
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* toInteger('z') */
	query = "RETURN toInteger('z')";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);

	/* toInteger(NULL) */
	query = "RETURN toInteger(null)";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected, NULL);
	AR_EXP_Free(arExp);
}

TEST_F(ArithmeticTest, ReverseTest) {
	SIValue result;
	const char *expected;
	const char *query;
	AR_ExpNode *arExp;

	/* REVERSE("muchacho") */
	query = "RETURN REVERSE('muchacho')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "ohcahcum";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* REVERSE("") */
	query = "RETURN REVERSE('')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* REVERSE() */
	query = "RETURN REVERSE(NULL)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(result.type, T_NULL);
	AR_EXP_Free(arExp);
}

TEST_F(ArithmeticTest, LeftTest) {
	SIValue result;
	const char *expected;
	const char *query;
	AR_ExpNode *arExp;

	/* LEFT("muchacho", 4) */
	query = "RETURN LEFT('muchacho', 4)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "much";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* LEFT("muchacho", 100) */
	query = "RETURN LEFT('muchacho', 100)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* LEFT(NULL, 100) */
	query = "RETURN LEFT(NULL, 100)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(result.type, T_NULL);
	AR_EXP_Free(arExp);
}

TEST_F(ArithmeticTest, RightTest) {
	SIValue result;
	const char *expected;
	const char *query;
	AR_ExpNode *arExp;

	/* RIGHT("muchacho", 4) */
	query = "RETURN RIGHT('muchacho', 4)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "acho";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* RIGHT("muchacho", 100) */
	query = "RETURN RIGHT('muchacho', 100)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* RIGHT(NULL, 100) */
	query = "RETURN RIGHT(NULL, 100)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(result.type, T_NULL);
	AR_EXP_Free(arExp);
}

TEST_F(ArithmeticTest, LTrimTest) {
	SIValue result;
	const char *expected;
	const char *query;
	AR_ExpNode *arExp;

	/* lTrim("   muchacho") */
	query = "RETURN lTrim('   muchacho')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* lTrim("muchacho   ") */
	query = "RETURN lTrim('muchacho   ')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho   ";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* lTrim("   much   acho   ") */
	query = "RETURN lTrim('   much   acho   ')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "much   acho   ";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* lTrim("muchacho") */
	query = "RETURN lTrim('muchacho')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* lTrim() */
	query = "RETURN lTrim(NULL)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(result.type, T_NULL);
	AR_EXP_Free(arExp);
}

TEST_F(ArithmeticTest, RTrimTest) {
	SIValue result;
	const char *expected;
	const char *query;
	AR_ExpNode *arExp;

	/* rTrim("   muchacho") */
	query = "RETURN rTrim('   muchacho')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "   muchacho";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* rTrim("muchacho   ") */
	query = "RETURN rTrim('muchacho   ')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* rTrim("   much   acho   ") */
	query = "RETURN rTrim('   much   acho   ')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "   much   acho";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* rTrim("muchacho") */
	query = "RETURN rTrim('muchacho')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* rTrim() */
	query = "RETURN rTrim(NULL)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(result.type, T_NULL);
	AR_EXP_Free(arExp);
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

TEST_F(ArithmeticTest, TrimTest) {
	SIValue result;
	const char *expected;
	const char *query;
	AR_ExpNode *arExp;

	/* trim("   muchacho") */
	query = "RETURN trim('   muchacho')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* trim("muchacho   ") */
	query = "RETURN trim('muchacho   ')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* trim("   much   acho   ") */
	query = "RETURN trim('   much   acho   ')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "much   acho";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* trim("muchacho") */
	query = "RETURN trim('muchacho')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* trim() */
	query = "RETURN trim(NULL)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(result.type, T_NULL);
	AR_EXP_Free(arExp);
}

TEST_F(ArithmeticTest, SubstringTest) {
	SIValue result;
	const char *expected;
	const char *query;
	AR_ExpNode *arExp;

	/* SUBSTRING("muchacho", 0, 4) */
	query = "RETURN SUBSTRING('muchacho', 0, 4)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "much";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* SUBSTRING("muchacho", 3, 20) */
	query = "RETURN SUBSTRING('muchacho', 3, 20)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "hacho";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* SUBSTRING(NULL, 3, 20) */
	query = "RETURN SUBSTRING(NULL, 3, 20)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(result.type, T_NULL);
	AR_EXP_Free(arExp);
}

TEST_F(ArithmeticTest, ToLowerTest) {
	SIValue result;
	const char *expected;
	const char *query;
	AR_ExpNode *arExp;

	/* toLower("MuChAcHo") */
	query = "RETURN toLower('MuChAcHo')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* toLower("mUcHaChO") */
	query = "RETURN toLower('mUcHaChO')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* toLower("mUcHaChO") */
	query = "RETURN toLower(NULL)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(result.type, T_NULL);
	AR_EXP_Free(arExp);
}

TEST_F(ArithmeticTest, ToUpperTest) {
	SIValue result;
	const char *expected;
	const char *query;
	AR_ExpNode *arExp;

	/* toUpper("MuChAcHo") */
	query = "RETURN toUpper('MuChAcHo')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "MUCHACHO";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* toUpper("mUcHaChO") */
	query = "RETURN toUpper('mUcHaChO')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "MUCHACHO";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* toUpper("mUcHaChO") */
	query = "RETURN toUpper(NULL)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(result.type, T_NULL);
	AR_EXP_Free(arExp);
}

TEST_F(ArithmeticTest, ToStringTest) {
	SIValue result;
	const char *expected;
	const char *query;
	AR_ExpNode *arExp;

	/* toString("muchacho") */
	query = "RETURN toString('muchacho')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* toString("3.14") */
	query = "RETURN toString(3.14)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "3.140000";
	ASSERT_STREQ(result.stringval, expected);
	AR_EXP_Free(arExp);

	/* toString() */
	query = "RETURN toString(NULL)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(result.type, T_NULL);
	AR_EXP_Free(arExp);
}

TEST_F(ArithmeticTest, ExistsTest) {
	/* Although EXISTS is supposed to be called
	* using entity alias and property, to make things easy
	* within a unit-test context we simply pass an evaluation
	* of an expression such as n.v, `null` when property `v`
	* isn't in `n` and `1` when n.v evaluates to 1. */

	SIValue result;
	const char *query;
	AR_ExpNode *arExp;

	/* Pass NULL indicating n.v doesn't exists. */
	query = "RETURN EXISTS(null)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(result.type, T_BOOL);
	ASSERT_EQ(result.longval, 0);
	AR_EXP_Free(arExp);

	/* Pass 1, in case n.v exists and evaluates to 1. */
	query = "RETURN EXISTS(1)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(result.type, T_BOOL);
	ASSERT_EQ(result.longval, 1);
	AR_EXP_Free(arExp);
}

TEST_F(ArithmeticTest, TimestampTest) {
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);

	query = "RETURN timestamp()";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	ASSERT_LE(abs((1000 * ts.tv_sec + ts.tv_nsec / 1000000) - result.longval), 5);
}

TEST_F(ArithmeticTest, CaseTest) {
	SIValue result;
	SIValue expected = SI_LongVal(2);
	const char *query;
	AR_ExpNode *arExp;

	/* Test "Simple form"
	 * Match one of the alternatives. */
	query = "RETURN CASE 'brown' WHEN 'blue' THEN 1+0 WHEN 'brown' THEN 2-0 ELSE 3*1 END";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	AR_EXP_Free(arExp);
	ASSERT_EQ(result.longval, expected.longval);

	/* Do not match any of the alternatives, return default. */
	query = "RETURN CASE 'green' WHEN 'blue' THEN 1+0 WHEN 'brown' THEN 2-0 ELSE 3*1 END";
	expected = SI_LongVal(3);
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	AR_EXP_Free(arExp);
	ASSERT_EQ(result.longval, expected.longval);

	/* Test "Generic form"
	 * One of the alternatives evaluates to a none null value.
	 * Default not specified. */
	query = "RETURN CASE WHEN NULL THEN 1+0 WHEN true THEN 2-0 END";
	expected = SI_LongVal(2);
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	AR_EXP_Free(arExp);
	ASSERT_EQ(result.longval, expected.longval);

	/* None of the alternatives evaluates to a none null value.
	 * Default specified, expecting default. */
	query = "RETURN CASE WHEN NULL THEN 1+0 WHEN NULL THEN 2-0 ELSE 3*1 END";
	expected = SI_LongVal(3);
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	AR_EXP_Free(arExp);
	ASSERT_EQ(result.longval, expected.longval);

	/* None of the alternatives evaluates to a none null value.
	 * Default not specified, expecting NULL */
	query = "RETURN CASE WHEN NULL THEN 1+0 WHEN NULL THEN 2-0 END";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	AR_EXP_Free(arExp);
	ASSERT_TRUE(SIValue_IsNull(result));
}

TEST_F(ArithmeticTest, AND) {
	SIValue truth_table[27] = {
		SI_ConstStringVal("false"), SI_ConstStringVal("false"), SI_BoolVal(false),
		SI_ConstStringVal("false"), SI_ConstStringVal("true"), SI_BoolVal(false),
		SI_ConstStringVal("false"), SI_NullVal(), SI_BoolVal(false),
		SI_ConstStringVal("true"), SI_ConstStringVal("false"), SI_BoolVal(false),
		SI_ConstStringVal("true"), SI_ConstStringVal("true"), SI_BoolVal(true),
		SI_ConstStringVal("true"), SI_NullVal(), SI_NullVal(),
		SI_NullVal(), SI_ConstStringVal("false"), SI_BoolVal(false),
		SI_NullVal(), SI_ConstStringVal("true"), SI_NullVal(),
		SI_NullVal(), SI_NullVal(), SI_NullVal()
	};

	for(int i = 0; i < 27; i += 3) {
		SIValue a = truth_table[i];
		SIValue b = truth_table[i + 1];
		SIValue expected = truth_table[i + 2];

		char *query;
		asprintf(&query, "RETURN %s AND %s", a.stringval, b.stringval);
		AR_ExpNode *arExp = _exp_from_query(query);
		SIValue result = AR_EXP_Evaluate(arExp, NULL);
		AR_EXP_Free(arExp);

		ASSERT_EQ(SI_TYPE(result), SI_TYPE(expected));
		if(SI_TYPE(result) != T_NULL) {
			ASSERT_EQ(result.longval, expected.longval);
		}
	}
}

TEST_F(ArithmeticTest, OR) {
	SIValue truth_table[27] = {
		SI_ConstStringVal("false"), SI_ConstStringVal("false"), SI_BoolVal(false),
		SI_ConstStringVal("false"), SI_ConstStringVal("true"), SI_BoolVal(true),
		SI_ConstStringVal("false"), SI_NullVal(), SI_NullVal(),
		SI_ConstStringVal("true"), SI_ConstStringVal("false"), SI_BoolVal(true),
		SI_ConstStringVal("true"), SI_ConstStringVal("true"), SI_BoolVal(true),
		SI_ConstStringVal("true"), SI_NullVal(), SI_BoolVal(true),
		SI_NullVal(), SI_ConstStringVal("false"), SI_NullVal(),
		SI_NullVal(), SI_ConstStringVal("true"), SI_BoolVal(true),
		SI_NullVal(), SI_NullVal(), SI_NullVal()
	};

	for(int i = 0; i < 27; i += 3) {
		SIValue a = truth_table[i];
		SIValue b = truth_table[i + 1];
		SIValue expected = truth_table[i + 2];

		char *query;
		asprintf(&query, "RETURN %s OR %s", a.stringval, b.stringval);
		AR_ExpNode *arExp = _exp_from_query(query);
		SIValue result = AR_EXP_Evaluate(arExp, NULL);
		AR_EXP_Free(arExp);

		ASSERT_EQ(SI_TYPE(result), SI_TYPE(expected));
		if(SI_TYPE(result) != T_NULL) {
			ASSERT_EQ(result.longval, expected.longval);
		}
	}
}

TEST_F(ArithmeticTest, XOR) {
	SIValue truth_table[27] = {
		SI_ConstStringVal("false"), SI_ConstStringVal("false"), SI_BoolVal(false),
		SI_ConstStringVal("false"), SI_ConstStringVal("true"), SI_BoolVal(true),
		SI_ConstStringVal("false"), SI_NullVal(), SI_NullVal(),
		SI_ConstStringVal("true"), SI_ConstStringVal("false"), SI_BoolVal(true),
		SI_ConstStringVal("true"), SI_ConstStringVal("true"), SI_BoolVal(false),
		SI_ConstStringVal("true"), SI_NullVal(), SI_NullVal(),
		SI_NullVal(), SI_ConstStringVal("false"), SI_NullVal(),
		SI_NullVal(), SI_ConstStringVal("true"), SI_NullVal(),
		SI_NullVal(), SI_NullVal(), SI_NullVal()
	};

	for(int i = 0; i < 27; i += 3) {
		SIValue a = truth_table[i];
		SIValue b = truth_table[i + 1];
		SIValue expected = truth_table[i + 2];

		char *query;
		asprintf(&query, "RETURN %s XOR %s", a.stringval, b.stringval);
		AR_ExpNode *arExp = _exp_from_query(query);
		SIValue result = AR_EXP_Evaluate(arExp, NULL);
		AR_EXP_Free(arExp);

		ASSERT_EQ(SI_TYPE(result), SI_TYPE(expected));
		if(SI_TYPE(result) != T_NULL) {
			ASSERT_EQ(result.longval, expected.longval);
		}
	}
}

TEST_F(ArithmeticTest, NOT) {
	SIValue truth_table[6] = {
		SI_ConstStringVal("false"), SI_BoolVal(true),
		SI_ConstStringVal("true"), SI_BoolVal(false),
		SI_NullVal(), SI_NullVal()
	};

	for(int i = 0; i < 6; i += 2) {
		SIValue a = truth_table[i];
		SIValue expected = truth_table[i + 1];

		char *query;
		asprintf(&query, "RETURN NOT %s", a.stringval);
		AR_ExpNode *arExp = _exp_from_query(query);
		SIValue result = AR_EXP_Evaluate(arExp, NULL);
		AR_EXP_Free(arExp);

		ASSERT_EQ(SI_TYPE(result), SI_TYPE(expected));
		if(SI_TYPE(result) != T_NULL) {
			ASSERT_EQ(result.longval, expected.longval);
		}
	}
}

TEST_F(ArithmeticTest, LT) {
	SIValue truth_table[27] = {
		SI_ConstStringVal("1"), SI_ConstStringVal("1"), SI_BoolVal(false),
		SI_ConstStringVal("1"), SI_ConstStringVal("2"), SI_BoolVal(true),
		SI_ConstStringVal("1"), SI_NullVal(), SI_NullVal(),
		SI_ConstStringVal("2"), SI_ConstStringVal("1"), SI_BoolVal(false),
		SI_ConstStringVal("2"), SI_ConstStringVal("2"), SI_BoolVal(false),
		SI_ConstStringVal("2"), SI_NullVal(), SI_NullVal(),
		SI_NullVal(), SI_ConstStringVal("1"), SI_NullVal(),
		SI_NullVal(), SI_ConstStringVal("2"), SI_NullVal(),
		SI_NullVal(), SI_NullVal(), SI_NullVal()
	};

	for(int i = 0; i < 27; i += 3) {
		SIValue a = truth_table[i];
		SIValue b = truth_table[i + 1];
		SIValue expected = truth_table[i + 2];

		char *query;
		asprintf(&query, "RETURN %s < %s", a.stringval, b.stringval);
		AR_ExpNode *arExp = _exp_from_query(query);
		SIValue result = AR_EXP_Evaluate(arExp, NULL);
		AR_EXP_Free(arExp);

		ASSERT_EQ(SI_TYPE(result), SI_TYPE(expected));
		if(SI_TYPE(result) != T_NULL) {
			ASSERT_EQ(result.longval, expected.longval);
		}
	}
}

TEST_F(ArithmeticTest, LE) {
	SIValue truth_table[27] = {
		SI_ConstStringVal("1"), SI_ConstStringVal("1"), SI_BoolVal(true),
		SI_ConstStringVal("1"), SI_ConstStringVal("2"), SI_BoolVal(true),
		SI_ConstStringVal("1"), SI_NullVal(), SI_NullVal(),
		SI_ConstStringVal("2"), SI_ConstStringVal("1"), SI_BoolVal(false),
		SI_ConstStringVal("2"), SI_ConstStringVal("2"), SI_BoolVal(true),
		SI_ConstStringVal("2"), SI_NullVal(), SI_NullVal(),
		SI_NullVal(), SI_ConstStringVal("1"), SI_NullVal(),
		SI_NullVal(), SI_ConstStringVal("2"), SI_NullVal(),
		SI_NullVal(), SI_NullVal(), SI_NullVal()
	};

	for(int i = 0; i < 27; i += 3) {
		SIValue a = truth_table[i];
		SIValue b = truth_table[i + 1];
		SIValue expected = truth_table[i + 2];

		char *query;
		asprintf(&query, "RETURN %s <= %s", a.stringval, b.stringval);
		AR_ExpNode *arExp = _exp_from_query(query);
		SIValue result = AR_EXP_Evaluate(arExp, NULL);
		AR_EXP_Free(arExp);

		ASSERT_EQ(SI_TYPE(result), SI_TYPE(expected));
		if(SI_TYPE(result) != T_NULL) {
			ASSERT_EQ(result.longval, expected.longval);
		}
	}
}

TEST_F(ArithmeticTest, EQ) {
	SIValue truth_table[27] = {
		SI_ConstStringVal("1"), SI_ConstStringVal("1"), SI_BoolVal(true),
		SI_ConstStringVal("1"), SI_ConstStringVal("2"), SI_BoolVal(false),
		SI_ConstStringVal("1"), SI_NullVal(), SI_NullVal(),
		SI_ConstStringVal("2"), SI_ConstStringVal("1"), SI_BoolVal(false),
		SI_ConstStringVal("2"), SI_ConstStringVal("2"), SI_BoolVal(true),
		SI_ConstStringVal("2"), SI_NullVal(), SI_NullVal(),
		SI_NullVal(), SI_ConstStringVal("1"), SI_NullVal(),
		SI_NullVal(), SI_ConstStringVal("2"), SI_NullVal(),
		SI_NullVal(), SI_NullVal(), SI_NullVal()
	};

	for(int i = 0; i < 27; i += 3) {
		SIValue a = truth_table[i];
		SIValue b = truth_table[i + 1];
		SIValue expected = truth_table[i + 2];

		char *query;
		asprintf(&query, "RETURN %s = %s", a.stringval, b.stringval);
		AR_ExpNode *arExp = _exp_from_query(query);
		SIValue result = AR_EXP_Evaluate(arExp, NULL);
		AR_EXP_Free(arExp);

		ASSERT_EQ(SI_TYPE(result), SI_TYPE(expected));
		if(SI_TYPE(result) != T_NULL) {
			ASSERT_EQ(result.longval, expected.longval);
		}
	}
}

TEST_F(ArithmeticTest, NE) {
	SIValue truth_table[27] = {
		SI_ConstStringVal("1"), SI_ConstStringVal("1"), SI_BoolVal(false),
		SI_ConstStringVal("1"), SI_ConstStringVal("2"), SI_BoolVal(true),
		SI_ConstStringVal("1"), SI_NullVal(), SI_NullVal(),
		SI_ConstStringVal("2"), SI_ConstStringVal("1"), SI_BoolVal(true),
		SI_ConstStringVal("2"), SI_ConstStringVal("2"), SI_BoolVal(false),
		SI_ConstStringVal("2"), SI_NullVal(), SI_NullVal(),
		SI_NullVal(), SI_ConstStringVal("1"), SI_NullVal(),
		SI_NullVal(), SI_ConstStringVal("2"), SI_NullVal(),
		SI_NullVal(), SI_NullVal(), SI_NullVal()
	};

	for(int i = 0; i < 27; i += 3) {
		SIValue a = truth_table[i];
		SIValue b = truth_table[i + 1];
		SIValue expected = truth_table[i + 2];

		char *query;
		asprintf(&query, "RETURN %s <> %s", a.stringval, b.stringval);
		AR_ExpNode *arExp = _exp_from_query(query);
		SIValue result = AR_EXP_Evaluate(arExp, NULL);
		AR_EXP_Free(arExp);

		ASSERT_EQ(SI_TYPE(result), SI_TYPE(expected));
		if(SI_TYPE(result) != T_NULL) {
			ASSERT_EQ(result.longval, expected.longval);
		}
	}
}
TEST_F(ArithmeticTest, ListTest) {
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;

	query = "RETURN [1,2.3,'4',True,False, null]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	ASSERT_EQ(T_ARRAY, result.type);

	SIValue longVal = SIArray_Get(result, 0);
	SIValue doubleVal = SIArray_Get(result, 1);
	SIValue stringVal = SIArray_Get(result, 2);
	SIValue trueVal = SIArray_Get(result, 3);
	SIValue falseVal = SIArray_Get(result, 4);
	SIValue nullVal = SIArray_Get(result, 5);

	ASSERT_EQ(T_INT64, longVal.type);
	ASSERT_EQ(1, longVal.longval);

	ASSERT_EQ(T_DOUBLE, doubleVal.type);
	ASSERT_EQ(2.3, doubleVal.doubleval);

	ASSERT_EQ(T_STRING, stringVal.type);
	ASSERT_EQ(0, strcmp("4", stringVal.stringval));

	ASSERT_EQ(T_BOOL, trueVal.type);
	ASSERT_EQ(true, trueVal.longval);

	ASSERT_EQ(T_BOOL, falseVal.type);
	ASSERT_EQ(false, falseVal.longval);

	ASSERT_TRUE(SIValue_IsNull(nullVal));
}

TEST_F(ArithmeticTest, ListSliceTest) {
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;

	query = "RETURN [0,1,2,3,4,5,6,7,8,9,10][3]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	ASSERT_EQ(T_INT64, result.type);
	ASSERT_EQ(3, result.longval);

	query = "RETURN [0,1,2,3,4,5,6,7,8,9,10][-3]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	ASSERT_EQ(T_INT64, result.type);
	ASSERT_EQ(8, result.longval);

	query = "RETURN [0,1,2,3,4,5,6,7,8,9,10][0..3]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	ASSERT_EQ(T_ARRAY, result.type);
	ASSERT_EQ(3, SIArray_Length(result));

	for(int i = 0; i < 3; i++) {
		SIValue value = SIArray_Get(result, i);
		ASSERT_EQ(T_INT64, value.type);
		ASSERT_EQ(i, value.longval);
	}

	query = "RETURN [0,1,2,3,4,5,6,7,8,9,10][0..-5]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	ASSERT_EQ(T_ARRAY, result.type);
	ASSERT_EQ(6, SIArray_Length(result));

	for(int i = 0; i < 6; i++) {
		SIValue value = SIArray_Get(result, i);
		ASSERT_EQ(T_INT64, value.type);
		ASSERT_EQ(i, value.longval);
	}

	query = "RETURN [0,1,2,3,4,5,6,7,8,9,10][-5..]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	ASSERT_EQ(T_ARRAY, result.type);
	ASSERT_EQ(5, SIArray_Length(result));

	for(int i = 0; i < 5; i++) {
		SIValue value = SIArray_Get(result, i);
		ASSERT_EQ(T_INT64, value.type);
		ASSERT_EQ(i + 6, value.longval);
	}

	query = "RETURN [0,1,2,3,4,5,6,7,8,9,10][..4]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	ASSERT_EQ(T_ARRAY, result.type);
	ASSERT_EQ(4, SIArray_Length(result));

	for(int i = 0; i < 4; i++) {
		SIValue value = SIArray_Get(result, i);
		ASSERT_EQ(T_INT64, value.type);
		ASSERT_EQ(i, value.longval);
	}
}

TEST_F(ArithmeticTest, RangeTest) {
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;

	// create range from 0 to 10 [0,1,2,3,4,5,6,7,8,9,10]
	query = "RETURN range(0,10)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	ASSERT_EQ(T_ARRAY, result.type);
	ASSERT_EQ(11, SIArray_Length(result));

	for(int i = 0; i < 11; i++) {
		SIValue value = SIArray_Get(result, i);
		ASSERT_EQ(T_INT64, value.type);
		ASSERT_EQ(i, value.longval);
	}

	// creae ragne with skips of 3, statring from 2 to 18 [2,5,8,11,14,17]
	query = "RETURN range(2,18,3)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	ASSERT_EQ(T_ARRAY, result.type);
	ASSERT_EQ(6, SIArray_Length(result));

	for(int i = 0; i < 6; i++) {
		SIValue value = SIArray_Get(result, i);
		ASSERT_EQ(T_INT64, value.type);
		ASSERT_EQ(i * 3 + 2, value.longval);
	}
}

TEST_F(ArithmeticTest, InTest) {
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;

	// check if 3 in [1,2,3]
	query = "RETURN 3 IN [1,2,3]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	ASSERT_EQ(T_BOOL, result.type);
	ASSERT_EQ(true, result.longval);

	// check if 4 in [1,2,3]
	query = "RETURN 4 IN [1,2,3]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	ASSERT_EQ(T_BOOL, result.type);
	ASSERT_EQ(false, result.longval);

	// check if [1,2] in [1,2,3]
	query = "RETURN [1,2] IN [1,2,3]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	ASSERT_EQ(T_BOOL, result.type);
	ASSERT_EQ(false, result.longval);

	// check if [1,2] in [[1,2],3]
	query = "RETURN [1,2] IN [[1,2],3]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	ASSERT_EQ(T_BOOL, result.type);
	ASSERT_EQ(true, result.longval);
}

TEST_F(ArithmeticTest, IsNullTest) {
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;

	// Check if null is null.
	query = "RETURN null IS NULL";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	ASSERT_EQ(T_BOOL, result.type);
	ASSERT_EQ(true, result.longval);

	// Check if null is not "not null".
	query = "RETURN null IS NOT NULL";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	ASSERT_EQ(T_BOOL, result.type);
	ASSERT_EQ(false, result.longval);

	// Check for different types values.
	char *values[6] = {"1", "1.2", "true", "false", "'string'", "[1,2,3]"};
	for(int i = 0; i < 6; i++) {
		char buff[128];
		// Check if value is not null.
		sprintf(buff, "RETURN %s IS NOT NULL", values[i]);
		arExp = _exp_from_query(buff);
		result = AR_EXP_Evaluate(arExp, NULL);

		ASSERT_EQ(T_BOOL, result.type);
		ASSERT_EQ(true, result.longval);

		sprintf(buff, "RETURN %s IS NULL", values[i]);
		arExp = _exp_from_query(buff);
		result = AR_EXP_Evaluate(arExp, NULL);

		ASSERT_EQ(T_BOOL, result.type);
		ASSERT_EQ(false, result.longval);
	}
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
	ASSERT_EQ(0, strcasecmp("rand", lhs->op.func_name));

	AR_ExpNode *rhs = arExp->op.children[1];
	ASSERT_EQ(AR_EXP_OPERAND, rhs->type);
	ASSERT_EQ(AR_EXP_CONSTANT, rhs->operand.type);
	ASSERT_EQ(T_ARRAY, rhs->operand.constant.type);
}

TEST_F(ArithmeticTest, CoalesceTest) {
	const char *query;
	AR_ExpNode *arExp;

	// Test reduction of coalesce over static values.
	query = "RETURN coalesce(1)";
	arExp = _exp_from_query(query);
	ASSERT_EQ(AR_EXP_OPERAND, arExp->type);
	ASSERT_EQ(AR_EXP_CONSTANT, arExp->operand.type);
	ASSERT_EQ(0, SIValue_Compare(SI_LongVal(1), arExp->operand.constant, NULL));

	// Test reduction of coalesce over static values.
	query = "RETURN coalesce(null, 1)";
	arExp = _exp_from_query(query);
	ASSERT_EQ(AR_EXP_OPERAND, arExp->type);
	ASSERT_EQ(AR_EXP_CONSTANT, arExp->operand.type);
	ASSERT_EQ(0, SIValue_Compare(SI_LongVal(1), arExp->operand.constant, NULL));
}
