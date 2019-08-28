/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "value.h"
#include "graph/entities/graph_entity.h"
#include "graph/entities/node.h"
#include "graph/entities/edge.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>
#include <assert.h>
#include "util/rmalloc.h"
#include "datatypes/array.h"

SIValue SI_LongVal(int64_t i) {
	return (SIValue) {
		.longval = i, .type = T_INT64
	};
}

SIValue SI_DoubleVal(double d) {
	return (SIValue) {
		.doubleval = d, .type = T_DOUBLE
	};
}

SIValue SI_NullVal(void) {
	return (SIValue) {
		.longval = 0, .type = T_NULL
	};
}

SIValue SI_BoolVal(int b) {
	return (SIValue) {
		.longval = b, .type = T_BOOL
	};
}

SIValue SI_PtrVal(void *v) {
	return (SIValue) {
		.ptrval = v, .type = T_PTR
	};
}

SIValue SI_Node(void *n) {
	return (SIValue) {
		.ptrval = n, .type = T_NODE, .allocation = M_VOLATILE
	};
}

SIValue SI_Edge(void *e) {
	return (SIValue) {
		.ptrval = e, .type = T_EDGE, .allocation = M_VOLATILE
	};
}
SIValue SI_Array(u_int64_t initialCapacity) {
	return SIArray_New(initialCapacity);
}

SIValue SI_EmptyArray() {
	return SIArray_New(0);
}

SIValue SI_DuplicateStringVal(const char *s) {
	return (SIValue) {
		.stringval = rm_strdup(s), .type = T_STRING, .allocation = M_SELF
	};
}

SIValue SI_ConstStringVal(char *s) {
	return (SIValue) {
		.stringval = s, .type = T_STRING, .allocation = M_CONST
	};
}

SIValue SI_TransferStringVal(char *s) {
	return (SIValue) {
		.stringval = s, .type = T_STRING, .allocation = M_SELF
	};
}

/* Make an SIValue that reuses the original's allocations, if any.
 * The returned value is not responsible for freeing any allocations,
 * and is not guaranteed that these allocations will remain in scope. */
SIValue SI_ShareValue(const SIValue v) {
	SIValue dup = v;
	// If the original value owns an allocation, mark that the duplicate shares it.
	if(v.allocation == M_SELF) dup.allocation = M_VOLATILE;
	return dup;
}

/* Make an SIValue that creates its own copies of the original's allocations, if any.
 * This is not a deep clone: if the inner value holds its own references,
 * such as the Entity pointer to the properties of a Node or Edge, those are unmodified. */
SIValue SI_CloneValue(const SIValue v) {
	if(v.allocation == M_NONE) return v; // Stack value; no allocation necessary.

	if(v.type == T_STRING) {
		// Allocate a new copy of the input's string value.
		return SI_DuplicateStringVal(v.stringval);
	}

	if(v.type == T_ARRAY) {
		return SIArray_Clone(v);
	}

	// Copy the memory region for Node and Edge values. This does not modify the
	// inner Entity pointer to the value's properties.
	SIValue clone;
	clone.type = v.type;
	clone.allocation = M_SELF;

	size_t size;
	if(v.type == T_NODE) {
		size = sizeof(Node);
	} else if(v.type == T_EDGE) {
		size = sizeof(Edge);
	} else {
		assert(false && "Encountered heap-allocated SIValue of unhandled type");
	}

	clone.ptrval = rm_malloc(size);
	memcpy(clone.ptrval, v.ptrval, size);
	return clone;
}

/* Make an SIValue that shares the original's allocations but can safely expect those allocations
 *  to remain in scope. This is most frequently the case for GraphEntity properties. */
SIValue SI_ConstValue(const SIValue v) {
	SIValue dup = v;
	if(v.allocation != M_NONE) dup.allocation = M_CONST;
	return dup;
}

/* Update an SIValue marked as owning its internal allocations so that it instead is sharing them,
 * with no responsibility for freeing or guarantee regarding scope.
 * This is used in cases like performing shallow copies of scalars in Record entries. */
void SIValue_MakeVolatile(SIValue *v) {
	if(v->allocation == M_SELF) v->allocation = M_VOLATILE;
}

/* Ensure that any allocation held by the given SIValue is guaranteed to not go out
 * of scope during the lifetime of this query by copying references to volatile memory.
 * Heap allocations that are not scoped to the input SIValue, such as strings from the AST
 * or a GraphEntity property, are not modified. */
