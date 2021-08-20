/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./acutest.h"
#include "../../src/value.h"
#include "../../src/query_ctx.h"
#include "../../src/arithmetic/funcs.h"
#include "../../src/arithmetic/arithmetic_expression.h"
#include "../../src/graph/entities/node.h"
#include "../../src/execution_plan/execution_plan.h"
#include "../../src/util/rmalloc.h"
#include "../../src/util/arr.h"
#include "../../src/datatypes/array.h"
#include <time.h>
#include <math.h>

// Declaration of function in execution_plan.h
AR_ExpNode **_BuildProjectionExpressions(const cypher_astnode_t *ret_clause, AST *ast);

bool initialized = false;

void setup(void) {
	if(initialized) return;
	initialized = true;

	// Use the malloc family for allocations
	Alloc_Reset();

	// Prepare thread-local variables
	TEST_CHECK(QueryCtx_Init());

	// Register functions
	AR_RegisterFuncs();
}

void _test_string(const AR_ExpNode *exp, const char *expected) {
	char *str;
	AR_EXP_ToString(exp, &str);
	TEST_CHECK(!strcmp(str, expected));
	free(str);
}

void _test_ar_func(AR_ExpNode *root, SIValue expected) {
	SIValue res = AR_EXP_Evaluate(root, NULL);
	if(SI_TYPE(res) == T_NULL && SI_TYPE(expected) == T_NULL) {
		// NULLs implicitly match
		return;
	} else if(SI_TYPE(res) & SI_NUMERIC && SI_TYPE(expected) & SI_NUMERIC) {
		// Compare numerics by internal value
		TEST_CHECK(SI_GET_NUMERIC(res) == SI_GET_NUMERIC(expected));
	} else {
		TEST_CHECK(false);
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

void test_Expression(void) {
	setup();
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;

	/* muchacho */
	query = "RETURN 'muchacho'";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(!strcmp(result.stringval, "muchacho"));
	AR_EXP_Free(arExp);

	/* 1 */
	query = "RETURN 1";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(result.longval == 1);
	AR_EXP_Free(arExp);

	/* 1+2*3 */
	query = "RETURN 1+2*3";
	arExp = _exp_from_query(query);
	// Entire expression should be reduce to a single constant.
	TEST_CHECK(arExp->type == AR_EXP_OPERAND);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(result.longval == 7);
	AR_EXP_Free(arExp);

	/* 1 + 1 + 1 + 1 + 1 + 1 */
	query = "RETURN 1 + 1 + 1 + 1 + 1 + 1";
	arExp = _exp_from_query(query);
	TEST_CHECK(arExp->type == AR_EXP_OPERAND);

	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(result.longval == 6);
	AR_EXP_Free(arExp);

	/* ABS(-5 + 2 * 1) */
	query = "RETURN ABS(-5 + 2 * 1)";
	arExp = _exp_from_query(query);
	TEST_CHECK(arExp->type == AR_EXP_OPERAND);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(result.longval == 3);
	AR_EXP_Free(arExp);

	/* 'a' + 'b' */
	query = "RETURN 'a' + 'b'";
	arExp = _exp_from_query(query);
	TEST_CHECK(arExp->type == AR_EXP_OPERAND);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(!strcmp(result.stringval, "ab"));
	AR_EXP_Free(arExp);

	/* 1 + 2 + 'a' + 2 + 1 */
	query = "RETURN 1 + 2 + 'a' + 2 + 1";
	arExp = _exp_from_query(query);
	TEST_CHECK(arExp->type == AR_EXP_OPERAND);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(!strcmp(result.stringval, "3a21"));
	AR_EXP_Free(arExp);

	/* 2 * 2 + 'a' + 3 * 3 */
	query = "RETURN 2 * 2 + 'a' + 3 * 3";
	arExp = _exp_from_query(query);
	TEST_CHECK(arExp->type == AR_EXP_OPERAND);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(!strcmp(result.stringval, "4a9"));
	AR_EXP_Free(arExp);

	query = "RETURN 9 % 5";
	arExp = _exp_from_query(query);
	TEST_CHECK(arExp->type == AR_EXP_OPERAND);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(result.longval == 4);
	AR_EXP_Free(arExp);

	query = "RETURN 9 % 5 % 3";
	arExp = _exp_from_query(query);
	TEST_CHECK(arExp->type == AR_EXP_OPERAND);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(result.longval == 1);
	AR_EXP_Free(arExp);

}

void test_NullArithmetic(void) {
	setup();
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;

	/* null + 1 */
	query = "RETURN null + 1";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(SIValue_IsNull(result));
	AR_EXP_Free(arExp);

	/* 1 + null */
	query = "RETURN 1 + null";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(SIValue_IsNull(result));
	AR_EXP_Free(arExp);

	/* null - 1 */
	query = "RETURN null - 1";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(SIValue_IsNull(result));
	AR_EXP_Free(arExp);

	/* 1 - null */
	query = "RETURN 1 - null";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(SIValue_IsNull(result));
	AR_EXP_Free(arExp);

	/* null * 1 */
	query = "RETURN null * 1";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(SIValue_IsNull(result));
	AR_EXP_Free(arExp);

	/* 1 * null */
	query = "RETURN 1 * null";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(SIValue_IsNull(result));
	AR_EXP_Free(arExp);

	/* null / 1 */
	query = "RETURN null / 1";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(SIValue_IsNull(result));
	AR_EXP_Free(arExp);

	/* 1 / null */
	query = "RETURN 1 / null";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(SIValue_IsNull(result));
	AR_EXP_Free(arExp);

	query = "RETURN 5 % null";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(SIValue_IsNull(result));
	AR_EXP_Free(arExp);

	query = "RETURN null % 5";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(SIValue_IsNull(result));
	AR_EXP_Free(arExp);

}

void test_Aggregate(void) {
	setup();
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;

	/* SUM(1) */
	query = "RETURN SUM(1)";
	arExp = _exp_from_query(query);

	AR_EXP_Aggregate(arExp, NULL);
	AR_EXP_Aggregate(arExp, NULL);
	AR_EXP_Aggregate(arExp, NULL);
	result = AR_EXP_Finalize(arExp, NULL);
	TEST_CHECK(result.doubleval == 3);
	AR_EXP_Free(arExp);

	/* 2+SUM(1) */
	query = "RETURN 2+SUM(1)";
	arExp = _exp_from_query(query);
	AR_EXP_Aggregate(arExp, NULL);
	AR_EXP_Aggregate(arExp, NULL);
	AR_EXP_Aggregate(arExp, NULL);
	result = AR_EXP_Finalize(arExp, NULL);
	TEST_CHECK(result.doubleval == 5);
	AR_EXP_Free(arExp);
}

void test_Abs(void) {
	setup();
	const char *query;
	AR_ExpNode *arExp;

	/* ABS(1) */
	query = "RETURN ABS(1)";
	arExp = _exp_from_query(query);
	SIValue expected = SI_LongVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* ABS(-1) */
	query = "RETURN ABS(-1)";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* ABS(0) */
	query = "RETURN ABS(0)";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(0);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* ABS() */
	query = "RETURN ABS(NULL)";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);
}

void test_Ceil(void) {
	setup();
	SIValue expected;
	const char *query;
	AR_ExpNode *arExp;

	/* CEIL(0.5) */
	query = "RETURN CEIL(0.5)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* CEIL(1) */
	query = "RETURN CEIL(1)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* CEIL(0.1) */
	query = "RETURN CEIL(0.1)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* CEIL() */
	query = "RETURN CEIL(NULL)";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);
}

