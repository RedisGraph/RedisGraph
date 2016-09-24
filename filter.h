#ifndef FILTER_H
#define FILTER_H

// Filter represents a single constraint an object
// needs to pass inorder to be part of a result set.

// Operator, different kinds of filter operations
typedef enum {EQ, GT, GE, LT, LE, RANGE} Operator;

// Types filter operates on
typedef enum { Integer, Float, String } ValueType;

typedef struct {
	union {
		int n;
		char* c;
		float f;
	};
	ValueType t;
} SIValue;

/* Equals to predicate */
typedef struct {
  // the value we must be equal to
  SIValue v;
} SIEquals;

/* Range predicate. Can also express GT / LT / GE / LE */
typedef struct {
	SIValue min;
	int minExclusive;
	SIValue max;
	int maxExclusive;
} SIRange;

/* NOT predicate */
typedef struct {
	// the value we must be different from
	SIValue v;
} SINotEquals;

typedef struct  {
	union {
	    SIEquals eq;
	    SIRange rng;
	    SINotEquals ne;
	};
	Operator op;		// Operation to apply.
} Filter;

// NewFilter creates a new filter for a property with a given operator and comparison value.
Filter* NewFilter(Operator op);

Filter* EqualIntegerFilter(int value);
Filter* EqualFloatFilter(float value);
Filter* EqualStringFilter(const char* value);

Filter* GreaterThanIntergerFilter(int value, int Exclusive);
Filter* LessThanIntergerFilter(int value, int Exclusive);

// Filter* RangeFilter(int min, int max);

// Validate checks the filter for validity and returns false if something is wrong with the filter
int ValidateFilter(const Filter* filter);

// Frees alocated space by given filter.
void FreeFilter(Filter* filter);

#endif