void SIValue_Persist(SIValue *v) {
	// Do nothing for non-volatile values.
	if(v->allocation != M_VOLATILE) return;

	// For volatile values, persisting uses the same logic as cloning.
	*v = SI_CloneValue(*v);
}

inline bool SIValue_IsNull(SIValue v) {
	return v.type == T_NULL;
}

inline bool SIValue_IsNullPtr(SIValue *v) {
	return v == NULL || v->type == T_NULL;
}

const char *SIType_ToString(SIType t) {
	if(t & T_NULL) {
		return "Null";
	} else if(t & T_STRING) {
		return "String";
	} else if(t & T_INT64) {
		return "Integer";
	} else if(t & T_BOOL) {
		return "Boolean";
	} else if(t & T_DOUBLE) {
		return "Float";
	} else if(t & T_PTR) {
		return "Pointer";
	} else if(t & T_NODE) {
		return "Node";
	} else if(t & T_EDGE) {
		return "Edge";
	} else if(t & T_ERROR) {
		return "Error";
	} else {
		return "Unknown";
	}
}

void SIString_ToString(SIValue str, char **buf, size_t *bufferLen, size_t *bytesWritten) {
	size_t strLen = strlen(str.stringval);
	if(*bufferLen - *bytesWritten < strLen) {
		*bufferLen += strLen;
		*buf = rm_realloc(*buf, *bufferLen);
	}
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "%s", str.stringval);
}

void SIValue_ToString(SIValue v, char **buf, size_t *bufferLen, size_t *bytesWritten) {
	// uint64 max and int64 min are 21 bytes long
	// float defaults to print 6 digit after the decimal-point
	// checkt for enough space
	if(*bufferLen - *bytesWritten < 64) {
		*bufferLen += 64;
		*buf = rm_realloc(*buf, sizeof(char) * *bufferLen);
	}

	switch(v.type) {
	case T_STRING:
		SIString_ToString(v, buf, bufferLen, bytesWritten);
		break;
	case T_INT64:
		*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "%lld", (long long)v.longval);
		break;
	case T_BOOL:
		*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "%s", v.longval ? "true" : "false");
		break;
	case T_DOUBLE:
		*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "%f", v.doubleval);
		break;
	case T_NODE:
		Node_ToString(v.ptrval, buf, bufferLen, bytesWritten, ENTITY_ID);
		break;
	case T_EDGE:
		Edge_ToString(v.ptrval, buf, bufferLen, bytesWritten, ENTITY_ID);
		break;
	case T_ARRAY:
		SIArray_ToString(v, buf, bufferLen, bytesWritten);
		break;
	case T_NULL:
	default:
		*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "NULL");
	}
}

int SIValue_ToDouble(const SIValue *v, double *d) {
	switch(v->type) {
	case T_DOUBLE:
		*d = v->doubleval;
		return 1;
	case T_INT64:
	case T_BOOL:
		*d = (double)v->longval;
		return 1;

	default:
		// cannot convert!
		return 0;
	}
}

SIValue SIValue_FromString(const char *s) {
	char *sEnd = NULL;

	errno = 0;
	double parsedval = strtod(s, &sEnd);
	/* The input was not a complete number or represented a number that
	 * cannot be represented as a double.
	 * Create a string SIValue. */
	if(sEnd[0] != '\0' || errno == ERANGE) {
		return SI_DuplicateStringVal(s);
	}

	// The input was fully converted; create a double SIValue.
	return SI_DoubleVal(parsedval);
}

size_t SIValue_StringConcatLen(SIValue *strings, unsigned int string_count) {
	size_t length = 0;
	size_t elem_len;

	/* Compute length. */
	for(int i = 0; i < string_count; i ++) {
		/* String elements representing bytes size strings,
		 * for all other SIValue types 32 bytes should be enough. */
		elem_len = (strings[i].type == T_STRING) ? strlen(strings[i].stringval) + 1 : 32;
		length += elem_len;
	}

	/* Account for NULL terminating byte. */
	length += string_count + 1;
	return length;
}

size_t SIValue_StringConcat(SIValue *strings, unsigned int string_count, char **buf,
							size_t *buf_len, size_t *bytesWritten) {

	for(int i = 0; i < string_count; i ++) {
		SIValue_ToString(strings[i], buf, buf_len, bytesWritten);
		if(i < string_count - 1) *bytesWritten += snprintf(*buf + *bytesWritten, *buf_len, ",");
	}
	return *bytesWritten;
}