void test_Floor(void) {
	setup();
	SIValue expected;
	const char *query;
	AR_ExpNode *arExp;

	/* FLOOR(0.5) */
	query = "RETURN FLOOR(0.5)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(0);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* FLOOR(1) */
	query = "RETURN FLOOR(1)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* FLOOR(0.1) */
	query = "RETURN FLOOR(0.1)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(0);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* FLOOR() */
	query = "RETURN FLOOR(NULL)";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);
}

void test_Round(void) {
	setup();
	SIValue expected;
	const char *query;
	AR_ExpNode *arExp;

	/* ROUND(0) */
	query = "RETURN ROUND(0)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(0);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* ROUND(0.49) */
	query = "RETURN ROUND(0.49)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(0);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* ROUND(0.5) */
	query = "RETURN ROUND(0.5)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* ROUND(1) */
	query = "RETURN ROUND(1)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* ROUND() */
	query = "RETURN ROUND(NULL)";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);
}

void test_Sign(void) {
	setup();
	SIValue expected;
	const char *query;
	AR_ExpNode *arExp;

	/* SIGN(0) */
	query = "RETURN SIGN(0)";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(0);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* SIGN(-1) */
	query = "RETURN SIGN(-1)";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(-1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* SIGN(1) */
	query = "RETURN SIGN(1)";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* SIGN() */
	query = "RETURN SIGN(NULL)";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);
}

