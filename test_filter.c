#include <stdio.h>
#include <string.h>
#include "filter.h"
#include "assert.h"

int main(int argc, char **argv) {
	// Equal filter
	Filter *equalFilter = EqualFilter("age", 32);
	assert(ValidateFilter(equalFilter) == 1);
	assert(strcmp(equalFilter->property, "age") == 0);
	assert(equalFilter->op == Eq);
	FreeFilter(equalFilter);

	// Greater than filter
	Filter *greaterThanFilter = GreaterThanFilter("rooms", 4);
	assert(ValidateFilter(greaterThanFilter) == 1);
	assert(strcmp(greaterThanFilter->property, "rooms") == 0);
	assert(greaterThanFilter->op == Gt);
	FreeFilter(greaterThanFilter);

	// Greater than filter
	Filter *lessThanFilter = LessThanFilter("kids", 3);
	assert(ValidateFilter(lessThanFilter) == 1);
	assert(strcmp(lessThanFilter->property, "kids") == 0);
	assert(lessThanFilter->op == Lt);
	FreeFilter(lessThanFilter);

	// Range filter
	Filter *rangeFilter = RangeFilter("beers", 2, 4);
	assert(ValidateFilter(rangeFilter) == 1);
	assert(strcmp(rangeFilter->property, "beers") == 0);
	assert(rangeFilter->op == Between);
	FreeFilter(rangeFilter);

	printf("PASS!");
    return 0;
}