// assumption: either a or b is a string
SIValue SIValue_ConcatString(const SIValue a, const SIValue b) {
	SIValue result;
	size_t bufferLen = 512;
	size_t argument_len = 0;
	char *buffer = rm_calloc(bufferLen, sizeof(char));
	char *string_arg = NULL;
	// in case a is not a string - concat scalar + string
	if(a.type != T_STRING && a.type != T_CONSTSTRING) {
		/* a is numeric, convert to string. */
		SIValue_ToString(a, &buffer, &bufferLen, &argument_len);
		result = SI_DuplicateStringVal(buffer);
	}
	// a is a string - concat string + value
	else result = SI_DuplicateStringVal(a.stringval);

	if(b.type != T_STRING && b.type != T_CONSTSTRING) {
		/* b is not a string, get a string representation. */
		SIValue_ToString(b, &buffer, &bufferLen, &argument_len);
		string_arg = buffer;
	} else {
		string_arg = b.stringval;
		argument_len = strlen(string_arg);
	}

	/* Concat, make sure result has enough space to hold new string. */
	unsigned int required_size = strlen(result.stringval) + argument_len + 1;
	result.stringval = rm_realloc(result.stringval, required_size);
	strcat(result.stringval, string_arg);
	rm_free(buffer);
	return result;
}

// assumption: either a or b is a list
SIValue SIValue_ConcatList(const SIValue a, const SIValue b) {
	SIValue resultArray;
	// scalar + array
	if(a.type != T_ARRAY) {
		SIValue resultArray = SI_Array(1 + SIArray_Length(b));
		SIArray_Append(&resultArray, a);
	}
	// array + value
	else {
		resultArray = SI_CloneValue(a);
	}
	// b is scalar
	if(b.type != T_ARRAY) {
		SIArray_Append(&resultArray, b);
	} else {
		uint bArrayLen = SIArray_Length(b);
		for(uint i = 0; i < bArrayLen; i++) {
			SIArray_Append(&resultArray, SIArray_Get(b, i));
		}
	}
	return resultArray;
}

SIValue SIValue_Add(const SIValue a, const SIValue b) {
	if(a.type == T_NULL || b.type == T_NULL) return SI_NullVal();
	if(a.type == T_ARRAY || b.type == T_ARRAY) return SIValue_ConcatList(a, b);
	if((a.type == T_STRING || a.type == T_CONSTSTRING) || (b.type == T_STRING ||
														   b.type == T_CONSTSTRING)) return SIValue_ConcatString(a, b);
	/* Only construct an integer return if both operands are integers. */
	if(a.type & b.type & T_INT64) {
		return SI_LongVal(a.longval + b.longval);
	}
	/* Return a double representation. */
	return SI_DoubleVal(SI_GET_NUMERIC(a) + SI_GET_NUMERIC(b));
}

SIValue SIValue_Subtract(const SIValue a, const SIValue b) {
	/* Only construct an integer return if both operands are integers. */
	if(a.type & b.type & T_INT64) {
		return SI_LongVal(a.longval - b.longval);
	}
	/* Return a double representation. */
	return SI_DoubleVal(SI_GET_NUMERIC(a) - SI_GET_NUMERIC(b));
}

SIValue SIValue_Multiply(const SIValue a, const SIValue b) {
	/* Only construct an integer return if both operands are integers. */
	if(a.type & b.type & T_INT64) {
		return SI_LongVal(a.longval * b.longval);
	}
	/* Return a double representation. */
	return SI_DoubleVal(SI_GET_NUMERIC(a) * SI_GET_NUMERIC(b));
}

SIValue SIValue_Divide(const SIValue a, const SIValue b) {
	/* Always perform floating-point division. */
	return SI_DoubleVal(SI_GET_NUMERIC(a) / (double)SI_GET_NUMERIC(b));
}