void test_Sqrt(void) {
	setup();
	SIValue expected;
	const char *query;
	AR_ExpNode *arExp;

	/* SQRT(1) */
	query = "RETURN sqrt(1)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* SQRT(0) */
	query = "RETURN sqrt(0)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(0);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* SQRT(4) */
	query = "RETURN sqrt(4)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(2);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* SQRT(-4) */
	query = "RETURN sqrt(-4)";
	arExp = _exp_from_query(query);
	SIValue res = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(isnan(SI_GET_NUMERIC(res)));
	AR_EXP_Free(arExp);

	/* SQRT(2.5) */
	query = "RETURN sqrt(2.5)";
	arExp = _exp_from_query(query);
	res = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(abs(1.58 - SI_GET_NUMERIC(res)) < 0.01);
	AR_EXP_Free(arExp);

	/* SQRT() */
	query = "RETURN sqrt(NULL)";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);
}

void test_Pow(void) {
	setup();
	SIValue expected;
	const char *query;
	AR_ExpNode *arExp;

	/* POW(1,0) */
	query = "RETURN pow(1,0)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* 1^0 */
	query = "RETURN 1^0";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* POW(0,1) */
	query = "RETURN pow(0,1)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(0);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* 0^1 */
	query = "RETURN 0^1";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(0);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* POW(0,0) */
	query = "RETURN pow(0,0)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* 0^0 */
	query = "RETURN 0^0";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* POW(2,3) */
	query = "RETURN pow(2,3)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(8);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* 2^3 */
	query = "RETURN 2^3";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(8);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* POW(2,-3) */
	query = "RETURN pow(2,-3)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(0.125);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* 2^-3 */
	query = "RETURN 2^-3";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(0.125);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* 2^(-3) */
	query = "RETURN 2^(-3)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(0.125);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* POW(0.5,2) */
	query = "RETURN pow(0.5,2)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(0.25);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* 0.5^2 */
	query = "RETURN 0.5^2";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(0.25);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* POW(-1,2) */
	query = "RETURN pow(-1,2)";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* -1^2 */
	query = "RETURN -1^2";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* (-1)^2 */
	query = "RETURN (-1)^2";
	arExp = _exp_from_query(query);
	expected = SI_DoubleVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* POW() */
	query = "RETURN pow(NULL,1)";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* NULL^1 */
	query = "RETURN NULL^1";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* POW() */
	query = "RETURN pow(1,NULL)";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* 1^NULL */
	query = "RETURN 1^NULL";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* POW() */
	query = "RETURN pow(NULL,NULL)";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* NULL^NULL */
	query = "RETURN NULL^NULL";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);
}

void test_ToInteger(void) {
	setup();
	SIValue expected;
	const char *query;
	AR_ExpNode *arExp;

	/* toInteger(1) */
	query = "RETURN toInteger(1)";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* toInteger(1.1) */
	query = "RETURN toInteger(1.1)";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* toInteger(1.9) */
	query = "RETURN toInteger(1.9)";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* toInteger('1') */
	query = "RETURN toInteger('1')";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* toInteger('1.1') */
	query = "RETURN toInteger('1.1')";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* toInteger('1.9') */
	query = "RETURN toInteger('1.9')";
	arExp = _exp_from_query(query);
	expected = SI_LongVal(1);
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* toInteger('z') */
	query = "RETURN toInteger('z')";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);

	/* toInteger(NULL) */
	query = "RETURN toInteger(null)";
	arExp = _exp_from_query(query);
	expected = SI_NullVal();
	_test_ar_func(arExp, expected);
	AR_EXP_Free(arExp);
}

