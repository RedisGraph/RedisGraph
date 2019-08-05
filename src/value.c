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
#include "util/arr.h"

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
SIValue SI_Array(SIValue *array) {
	return (SIValue) {
		.array = array, .type = T_ARRAY, .allocation = M_VOLATILE
	};
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
int SIArray_ToString(SIValue *array, char *buf, size_t len) {
	int bytes_written = snprintf(buf, len, "[");
	len -= bytes_written;
	uint arrayLen = array_len(array);
	for(uint i = 0; i < arrayLen; i ++) {
		int b = SIValue_ToString(array[i], buf + bytes_written, len);
		b += snprintf(buf + b, len - b, ", ");
		bytes_written += b;
		len -= b;
	}
	bytes_written += snprintf(buf, len,  "]");
	return bytes_written;
}

int SIValue_ToString(SIValue v, char *buf, size_t len) {
	int bytes_written = 0;

	switch(v.type) {
	case T_STRING:
		strncpy(buf, v.stringval, len);
		bytes_written = strlen(buf);
		break;
	case T_INT64:
		bytes_written = snprintf(buf, len, "%lld", (long long)v.longval);
		break;
	case T_BOOL:
		bytes_written = snprintf(buf, len, "%s", v.longval ? "true" : "false");
		break;
	case T_DOUBLE:
		bytes_written = snprintf(buf, len, "%f", v.doubleval);
		break;
	case T_NODE:
	case T_EDGE:
		bytes_written = snprintf(buf, len, "%llu", ENTITY_GET_ID((GraphEntity *)v.ptrval));
		break;
	case T_ARRAY:
		bytes_written = SIArray_ToString(v.array, buf, len);
		break;
	case T_NULL:
	default:
		bytes_written = snprintf(buf, len, "NULL");
	}

	return bytes_written;
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

size_t SIValue_StringConcat(SIValue *strings, unsigned int string_count, char *buf,
							size_t buf_len) {
	size_t offset = 0;

	for(int i = 0; i < string_count; i ++) {
		offset += SIValue_ToString(strings[i], buf + offset, buf_len - offset - 1);
		buf[offset++] = ',';
	}
	/* Backtrack once and discard last delimiter. */
	buf[--offset] = '\0';

	return offset;
}

SIValue SIValue_Add(const SIValue a, const SIValue b) {
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

int SIArray_Compare(SIValue *arrayA, SIValue *arrayB) {

	uint arrayALen = array_len(arrayA);
	uint arrayBLen = array_len(arrayB);
	// check empty list
	if(arrayALen == 0 && arrayBLen == 0) return 0;
	int lenDiff = arrayALen - arrayBLen;
	// check for the common range of indices
	uint minLengh = arrayALen < arrayBLen ? arrayALen : arrayBLen;

	bool allDisjoint = true;
	bool nullCompare = false;

	// go over the common range for both arrays
	for(uint i = 0; i < minLengh; i++) {
		SIValue aValue = arrayA[i];
		SIValue bValue = arrayB[i];
		int compareResult = SIValue_Compare(aValue, bValue);
		switch(compareResult) {
		case 0:
			// we have a matching value
			allDisjoint = false;
			break;
		case COMPARED_NULL:
			// there was a null comparison
			allDisjoint = false;
			nullCompare = true;
			break;
		default:
			// if comparison is not 0 (a != b), return it.
			return compareResult;
		}
	}
	// if all the elements in the shared range are from disjoint types return DISJOINT array
	if(allDisjoint) return DISJOINT;
	// if all elemnts are equal and length are equal so arrays are equal
	if(lenDiff) return lenDiff;

	// both arrays are in same length, we need to check if one  of them contains null,
	// if so then it null compared, other wise they are equal
	return nullCompare ? COMPARED_NULL : 0;
}

int SIValue_Compare(const SIValue a, const SIValue b) {
	/* In order to be comparable, both SIValues must be strings,
	 * booleans, or numerics. */
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
			return SIArray_Compare(a.array, b.array);
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
	default:
		return;
	}
}