int SIArray_Compare(SIValue arrayA, SIValue arrayB) {
	uint arrayALen = SIArray_Length(arrayA);
	uint arrayBLen = SIArray_Length(arrayB);
	// check empty list
	if(arrayALen == 0 && arrayBLen == 0) return 0;
	int lenDiff = arrayALen - arrayBLen;
	// check for the common range of indices
	uint minLengh = arrayALen < arrayBLen ? arrayALen : arrayBLen;

	uint allDisjoint = 0;   // counter for the amount of disjoint comparisons
	uint nullCompare = 0;   // counter for the amount of null comparison
	int notEqual =
		0;       // will hold the first false (result != 0) comparison result between two values from the same type, which are not equal

	// go over the common range for both arrays
	for(uint i = 0; i < minLengh; i++) {
		SIValue aValue = SIArray_Get(arrayA, i);
		SIValue bValue = SIArray_Get(arrayB, i);
		int compareResult = SIValue_Compare(aValue, bValue);
		switch(compareResult) {
		case 0:
			// we have a matching value
			break;
		case COMPARED_NULL:
			// there was a null comparison
			nullCompare++;  // update null comparison counter
			allDisjoint++;  // null comparison is also a disjoint comparison
			break;
		case DISJOINT:
			allDisjoint++;  // there was a disjoint comparison
			if(notEqual == 0) notEqual =
					compareResult; // if there wasn't any false comparison, update the false comparison to disjoint value
			break;
		default:
			// if comparison is not 0 (a != b), set the first value
			if(notEqual == 0) notEqual = compareResult;
			break;
		}
	}
	// if all the elements in the shared range are from disjoint types return DISJOINT array
	if(allDisjoint == minLengh && allDisjoint > nullCompare) return DISJOINT;
	// if there was a null comperison on non disjoint arrays
	if(nullCompare) return COMPARED_NULL;
	// if there was a diffrence in some member
	if(notEqual) return notEqual;
	// if all elemnts are equal and length are equal so arrays are equal, otherwise return lengh diff
	return lenDiff;
}

int SIValue_Compare(const SIValue a, const SIValue b) {
	/* In order to be comparable, both SIValues must be from the same type */
	if(a.type == T_NULL || b.type == T_NULL) return COMPARED_NULL;
	if(a.type == b.type) {
		switch(a.type) {
		case T_INT64:
		case T_BOOL:
			return a.longval - b.longval;
		case T_DOUBLE:
			return SAFE_COMPARISON_RESULT(a.doubleval - b.doubleval);
		case T_STRING:
			return strcmp(a.stringval, b.stringval);
		case T_NODE:
		case T_EDGE:
			return ENTITY_GET_ID((GraphEntity *)a.ptrval) - ENTITY_GET_ID((GraphEntity *)b.ptrval);
		case T_ARRAY:
			return SIArray_Compare(a, b);
		default:
			// Both inputs were of an incomparable type, like a pointer or NULL
			return DISJOINT;
		}
	}

	/* The inputs have different SITypes - compare them if they
	 * are both numerics of differing types */
	if(SI_TYPE(a) & SI_NUMERIC && SI_TYPE(b) & SI_NUMERIC) {
		double diff = SI_GET_NUMERIC(a) - SI_GET_NUMERIC(b);
		return SAFE_COMPARISON_RESULT(diff);
	}

	return DISJOINT;
}

// Return a strcmp-like value wherein -1 indicates that the value
// on the left-hand side is lesser, and 1 indicates that is greater.
int SIValue_Order(const SIValue a, const SIValue b) {
	// If the values are directly comparable, return the comparison result
	int cmp = SIValue_Compare(a, b);
	if(cmp != DISJOINT && cmp != COMPARED_NULL) return cmp;

	// Cypher's orderability property defines string < boolean < numeric < NULL.
	if(a.type == T_STRING) {
		return -1;
	} else if(b.type == T_STRING) {
		return 1;
	} else if(a.type == T_BOOL) {
		return -1;
	} else if(b.type == T_BOOL) {
		return 1;
	} else if(a.type & SI_NUMERIC) {
		return -1;
	} else if(b.type & SI_NUMERIC) {
		return 1;
	}

	// We can reach here if both values are NULL, in which case no order is imposed.
	return 0;
}

void SIValue_Free(SIValue *v) {
	// The free routine only performs work if it owns a heap allocation.
	if(v->allocation != M_SELF) return;

	switch(v->type) {
	case T_STRING:
		rm_free(v->stringval);
		v->stringval = NULL;
		return;
	case T_NODE:
	case T_EDGE:
		rm_free(v->ptrval);
		return;
	case T_ARRAY:
		SIArray_Free(*v);
	default:
		return;
	}
}