void test_Reverse(void) {
	setup();
	SIValue result;
	const char *expected;
	const char *query;
	AR_ExpNode *arExp;

	/* REVERSE("muchacho") */
	query = "RETURN REVERSE('muchacho')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "ohcahcum";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* REVERSE("") */
	query = "RETURN REVERSE('')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* REVERSE() */
	query = "RETURN REVERSE(NULL)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(result.type == T_NULL);
	AR_EXP_Free(arExp);
}

void test_Left(void) {
	setup();
	SIValue result;
	const char *expected;
	const char *query;
	AR_ExpNode *arExp;

	/* LEFT("muchacho", 4) */
	query = "RETURN LEFT('muchacho', 4)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "much";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* LEFT("muchacho", 100) */
	query = "RETURN LEFT('muchacho', 100)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* LEFT(NULL, 100) */
	query = "RETURN LEFT(NULL, 100)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(result.type == T_NULL);
	AR_EXP_Free(arExp);
}

void test_Right(void) {
	setup();
	SIValue result;
	const char *expected;
	const char *query;
	AR_ExpNode *arExp;

	/* RIGHT("muchacho", 4) */
	query = "RETURN RIGHT('muchacho', 4)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "acho";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* RIGHT("muchacho", 100) */
	query = "RETURN RIGHT('muchacho', 100)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* RIGHT(NULL, 100) */
	query = "RETURN RIGHT(NULL, 100)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(result.type == T_NULL);
	AR_EXP_Free(arExp);
}

void test_LTrim(void) {
	setup();
	SIValue result;
	const char *expected;
	const char *query;
	AR_ExpNode *arExp;

	/* lTrim("   muchacho") */
	query = "RETURN lTrim('   muchacho')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* lTrim("muchacho   ") */
	query = "RETURN lTrim('muchacho   ')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho   ";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* lTrim("   much   acho   ") */
	query = "RETURN lTrim('   much   acho   ')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "much   acho   ";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* lTrim("muchacho") */
	query = "RETURN lTrim('muchacho')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* lTrim() */
	query = "RETURN lTrim(NULL)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(result.type == T_NULL);
	AR_EXP_Free(arExp);
}

void test_RTrim(void) {
	setup();
	SIValue result;
	const char *expected;
	const char *query;
	AR_ExpNode *arExp;

	/* rTrim("   muchacho") */
	query = "RETURN rTrim('   muchacho')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "   muchacho";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* rTrim("muchacho   ") */
	query = "RETURN rTrim('muchacho   ')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* rTrim("   much   acho   ") */
	query = "RETURN rTrim('   much   acho   ')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "   much   acho";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* rTrim("muchacho") */
	query = "RETURN rTrim('muchacho')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* rTrim() */
	query = "RETURN rTrim(NULL)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(result.type == T_NULL);
	AR_EXP_Free(arExp);
}

void test_RandomUUID(void) {
	setup();
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;
	char v;

	query = "RETURN randomUUID()";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(36 == strlen(result.stringval));
	TEST_CHECK('-' == result.stringval[8]);
	TEST_CHECK('-' == result.stringval[13]);
	TEST_CHECK('4' == result.stringval[14]);
	TEST_CHECK('-' == result.stringval[18]);
	v = result.stringval[19];
	TEST_CHECK(v == '8' || v == '9' || v == 'a' || v == 'b');
	TEST_CHECK('-' == result.stringval[23]);
	AR_EXP_Free(arExp);
}

void test_Trim(void) {
	setup();
	SIValue result;
	const char *expected;
	const char *query;
	AR_ExpNode *arExp;

	/* trim("   muchacho") */
	query = "RETURN trim('   muchacho')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* trim("muchacho   ") */
	query = "RETURN trim('muchacho   ')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* trim("   much   acho   ") */
	query = "RETURN trim('   much   acho   ')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "much   acho";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* trim("muchacho") */
	query = "RETURN trim('muchacho')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* trim() */
	query = "RETURN trim(NULL)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(result.type == T_NULL);
	AR_EXP_Free(arExp);
}

