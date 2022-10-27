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

// Count valid entities
TEST_F(AggregateTest, CountTest) {
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;

	/* count(1) */
	query = "RETURN count(1)";
	arExp = _exp_from_query(query);

	int num_values = 5;
	for(int i = 0; i < num_values; i++) AR_EXP_Aggregate(arExp, NULL);
	result = AR_EXP_FinalizeAggregations(arExp, NULL);
	ASSERT_EQ(result.longval, num_values);
	AR_EXP_Free(arExp);
}

// Count a mix of valid and invalid entities
TEST_F(AggregateTest, PartialCountTest) {
	const char *query;
	AR_ExpNode *arExp;
	AR_ExpNode *arExpOne;
	AR_ExpNode *arExpNULL;

	query = "RETURN 1";
	arExpOne = _exp_from_query(query);

	query = "RETURN NULL";
	arExpNULL = _exp_from_query(query);

	/* count(1) */
	query = "RETURN count(1)";
	arExp = _exp_from_query(query);

	int num_values = 10;
	for(int i = 0; i < num_values; i++) {
		// Every odd entity will be a valid numeric,
		// and every even entity will be a null value
		if(i % 2)
			arExp->op.children[0] = arExpOne;
		else
			arExp->op.children[0] = arExpNULL;
		AR_EXP_Aggregate(arExp, NULL);
	}
	SIValue res = AR_EXP_FinalizeAggregations(arExp, NULL);

	// The counted result should be half the number of inserted entities,
	// as the null values are ignored.
	ASSERT_EQ(res.longval, num_values / 2);
	AR_EXP_Free(arExp);
}

TEST_F(AggregateTest, PercentileContTest) {
	// Percentiles to check
	AR_ExpNode *zero = AR_EXP_NewConstOperandNode(SI_DoubleVal(0));
	AR_ExpNode *dot_one = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.1));
	AR_ExpNode *one_third = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.33));
	AR_ExpNode *half = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.5));
	AR_ExpNode *one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));

	AR_ExpNode *test_percentiles[5] = {zero, dot_one, one_third, half, one};
	/*
	 * The correct value for each percentile will be:
	 * x = percentile * (count - 1)
	 * if x is an integer return value[x]
	 * else:
	 * (value[floor(x)] * fraction(x)) + (value[ceil(x)] * (1 - fraction(x)))
	 */
	double expected_values[5];
	double arr[5] = {2, 4, 6, 8, 10};
	int count = 5;

	double percentile_doubles[5] = {0, 0.1, 0.33, 0.5, 1};
	double x;
	int lower_idx, upper_idx;
	double lower, upper;

	for(int i = 0; i < 5; i++) {
		x = percentile_doubles[i] * (count - 1);
		lower_idx = floor(x);
		upper_idx = ceil(x);

		if(lower_idx == upper_idx || lower_idx == (count - 1)) {
			expected_values[i] = arr[lower_idx];
			continue;
		}

		lower = arr[lower_idx];
		upper = arr[upper_idx];

		expected_values[i] = (lower * ((double)upper_idx - x)) + (upper * (x - (double)lower_idx));
	}

	AR_ExpNode *perc;
	SIValue result;
	for(int i = 0; i < 5; i++) {
		perc = _exp_from_query("RETURN percentileCont(1, 1)");
		for(int j = 1; j <= 5; j++) {
			perc->op.children[0] = AR_EXP_NewConstOperandNode(SI_DoubleVal(j * 2));
			// The last child of this node will be the requested percentile
			perc->op.children[1] = test_percentiles[i];
			AR_EXP_Aggregate(perc, NULL);
		}
		result = AR_EXP_FinalizeAggregations(perc, NULL);
		ASSERT_EQ(result.doubleval, expected_values[i]);
		AR_EXP_Free(perc);
	}
}

TEST_F(AggregateTest, PercentileDiscTest) {
	// Percentiles to check
	AR_ExpNode *zero = AR_EXP_NewConstOperandNode(SI_DoubleVal(0));
	AR_ExpNode *dot_one = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.1));
	AR_ExpNode *one_third = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.33));
	AR_ExpNode *half = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.5));
	AR_ExpNode *one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));

	AR_ExpNode *test_percentiles[5] = {zero, dot_one, one_third, half, one};
	// percentileDisc should always return a value actually contained in the set
	int expected[5] = {0, 0, 1, 2, 4};
	AR_ExpNode *expectedResults[5];
	for(int i = 1; i <= 5; i ++) {
		expectedResults[i - 1] = AR_EXP_NewConstOperandNode(SI_DoubleVal(i * 2));
	}
	SIValue expected_outcome;
	AR_ExpNode *perc;
	for(int i = 0; i < 5; i++) {
		perc = _exp_from_query("RETURN percentileDisc(1, 1)");
		for(int j = 1; j <= 5; j++) {
			perc->op.children[0] = AR_EXP_NewConstOperandNode(SI_DoubleVal(j * 2));
			// The last child of this node will be the requested percentile
			perc->op.children[1] = test_percentiles[i];
			AR_EXP_Aggregate(perc, NULL);
		}
		// Reduce sorts the list and applies the percentile formula
		SIValue res = AR_EXP_FinalizeAggregations(perc, NULL);
		expected_outcome = AR_EXP_Evaluate(expectedResults[expected[i]], NULL);
		ASSERT_EQ(res.doubleval, expected_outcome.doubleval);
		AR_EXP_Free(perc);
	}
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

TEST_F(AggregateTest, AverageLongOverflowTest) {
	// values to average
	AR_ExpNode *max = AR_EXP_NewConstOperandNode(SI_LongVal(LONG_MAX));
	AR_ExpNode *half_max = AR_EXP_NewConstOperandNode(SI_LongVal(LONG_MAX / 2));

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
	SIValue expected_outcome = SI_DoubleVal((LONG_MAX / 2) + ((LONG_MAX / 2) / 2));
	ASSERT_EQ(res.doubleval, expected_outcome.doubleval);

	AR_EXP_Free(avg);
	AR_EXP_Free(max);
}

