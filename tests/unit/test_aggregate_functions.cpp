/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
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
#include "../../src/arithmetic/agg_funcs.h"
#include "../../src/execution_plan/record.h"
#include "../../src/execution_plan/execution_plan.h"
#include "../../src/arithmetic/arithmetic_expression.h"
#include "../../src/util/arr.h"

// Declaration of used functions not in header files
extern void _BuildReturnExpressions(ExecutionPlanSegment *segment,
									const cypher_astnode_t *ret_clause, AST *ast);
extern AR_ExpNode *AR_EXP_NewOpNode(const char *func_name, uint child_count);


#ifdef __cplusplus
}
#endif

class AggregateTest: public ::testing::Test {
  protected:
	Record r = NULL;
	static void SetUpTestCase() {
		// Use the malloc family for allocations
		Alloc_Reset();

		// Prepare thread-local variables
		QueryCtx_Init();

		// Register functions
		AR_RegisterFuncs();
		Agg_RegisterFuncs();
	}
};

AR_ExpNode *_exp_from_query(const char *query) {
	cypher_parse_result_t *parse_result = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
	AST *ast = AST_Build(parse_result);

	const cypher_astnode_t *ret_clause = AST_GetClause(ast, CYPHER_AST_RETURN);
	ExecutionPlanSegment *segment = (ExecutionPlanSegment *)rm_malloc(sizeof(ExecutionPlanSegment));
	_BuildReturnExpressions(segment, ret_clause, NULL);

	return segment->projections[0];
}

// Count valid entities
TEST_F(AggregateTest, CountTest) {
	SIValue result;
	const char *query;
	AR_ExpNode *arExp;
	Record r = Record_New(0);

	/* count(1) */
	query = "RETURN count(1)";
	arExp = _exp_from_query(query);

	int num_values = 5;
	for(int i = 0; i < num_values; i++)
		AR_EXP_Aggregate(arExp, r);
	AR_EXP_Reduce(arExp);
	result = AR_EXP_Evaluate(arExp, r);
	ASSERT_EQ(result.longval, num_values);
	AR_EXP_Free(arExp);
}

// Count a mix of valid and invalid entities
TEST_F(AggregateTest, PartialCountTest) {
	const char *query;
	AR_ExpNode *arExp;
	AR_ExpNode *arExpOne;
	AR_ExpNode *arExpNULL;
	Record r = Record_New(0);

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
		AR_EXP_Aggregate(arExp, r);
	}
	AR_EXP_Reduce(arExp);
	SIValue res = AR_EXP_Evaluate(arExp, r);

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
		perc = AR_EXP_NewOpNode("percentileCont", 6);
		for(int j = 1; j <= 5; j++) {
			perc->op.children[j - 1] = AR_EXP_NewConstOperandNode(SI_DoubleVal(j * 2));
		}
		// The last child of this node will be the requested percentile
		perc->op.children[5] = test_percentiles[i];
		AR_EXP_Aggregate(perc, r);
		// Reduce sorts the list and applies the percentile formula
		AR_EXP_Reduce(perc);
		result = AR_EXP_Evaluate(perc, r);
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

	SIValue expected_outcome;
	AR_ExpNode *perc;
	for(int i = 0; i < 5; i++) {
		perc = AR_EXP_NewOpNode("percentileDisc", 6);
		for(int j = 1; j <= 5; j++) {
			perc->op.children[j - 1] = AR_EXP_NewConstOperandNode(SI_DoubleVal(j * 2));
		}
		// The last child of this node will be the requested percentile
		perc->op.children[5] = test_percentiles[i];
		AR_EXP_Aggregate(perc, r);
		// Reduce sorts the list and applies the percentile formula
		AR_EXP_Reduce(perc);
		SIValue res = AR_EXP_Evaluate(perc, r);
		expected_outcome = AR_EXP_Evaluate(perc->op.children[expected[i]], r);
		ASSERT_EQ(res.doubleval, expected_outcome.doubleval);
		AR_EXP_Free(perc);
	}
}

// Tests both stDev and stDevP
TEST_F(AggregateTest, StDevTest) {
	// Edge case - operation called on < 2 values
	AR_ExpNode *stdev = AR_EXP_NewOpNode("stDev", 1);
	stdev->op.children[0] = AR_EXP_NewConstOperandNode(SI_DoubleVal(5.1));
	AR_EXP_Aggregate(stdev, r);
	AR_EXP_Reduce(stdev);
	SIValue result = AR_EXP_Evaluate(stdev, r);
	ASSERT_EQ(result.doubleval, 0);
	AR_EXP_Free(stdev);

	// Stdev of squares of first 10 positive integers
	stdev = AR_EXP_NewOpNode("stDev", 10);
	double sum = 0;
	for(int i = 1; i <= 10; i++) {
		stdev->op.children[i - 1] = AR_EXP_NewConstOperandNode(SI_DoubleVal(i));
		sum += i;
	}
	double mean = sum / 10;
	double tmp_variance = 0;
	for(int i = 1; i <= 10; i++) {
		tmp_variance += pow(i - mean, 2);
	}
	double sample_variance = tmp_variance / 9;
	double sample_result = sqrt(sample_variance);

	AR_EXP_Aggregate(stdev, r);
	AR_EXP_Reduce(stdev);
	result = AR_EXP_Evaluate(stdev, r);

	ASSERT_EQ(result.doubleval, sample_result);
	AR_EXP_Free(stdev);

	// Perform last test with stDevP
	AR_ExpNode *stdevp = AR_EXP_NewOpNode("stDevP", 10);
	for(int i = 1; i <= 10; i++) {
		stdevp->op.children[i - 1] = AR_EXP_NewConstOperandNode(SI_DoubleVal(i));
	}

	double pop_variance = tmp_variance / 10;
	double pop_result = sqrt(pop_variance);

	AR_EXP_Aggregate(stdevp, r);
	AR_EXP_Reduce(stdevp);
	result = AR_EXP_Evaluate(stdevp, r);

	ASSERT_EQ(result.doubleval, pop_result);
	AR_EXP_Free(stdevp);
}