void test_Substring(void) {
	setup();
	SIValue result;
	const char *expected;
	const char *query;
	AR_ExpNode *arExp;

	/* SUBSTRING("muchacho", 0, 4) */
	query = "RETURN SUBSTRING('muchacho', 0, 4)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "much";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* SUBSTRING("muchacho", 3, 20) */
	query = "RETURN SUBSTRING('muchacho', 3, 20)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "hacho";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* SUBSTRING(NULL, 3, 20) */
	query = "RETURN SUBSTRING(NULL, 3, 20)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(result.type == T_NULL);
	AR_EXP_Free(arExp);
}

void test_ToLower(void) {
	setup();
	SIValue result;
	const char *expected;
	const char *query;
	AR_ExpNode *arExp;

	/* toLower("MuChAcHo") */
	query = "RETURN toLower('MuChAcHo')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* toLower("mUcHaChO") */
	query = "RETURN toLower('mUcHaChO')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* toLower("mUcHaChO") */
	query = "RETURN toLower(NULL)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(result.type == T_NULL);
	AR_EXP_Free(arExp);
}

void test_ToUpper(void) {
	setup();
	SIValue result;
	const char *expected;
	const char *query;
	AR_ExpNode *arExp;

	/* toUpper("MuChAcHo") */
	query = "RETURN toUpper('MuChAcHo')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "MUCHACHO";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* toUpper("mUcHaChO") */
	query = "RETURN toUpper('mUcHaChO')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "MUCHACHO";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* toUpper("mUcHaChO") */
	query = "RETURN toUpper(NULL)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(result.type == T_NULL);
	AR_EXP_Free(arExp);
}

void test_ToString(void) {
	setup();
	SIValue result;
	const char *expected;
	const char *query;
	AR_ExpNode *arExp;

	/* toString("muchacho") */
	query = "RETURN toString('muchacho')";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "muchacho";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* toString("3.14") */
	query = "RETURN toString(3.14)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	expected = "3.140000";
	TEST_CHECK(!strcmp(result.stringval, expected));
	AR_EXP_Free(arExp);

	/* toString() */
	query = "RETURN toString(NULL)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(result.type == T_NULL);
	AR_EXP_Free(arExp);
}

void test_Exists(void) {
	setup();
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
	TEST_CHECK(result.type == T_BOOL);
	TEST_CHECK(result.longval == 0);
	AR_EXP_Free(arExp);

	/* Pass 1, in case n.v exists and evaluates to 1. */
	query = "RETURN EXISTS(1)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(result.type == T_BOOL);
	TEST_CHECK(result.longval == 1);
	AR_EXP_Free(arExp);
}

void test_Timestamp(void) {
	setup();
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);

	query = "RETURN timestamp()";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	TEST_CHECK(abs((1000 * ts.tv_sec + ts.tv_nsec / 1000000) - result.longval) < 5);
}

void test_Case(void) {
	setup();
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
	TEST_CHECK(result.longval == expected.longval);

	/* Do not match any of the alternatives, return default. */
	query = "RETURN CASE 'green' WHEN 'blue' THEN 1+0 WHEN 'brown' THEN 2-0 ELSE 3*1 END";
	expected = SI_LongVal(3);
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	AR_EXP_Free(arExp);
	TEST_CHECK(result.longval == expected.longval);

	/* Test "Generic form"
	 * One of the alternatives evaluates to a none null value.
	 * Default not specified. */
	query = "RETURN CASE WHEN NULL THEN 1+0 WHEN true THEN 2-0 END";
	expected = SI_LongVal(2);
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	AR_EXP_Free(arExp);
	TEST_CHECK(result.longval == expected.longval);

	/* None of the alternatives evaluates to a none null value.
	 * Default specified, expecting default. */
	query = "RETURN CASE WHEN NULL THEN 1+0 WHEN NULL THEN 2-0 ELSE 3*1 END";
	expected = SI_LongVal(3);
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	AR_EXP_Free(arExp);
	TEST_CHECK(result.longval == expected.longval);

	/* None of the alternatives evaluates to a none null value.
	 * Default not specified, expecting NULL */
	query = "RETURN CASE WHEN NULL THEN 1+0 WHEN NULL THEN 2-0 END";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	AR_EXP_Free(arExp);
	TEST_CHECK(SIValue_IsNull(result));
}

