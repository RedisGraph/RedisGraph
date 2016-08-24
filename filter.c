#include "filter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

Filter* NewFilter(const char* property, Operator op, int n_args, int values, ...) {
	Filter *filter = (Filter *)malloc(sizeof(Filter));
	filter->property = (char*)malloc(strlen(property) + 1);
	strcpy(filter->property, property);
	filter->op = op;
	filter->values = NewVector(int, 0);

	// read values
	va_list args;
	va_start(args, values);

	for(int i = 0; i < n_args; i++) {
		Vector_Push(filter->values, va_arg(args, int));
	}

	return filter;
}

Filter* EqualFilter(const char* property, int value) {
	return NewFilter(property, Eq, 1, value);
}

Filter* GreaterThanFilter(const char* property, int value) {
	return NewFilter(property, Gt, 1, value);
}

Filter* LessThanFilter(const char* property, int value) {
	return NewFilter(property, Lt, 1, value);
}

Filter* RangeFilter(const char* property, int min, int max) {
	return NewFilter(property, Between, 2, min, max);
}

int ValidateFilter(const Filter* filter) {	
	// Check that we have a property
	if(strlen(filter->property) == 0) {
		fprintf(stderr, "No property given for filter\n");
		return 0;
	}

	int valuesCount = Vector_Size(filter->values);
	// Check that we have values
	if(valuesCount == 0) {
		fprintf(stderr, "No values given for filter\n");
		return 0;
	}

	// Check for unknow operator
	switch(filter->op) {
		case Eq:
		case Gt:
		case Lt:
			if(valuesCount > 1) {
				fprintf(stderr, "Too many values for filter\n");
				return 0;
			}
			break;

		case Between:
			if(valuesCount != 2) {				
				fprintf(stderr, "Range filters must have exactly 2 values, %d given\n", valuesCount);
				return 0;
			}
			break;

		default:
			fprintf(stderr, "Unsupported operator: %d\n", filter->op);
			return 0;
	}

	return 1;
}

void FreeFilter(Filter* filter) {
	Vector_Free(filter->values);
	free(filter->property);
	free(filter);
}