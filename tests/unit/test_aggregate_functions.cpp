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

#include "../../src/query_ctx.h"
#include "../../src/util/rmalloc.h"
#include "../../src/arithmetic/funcs.h"
#include "../../src/execution_plan/execution_plan.h"
#include "../../src/arithmetic/arithmetic_expression.h"
#include "../../src/util/arr.h"
#include "../../src/errors.h"

// Declaration of used functions not in header files
extern AR_ExpNode **_BuildProjectionExpressions(const cypher_astnode_t *ret_clause, AST *ast);


#ifdef __cplusplus
}
#endif

class AggregateTest: public ::testing::Test {
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
};

AR_ExpNode *_exp_from_query(const char *query) {
	cypher_parse_result_t *parse_result = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
	AST *ast = AST_Build(parse_result);

	const cypher_astnode_t *ret_clause = AST_GetClause(ast, CYPHER_AST_RETURN,
			NULL);
	return _BuildProjectionExpressions(ret_clause, ast)[0];
}

// Tests both stDev and stDevP
TEST_F(AggregateTest, StDevTest) {
	// Edge case - operation called on < 2 values
	AR_ExpNode *stdev = _exp_from_query("RETURN stDev(1)");
	stdev->op.children[0] = AR_EXP_NewConstOperandNode(SI_DoubleVal(5.1));
	AR_EXP_Aggregate(stdev, NULL);
	SIValue result = AR_EXP_FinalizeAggregations(stdev, NULL);
	ASSERT_EQ(result.doubleval, 0);
	AR_EXP_Free(stdev);

	// Stdev of squares of first 10 positive integers
	stdev = _exp_from_query("RETURN stDev(1)");
	double sum = 0;
	for(int i = 1; i <= 10; i++) {
		stdev->op.children[0] = AR_EXP_NewConstOperandNode(SI_DoubleVal(i));
		AR_EXP_Aggregate(stdev, NULL);
		sum += i;
		// Avoid double free on the last value.
		if(i < 10) AR_EXP_Free(stdev->op.children[0]);
	}
	double mean = sum / 10;
	double tmp_variance = 0;
	for(int i = 1; i <= 10; i++) {
		tmp_variance += pow(i - mean, 2);
	}
	double sample_variance = tmp_variance / 9;
	double sample_result = sqrt(sample_variance);

	result = AR_EXP_FinalizeAggregations(stdev, NULL);

	ASSERT_EQ(result.doubleval, sample_result);
	AR_EXP_Free(stdev);

	// Perform last test with stDevP
	AR_ExpNode *stdevp = _exp_from_query("RETURN stDevP(1)");
	for(int i = 1; i <= 10; i++) {
		stdevp->op.children[0] = AR_EXP_NewConstOperandNode(SI_DoubleVal(i));
		AR_EXP_Aggregate(stdevp, NULL);
		// Avoide double free on the last value.
		if(i < 10) AR_EXP_Free(stdevp->op.children[0]);
	}

	double pop_variance = tmp_variance / 10;
	double pop_result = sqrt(pop_variance);

	result = AR_EXP_FinalizeAggregations(stdevp, NULL);

	ASSERT_EQ(result.doubleval, pop_result);
	AR_EXP_Free(stdevp);
}

TEST_F(AggregateTest, AverageDoubleOverflowTest) {
	// values to average
	AR_ExpNode *max = AR_EXP_NewConstOperandNode(SI_DoubleVal(DBL_MAX));
	AR_ExpNode *half_max = AR_EXP_NewConstOperandNode(SI_DoubleVal(DBL_MAX / 2));

	// build new average function
	AR_ExpNode *avg = _exp_from_query("RETURN avg(1)");

	// first value to average is DBL_MAX
	avg->op.children[0] = max;
	AR_EXP_Aggregate(avg, NULL);

	// second value is DBL_MAX / 2, causing overflow
	avg->op.children[0] = half_max;
	AR_EXP_Aggregate(avg, NULL);

	// test average of DBL_MAX and DBL_MAX / 2
	SIValue res = AR_EXP_FinalizeAggregations(avg, NULL);
	SIValue expected_outcome = SI_DoubleVal((DBL_MAX / 2) + ((DBL_MAX / 2) / 2));
	ASSERT_EQ(res.doubleval, expected_outcome.doubleval);

	AR_EXP_Free(avg);
	AR_EXP_Free(max);
}

// TEST_F(AggregateTest, AverageLongOverflowTest) {
// 	// values to average
// 	AR_ExpNode *max = AR_EXP_NewConstOperandNode(SI_LongVal(LONG_MAX));
// 	AR_ExpNode *half_max = AR_EXP_NewConstOperandNode(SI_LongVal(LONG_MAX / 2));

// 	// build new average function
// 	AR_ExpNode *avg = _exp_from_query("RETURN avg(1)");

// 	// first value to average is DBL_MAX
// 	avg->op.children[0] = max;
// 	AR_EXP_Aggregate(avg, NULL);

// 	// second value is DBL_MAX / 2, causing overflow
// 	avg->op.children[0] = half_max;
// 	AR_EXP_Aggregate(avg, NULL);

// 	// test average of DBL_MAX and DBL_MAX / 2
// 	SIValue res = AR_EXP_FinalizeAggregations(avg, NULL);
// 	SIValue expected_outcome = SI_DoubleVal((LONG_MAX / 2) + ((LONG_MAX / 2) / 2));
// 	ASSERT_EQ(res.doubleval, expected_outcome.doubleval);

// 	AR_EXP_Free(avg);
// 	AR_EXP_Free(max);
// }