void test_AND(void) {
	setup();
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

		TEST_CHECK(SI_TYPE(result) == SI_TYPE(expected));
		if(SI_TYPE(result) != T_NULL) {
			TEST_CHECK(result.longval == expected.longval);
		}
	}
}

void test_OR(void) {
	setup();
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

		TEST_CHECK(SI_TYPE(result) == SI_TYPE(expected));
		if(SI_TYPE(result) != T_NULL) {
			TEST_CHECK(result.longval == expected.longval);
		}
	}
}

void test_XOR(void) {
	setup();
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

		TEST_CHECK(SI_TYPE(result) == SI_TYPE(expected));
		if(SI_TYPE(result) != T_NULL) {
			TEST_CHECK(result.longval == expected.longval);
		}
	}
}

void test_NOT(void) {
	setup();
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

		TEST_CHECK(SI_TYPE(result) == SI_TYPE(expected));
		if(SI_TYPE(result) != T_NULL) {
			TEST_CHECK(result.longval == expected.longval);
		}
	}
}

void test_LT(void) {
	setup();
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

		TEST_CHECK(SI_TYPE(result) == SI_TYPE(expected));
		if(SI_TYPE(result) != T_NULL) {
			TEST_CHECK(result.longval == expected.longval);
		}
	}
}

void test_LE(void) {
	setup();
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

		TEST_CHECK(SI_TYPE(result) == SI_TYPE(expected));
		if(SI_TYPE(result) != T_NULL) {
			TEST_CHECK(result.longval == expected.longval);
		}
	}
}

void test_EQ(void) {
	setup();
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

		TEST_CHECK(SI_TYPE(result) == SI_TYPE(expected));
		if(SI_TYPE(result) != T_NULL) {
			TEST_CHECK(result.longval == expected.longval);
		}
	}
}

void test_NE(void) {
	setup();
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

		TEST_CHECK(SI_TYPE(result) == SI_TYPE(expected));
		if(SI_TYPE(result) != T_NULL) {
			TEST_CHECK(result.longval == expected.longval);
		}
	}
}

void test_List(void) {
	setup();
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;

	query = "RETURN [1,2.3,'4',True,False, null]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);
	TEST_CHECK(T_ARRAY == result.type);

	SIValue longVal = SIArray_Get(result, 0);
	SIValue doubleVal = SIArray_Get(result, 1);
	SIValue stringVal = SIArray_Get(result, 2);
	SIValue trueVal = SIArray_Get(result, 3);
	SIValue falseVal = SIArray_Get(result, 4);
	SIValue nullVal = SIArray_Get(result, 5);

	TEST_CHECK(T_INT64 == longVal.type);
	TEST_CHECK(1 == longVal.longval);

	TEST_CHECK(T_DOUBLE == doubleVal.type);
	TEST_CHECK(2.3 == doubleVal.doubleval);

	TEST_CHECK(T_STRING == stringVal.type);
	TEST_CHECK(!strcmp("4", stringVal.stringval));

	TEST_CHECK(T_BOOL == trueVal.type);
	TEST_CHECK(true == trueVal.longval);

	TEST_CHECK(T_BOOL == falseVal.type);
	TEST_CHECK(false == falseVal.longval);

	TEST_CHECK(SIValue_IsNull(nullVal));
}

