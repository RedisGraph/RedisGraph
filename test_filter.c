#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "filter.h"
#include "assert.h"

int main(int argc, char **argv) {
	// Equal filter
	Filter *equalFilter = EqualIntegerFilter(32);
	assert(ValidateFilter(equalFilter) == 1);
	assert(equalFilter->op == EQ);
	assert(equalFilter->eq.v.n == 32);
	assert(equalFilter->eq.v.t == Integer);
	FreeFilter(equalFilter);

	equalFilter = EqualFloatFilter(16.5);
	assert(ValidateFilter(equalFilter) == 1);
	assert(equalFilter->op == EQ);
	assert(equalFilter->eq.v.f == 16.5);
	assert(equalFilter->eq.v.t == Float);
	FreeFilter(equalFilter);

	equalFilter = EqualStringFilter("Tokyo");
	assert(ValidateFilter(equalFilter) == 1);
	assert(equalFilter->op == EQ);
	assert(strcmp(equalFilter->eq.v.c, "Tokyo") == 0);
	assert(equalFilter->eq.v.t == String);
	FreeFilter(equalFilter);

	// Greater than filter
	Filter *greaterThanFilter = GreaterThanIntergerFilter(4, 1);
	assert(ValidateFilter(greaterThanFilter) == 1);
	assert(greaterThanFilter->op == GE);
	assert(greaterThanFilter->rng.minExclusive == 1);
	assert(greaterThanFilter->rng.min.n == 4);
	assert(greaterThanFilter->rng.min.t == Integer);
	assert(greaterThanFilter->rng.max.n == INT_MAX);
	assert(greaterThanFilter->rng.max.t == Integer);
	FreeFilter(greaterThanFilter);

	greaterThanFilter = GreaterThanIntergerFilter(4, 0);
	assert(ValidateFilter(greaterThanFilter) == 1);
	assert(greaterThanFilter->op == GT);
	assert(greaterThanFilter->rng.minExclusive == 0);
	assert(greaterThanFilter->rng.min.n == 4);
	assert(greaterThanFilter->rng.min.t == Integer);
	assert(greaterThanFilter->rng.max.n == INT_MAX);
	assert(greaterThanFilter->rng.max.t == Integer);
	FreeFilter(greaterThanFilter);

	// Less than filter
	Filter *lessThanFilter = LessThanIntergerFilter(9, 1);
	assert(ValidateFilter(lessThanFilter) == 1);
	assert(lessThanFilter->op == LE);
	assert(lessThanFilter->rng.maxExclusive == 1);
	assert(lessThanFilter->rng.max.n == 9);
	assert(lessThanFilter->rng.max.t == Integer);
	assert(lessThanFilter->rng.min.n == INT_MIN);
	assert(lessThanFilter->rng.min.t == Integer);
	FreeFilter(lessThanFilter);

	lessThanFilter = LessThanIntergerFilter(9, 0);
	assert(ValidateFilter(lessThanFilter) == 1);
	assert(lessThanFilter->op == LT);
	assert(lessThanFilter->rng.maxExclusive == 0);
	assert(lessThanFilter->rng.max.n == 9);
	assert(lessThanFilter->rng.max.t == Integer);
	assert(lessThanFilter->rng.min.n == INT_MIN);
	assert(lessThanFilter->rng.min.t == Integer);
	FreeFilter(lessThanFilter);

	printf("PASS!");
    return 0;
}