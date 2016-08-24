#ifndef FILTER_H
#define FILTER_H

#include "rmutil/vector.h"
// Filter represents a single constraint an object
// needs to pass inorder to be part of a result set.


// Operator, different kinds of filter operations
// currently we're only supporting numeric operations.
typedef enum {Eq, Gt, Lt, Between} Operator;

typedef struct  {
	char* property;	// Filter will be applied to this property.
	Operator op;			// Operation to apply.
	Vector* values;			// List of values to compare against.
} Filter;

// NewFilter creates a new filter for a property with a given operator and comparison value.
Filter* NewFilter(const char* property, Operator op, int n_args, int, ...);

Filter* EqualFilter(const char* property, int value);
Filter* GreaterThanFilter(const char* property, int value);
Filter* LessThanFilter(const char* property, int value);
Filter* RangeFilter(const char* property, int min, int max);

// Validate checks the filter for validity and returns false if something is wrong with the filter
int ValidateFilter(const Filter* filter);

// Frees alocated space by given filter.
void FreeFilter(Filter* filter);

#endif