void test_ListSlice(void) {
	setup();
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;

	query = "RETURN [0,1,2,3,4,5,6,7,8,9,10][3]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	TEST_CHECK(T_INT64 == result.type);
	TEST_CHECK(3 == result.longval);

	query = "RETURN [0,1,2,3,4,5,6,7,8,9,10][-3]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	TEST_CHECK(T_INT64 == result.type);
	TEST_CHECK(8 == result.longval);

	query = "RETURN [0,1,2,3,4,5,6,7,8,9,10][0..3]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	TEST_CHECK(T_ARRAY == result.type);
	TEST_CHECK(3 == SIArray_Length(result));

	for(int i = 0; i < 3; i++) {
		SIValue value = SIArray_Get(result, i);
		TEST_CHECK(T_INT64 == value.type);
		TEST_CHECK(i == value.longval);
	}

	query = "RETURN [0,1,2,3,4,5,6,7,8,9,10][0..-5]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	TEST_CHECK(T_ARRAY == result.type);
	TEST_CHECK(6 == SIArray_Length(result));

	for(int i = 0; i < 6; i++) {
		SIValue value = SIArray_Get(result, i);
		TEST_CHECK(T_INT64 == value.type);
		TEST_CHECK(i == value.longval);
	}

	query = "RETURN [0,1,2,3,4,5,6,7,8,9,10][-5..]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	TEST_CHECK(T_ARRAY == result.type);
	TEST_CHECK(5 == SIArray_Length(result));

	for(int i = 0; i < 5; i++) {
		SIValue value = SIArray_Get(result, i);
		TEST_CHECK(T_INT64 == value.type);
		TEST_CHECK(i + 6 == value.longval);
	}

	query = "RETURN [0,1,2,3,4,5,6,7,8,9,10][..4]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	TEST_CHECK(T_ARRAY == result.type);
	TEST_CHECK(4 == SIArray_Length(result));

	for(int i = 0; i < 4; i++) {
		SIValue value = SIArray_Get(result, i);
		TEST_CHECK(T_INT64 == value.type);
		TEST_CHECK(i == value.longval);
	}
}

void test_Range(void) {
	setup();
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;

	// create range from 0 to 10 [0,1,2,3,4,5,6,7,8,9,10]
	query = "RETURN range(0,10)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	TEST_CHECK(T_ARRAY == result.type);
	TEST_CHECK(11 == SIArray_Length(result));

	for(int i = 0; i < 11; i++) {
		SIValue value = SIArray_Get(result, i);
		TEST_CHECK(T_INT64 == value.type);
		TEST_CHECK(i == value.longval);
	}

	// creae ragne with skips of 3, statring from 2 to 18 [2,5,8,11,14,17]
	query = "RETURN range(2,18,3)";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	TEST_CHECK(T_ARRAY == result.type);
	TEST_CHECK(6 == SIArray_Length(result));

	for(int i = 0; i < 6; i++) {
		SIValue value = SIArray_Get(result, i);
		TEST_CHECK(T_INT64 == value.type);
		TEST_CHECK(i * 3 + 2 == value.longval);
	}
}

void test_In(void) {
	setup();
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;

	// check if 3 in [1,2,3]
	query = "RETURN 3 IN [1,2,3]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	TEST_CHECK(T_BOOL == result.type);
	TEST_CHECK(true == result.longval);

	// check if 4 in [1,2,3]
	query = "RETURN 4 IN [1,2,3]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	TEST_CHECK(T_BOOL == result.type);
	TEST_CHECK(false == result.longval);

	// check if [1,2] in [1,2,3]
	query = "RETURN [1,2] IN [1,2,3]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	TEST_CHECK(T_BOOL == result.type);
	TEST_CHECK(false == result.longval);

	// check if [1,2] in [[1,2],3]
	query = "RETURN [1,2] IN [[1,2],3]";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	TEST_CHECK(T_BOOL == result.type);
	TEST_CHECK(true == result.longval);
}

void test_IsNull(void) {
	setup();
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;

	// Check if null is null.
	query = "RETURN null IS NULL";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	TEST_CHECK(T_BOOL == result.type);
	TEST_CHECK(true == result.longval);

	// Check if null is not "not null".
	query = "RETURN null IS NOT NULL";
	arExp = _exp_from_query(query);
	result = AR_EXP_Evaluate(arExp, NULL);

	TEST_CHECK(T_BOOL == result.type);
	TEST_CHECK(false == result.longval);

	// Check for different types values.
	char *values[6] = {"1", "1.2", "true", "false", "'string'", "[1,2,3]"};
	for(int i = 0; i < 6; i++) {
		char buff[128];
		// Check if value is not null.
		sprintf(buff, "RETURN %s IS NOT NULL", values[i]);
		arExp = _exp_from_query(buff);
		result = AR_EXP_Evaluate(arExp, NULL);

		TEST_CHECK(T_BOOL == result.type);
		TEST_CHECK(true == result.longval);

		sprintf(buff, "RETURN %s IS NULL", values[i]);
		arExp = _exp_from_query(buff);
		result = AR_EXP_Evaluate(arExp, NULL);

		TEST_CHECK(T_BOOL == result.type);
		TEST_CHECK(false == result.longval);
	}
}

