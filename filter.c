#include "filter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

Filter* NewFilter(Operator op) {
	Filter *filter = (Filter *)malloc(sizeof(Filter));
	filter->op = op;
	return filter;
}

Filter* EqualIntegerFilter(int value) {
	Filter* f = NewFilter(EQ);

	f->eq.v.n = value;
	f->eq.v.t = Integer;

	return f;
}

Filter* EqualFloatFilter(float value) {
	Filter* f = NewFilter(EQ);

	f->eq.v.f = value;
	f->eq.v.t = Float;

	return f;
}

Filter* EqualStringFilter(const char* value) {
	Filter* f = NewFilter(EQ);

	f->eq.v.c = (char*)malloc(strlen(value)+1);
	strcpy(f->eq.v.c, value);
	f->eq.v.t = String;

	return f;
}

Filter* GreaterThanIntergerFilter(int value, int exclusive) {
	Operator op;

	if(exclusive) {
		op = GE;
	} else {
		op = GT;
	}

	Filter* f = NewFilter(op);
	f->rng.min.n = value;
	f->rng.min.t = Integer;
	f->rng.minExclusive = exclusive;
	f->rng.max.n = INT_MAX;
	f->rng.max.t = Integer;

	return f;
}

Filter* GreaterThanFloatFilter(float value, int exclusive) {
	Operator op;

	if(exclusive) {
		op = GE;
	} else {
		op = GT;
	}

	Filter* f = NewFilter(op);
	f->rng.min.f = value;
	f->rng.min.t = Float;
	f->rng.minExclusive = exclusive;
	f->rng.max.f = LONG_MAX;
	f->rng.max.t = Float;

	return f;
}

Filter* LessThanIntergerFilter(int value, int exclusive) {
	Operator op;

	if(exclusive) {
		op = LE;
	} else {
		op = LT;
	}

	Filter* f = NewFilter(op);
	f->rng.max.n = value;
	f->rng.max.t = Integer;
	f->rng.maxExclusive = exclusive;
	f->rng.min.n = INT_MIN;
	f->rng.min.t = Integer;

	return f;
}

Filter* LessThanFloatFilter(int value, int exclusive) {
	Operator op;

	if(exclusive) {
		op = LE;
	} else {
		op = LT;
	}

	Filter* f = NewFilter(op);
	f->rng.max.n = value;
	f->rng.max.t = Integer;
	f->rng.maxExclusive = exclusive;
	f->rng.min.f = LONG_MIN;
	f->rng.min.t = Integer;

	return f;
}

int ValidateFilter(const Filter* filter) {
	// Check for unknow operator
	switch(filter->op) {
		case EQ:
			if(filter->eq.v.t == String && filter->eq.v.c == 0) {
				fprintf(stderr, "missing value to compare against\n");
				return 0;
			}
			break;

		case GT:
		case GE:
		case LT:
		case LE:
			if(filter->rng.min.t != filter->rng.max.t) {
				fprintf(stderr, "range min and max value types should be the same\n");
				return 0;
			}

			if(filter->rng.min.t == Integer) {
				if(filter->rng.min.n >= filter->rng.max.n) {
					fprintf(stderr, "range min value %d can not be greater or equals to range max value %d\n", filter->rng.min.n, filter->rng.max.n);
					return 0;
				}
			}

			if(filter->rng.min.t == Float) {
				if(filter->rng.min.f >= filter->rng.max.f) {
					fprintf(stderr, "range min value %f can not be greater or equals to range max value %f\n", filter->rng.min.f, filter->rng.max.f);
					return 0;
				}
			}
			break;

		default:
			fprintf(stderr, "Unsupported operator: %d\n", filter->op);
			return 0;
	}

	return 1;
}

void FreeFilter(Filter* filter) {
	if(filter->op == EQ) {
		if(filter->eq.v.t == String) {
			free(filter->eq.v.c);
		}
	}

	free(filter);
}