void test_Reduce(void) {
	setup();
	const char *query;
	AR_ExpNode *arExp;

	// Validate reduce to a constant value in applicable case.
	query = "RETURN 1 IN [1,2,3]";
	arExp = _exp_from_query(query);
	TEST_CHECK(AR_EXP_OPERAND == arExp->type);
	TEST_CHECK(AR_EXP_CONSTANT == arExp->operand.type);
	TEST_CHECK(0 == SIValue_Compare(SI_BoolVal(true), arExp->operand.constant, NULL));

	// Validate partial reduce of the AR_Exp tree. Reduce only applicable nodes.
	query = "RETURN rand() IN [1,2,3]";
	arExp = _exp_from_query(query);

	TEST_CHECK(AR_EXP_OP == arExp->type);
	TEST_CHECK(2 == arExp->op.child_count);

	AR_ExpNode *lhs = arExp->op.children[0];
	TEST_CHECK(AR_EXP_OP == lhs->type);
	TEST_CHECK(!strcasecmp("rand", lhs->op.func_name));

	AR_ExpNode *rhs = arExp->op.children[1];
	TEST_CHECK(AR_EXP_OPERAND == rhs->type);
	TEST_CHECK(AR_EXP_CONSTANT == rhs->operand.type);
	TEST_CHECK(T_ARRAY == rhs->operand.constant.type);
}

void test_Coalesce(void) {
	setup();
	const char *query;
	AR_ExpNode *arExp;

	// Test reduction of coalesce over static values.
	query = "RETURN coalesce(1)";
	arExp = _exp_from_query(query);
	TEST_CHECK(AR_EXP_OPERAND == arExp->type);
	TEST_CHECK(AR_EXP_CONSTANT == arExp->operand.type);
	TEST_CHECK(0 == SIValue_Compare(SI_LongVal(1), arExp->operand.constant, NULL));

	// Test reduction of coalesce over static values.
	query = "RETURN coalesce(null, 1)";
	arExp = _exp_from_query(query);
	TEST_CHECK(AR_EXP_OPERAND == arExp->type);
	TEST_CHECK(AR_EXP_CONSTANT == arExp->operand.type);
	TEST_CHECK(0 == SIValue_Compare(SI_LongVal(1), arExp->operand.constant, NULL));
}

TEST_LIST = {
	{ "test_AND", test_AND },
	{ "test_Abs", test_Abs },
	{ "test_Aggregate", test_Aggregate },
	{ "test_Case", test_Case },
	{ "test_Ceil", test_Ceil },
	{ "test_Coalesce", test_Coalesce },
	{ "test_EQ", test_EQ },
	{ "test_Exists", test_Exists },
	{ "test_Expression", test_Expression },
	{ "test_Floor", test_Floor },
	{ "test_In", test_In },
	{ "test_IsNull", test_IsNull },
	{ "test_LE", test_LE },
	{ "test_LT", test_LT },
	{ "test_LTrim", test_LTrim },
	{ "test_Left", test_Left },
	{ "test_List", test_List },
	{ "test_ListSlice", test_ListSlice },
	{ "test_NE", test_NE },
	{ "test_NOT", test_NOT },
	{ "test_NullArithmetic", test_NullArithmetic },
	{ "test_OR", test_OR },
	{ "test_Pow", test_Pow },
	{ "test_RTrim", test_RTrim },
	{ "test_RandomUUID", test_RandomUUID },
	{ "test_Range", test_Range },
	{ "test_Reduce", test_Reduce },
	{ "test_Reverse", test_Reverse },
	{ "test_Right", test_Right },
	{ "test_Round", test_Round },
	{ "test_Sign", test_Sign },
	{ "test_Sqrt", test_Sqrt },
	{ "test_Substring", test_Substring },
	{ "test_Timestamp", test_Timestamp },
	{ "test_ToInteger", test_ToInteger },
	{ "test_ToLower", test_ToLower },
	{ "test_ToString", test_ToString },
	{ "test_ToUpper", test_ToUpper },
	{ "test_Trim", test_Trim },
	{ "test_XOR", test_XOR },
	{ NULL, NULL }     // zeroed record marking the end of the list
};

