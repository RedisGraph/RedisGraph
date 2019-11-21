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
#include "datatypes/path/sipath.h"

static inline void _SIString_ToString(SIValue str, char **buf, size_t *bufferLen,
									  size_t *bytesWritten) {
	size_t strLen = strlen(str.stringval);
	if(*bufferLen - *bytesWritten < strLen) {
		*bufferLen += strLen;
		*buf = rm_realloc(*buf, *bufferLen);
	}
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "%s", str.stringval);
}

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

SIValue SI_Path(void *p) {
	Path *path = (Path *)p;
	return SIPath_New(path);
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

	if(v.type == T_PATH) {
		return SIPath_Clone(v);
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
	if(t & T_STRING) {
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
	} else if(t & T_ARRAY) {
		return "List";
	} else if(t & T_PATH) {
		return "Path";
	} else if(t & T_NULL) {
		return "Null";
	} else {
		return "Unknown";
	}
}

void SIValue_ToString(SIValue v, char **buf, size_t *bufferLen, size_t *bytesWritten) {
	// uint64 max and int64 min string representation requires 21 bytes
	// float defaults to print 6 digit after the decimal-point
	// checkt for enough space
	if(*bufferLen - *bytesWritten < 64) {
		*bufferLen += 64;
		*buf = rm_realloc(*buf, sizeof(char) * *bufferLen);
	}

	switch(v.type) {
	case T_STRING:
		_SIString_ToString(v, buf, bufferLen, bytesWritten);
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
	case T_PATH:
		SIPath_ToString(v, buf, bufferLen, bytesWritten);
		break;
	case T_NULL:
		*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "NULL");
		break;
	case T_PTR:
		*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "POINTER");
		break;
	default:
		// unrecognized type
		printf("unrecognized type: %d\n", v.type);
		assert(false);
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

size_t SIValue_StringJoinLen(SIValue *strings, unsigned int string_count, const char *delimiter) {
	size_t length = 0;
	size_t elem_len;
	size_t delimiter_len = strlen(delimiter);
	/* Compute length. */
	for(int i = 0; i < string_count; i ++) {
		/* String elements representing bytes size strings,
		 * for all other SIValue types 32 bytes should be enough. */
		elem_len = (strings[i].type == T_STRING) ? strlen(strings[i].stringval) + delimiter_len : 32;
		length += elem_len;
	}

	/* Account for NULL terminating byte. */
	length += string_count + 1;
	return length;
}

void SIValue_StringJoin(SIValue *strings, unsigned int string_count, const char *delimiter,
						char **buf, size_t *buf_len, size_t *bytesWritten) {

	for(int i = 0; i < string_count; i ++) {
		SIValue_ToString(strings[i], buf, buf_len, bytesWritten);
		if(i < string_count - 1) *bytesWritten += snprintf(*buf + *bytesWritten, *buf_len, "%s", delimiter);
	}
}

// assumption: either a or b is a string
static SIValue SIValue_ConcatString(const SIValue a, const SIValue b) {
	size_t bufferLen = 512;
	size_t argument_len = 0;
	char *buffer = rm_calloc(bufferLen, sizeof(char));
	SIValue args[2] = {a, b};
	SIValue_StringJoin(args, 2, "", &buffer, &bufferLen, &argument_len);
	SIValue result = SI_DuplicateStringVal(buffer);
	rm_free(buffer);
	return result;
}

// assumption: either a or b is a list - static function, the caller validate types
static SIValue SIValue_ConcatList(const SIValue a, const SIValue b) {
	uint a_len = (a.type == T_ARRAY) ? SIArray_Length(a) : 1;
	uint b_len = (b.type == T_ARRAY) ? SIArray_Length(b) : 1;
	SIValue resultArray = SI_Array(a_len + b_len);

	// Append a to resultArray
	if(a.type == T_ARRAY) {
		// in thae case of a is an array
		for(uint i = 0; i < a_len; i++) SIArray_Append(&resultArray, SIArray_Get(a, i));
	} else {
		// in thae case of a is not an array
		SIArray_Append(&resultArray, a);
	}

	if(b.type == T_ARRAY) {
		// b is an array
		uint bArrayLen = SIArray_Length(b);
		for(uint i = 0; i < bArrayLen; i++) {
			SIArray_Append(&resultArray, SIArray_Get(b, i));
		}
	} else {
		// b is not an array
		SIArray_Append(&resultArray, b);
	}
	return resultArray;
}

SIValue SIValue_Add(const SIValue a, const SIValue b) {
	if(a.type == T_NULL || b.type == T_NULL) return SI_NullVal();
	if(a.type == T_ARRAY || b.type == T_ARRAY) return SIValue_ConcatList(a, b);
	if(a.type == T_STRING || b.type == T_STRING) return SIValue_ConcatString(a, b);
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

int SIArray_Compare(SIValue arrayA, SIValue arrayB, int *disjointOrNull) {
	uint arrayALen = SIArray_Length(arrayA);
	uint arrayBLen = SIArray_Length(arrayB);
	// Check empty list.
	if(arrayALen == 0 && arrayBLen == 0) return 0;
	int lenDiff = arrayALen - arrayBLen;
	// Check for the common range of indices.
	uint minLength = arrayALen <= arrayBLen ? arrayALen : arrayBLen;
	// notEqual holds the first false (result != 0) comparison result between two values from the same type, which are not equal.
	int notEqual = 0;
	uint nullCounter = 0;       // Counter for the amount of null comparison.
	uint notEqualCounter = 0;   // Counter for the amount of false (compare(a,b) !=0) comparisons.

	// Go over the common range for both arrays.
	for(uint i = 0; i < minLength; i++) {
		SIValue aValue = SIArray_Get(arrayA, i);
		SIValue bValue = SIArray_Get(arrayB, i);
		// Current comparison special cases indication variable.
		int currentDisjointOrNull = 0;
		int compareResult = SIValue_Compare(aValue, bValue, &currentDisjointOrNull);
		// In case of special case such null or disjoint comparison.
		if(currentDisjointOrNull) {
			if(currentDisjointOrNull == COMPARED_NULL) nullCounter++;   // Update null comparison counter.
			// Null or disjoint comparison is also a false comparison, so increase the number of false comparisons in one.
			notEqualCounter++;
			// Set the first difference value, if not set before.
			if(notEqual == 0) notEqual = compareResult;
		} else if(compareResult != 0) {
			// In the normal false comparison case, update false comparison counter.
			notEqualCounter++;
			// Set the first difference value, if not set before.
			if(notEqual == 0) notEqual = compareResult;
		}
		// Note: In the case of compareResult = 0, there is nothing to be done.
	}
	// If all the elements in the shared range yielded false comparisons.
	if(notEqualCounter == minLength && notEqualCounter > nullCounter) return notEqual;
	// If there was a null comperison on non disjoint arrays.
	if(nullCounter) {
		if(disjointOrNull) *disjointOrNull = COMPARED_NULL;
		return notEqual;
	}
	// If there was a difference in some member, without any null compare.
	if(notEqual) return notEqual;
	// In this state, the common range is equal. We return lenDiff, which is 0 in case the lists are equal, and not 0 otherwise.
	return lenDiff;
}

int SIValue_Compare(const SIValue a, const SIValue b, int *disjointOrNull) {

	/* No special case (null or disjoint comparison) happened yet.
	 * If indication for such cases is required, first set the indication value to zero (not happen). */
	if(disjointOrNull) *disjointOrNull = 0;

	/* In order to be comparable, both SIValues must be from the same type. */
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
			return SIArray_Compare(a, b, disjointOrNull);
		case T_NULL:
			break;
		default:
			// Both inputs were of an incomparable type, like a pointer, or not implemented comparison yet.
			assert(false);
		}
	}

	/* The inputs have different SITypes - compare them if they
	 * are both numerics of differing types. */
	if(SI_TYPE(a) & SI_NUMERIC && SI_TYPE(b) & SI_NUMERIC) {
		double diff = SI_GET_NUMERIC(a) - SI_GET_NUMERIC(b);
		return SAFE_COMPARISON_RESULT(diff);
	}

	// Check if either type is null.
	if(a.type == T_NULL || b.type == T_NULL) {
		// Check if indication is required and inform about null comparison.
		if(disjointOrNull) *disjointOrNull = COMPARED_NULL;
	} else {
		// Check if indication is required, and inform about disjoint comparison.
		if(disjointOrNull) *disjointOrNull = DISJOINT;
	}

	// In case of disjoint or null comparison, return value type difference.
	return a.type - b.type;
}

/* Hashes the id and properties of the node*/
XXH64_hash_t SINode_HashCode(const SIValue v) {
	XXH_errorcode res;
	XXH64_state_t state;
	res = XXH64_reset(&state, 0);
	assert(res != XXH_ERROR);

	Node *n = (Node *)v.ptrval;
	int id = ENTITY_GET_ID(n);
	SIType t = SI_TYPE(v);
	XXH64_update(&state, &(t), sizeof(t));
	XXH64_update(&state, &id, sizeof(id));

	XXH64_hash_t hashCode = XXH64_digest(&state);
	return hashCode;
}

/* Hashes the id and properties of the edge. */
XXH64_hash_t SIEdge_HashCode(const SIValue v) {

	XXH_errorcode res;
	XXH64_state_t state;
	res = XXH64_reset(&state, 0);
	assert(res != XXH_ERROR);

	Edge *e = (Edge *)v.ptrval;
	int id = ENTITY_GET_ID(e);
	SIType t = SI_TYPE(v);
	XXH64_update(&state, &(t), sizeof(t));
	XXH64_update(&state, &id, sizeof(id));

	XXH64_hash_t hashCode = XXH64_digest(&state);
	return hashCode;
}

/* This method hashes primitive types in place. Compound types have their own
 * hashing method */
XXH64_hash_t SIValue_HashCode(SIValue v) {
	XXH_errorcode res;
	XXH64_state_t state;
	res = XXH64_reset(&state, 0);
	assert(res != XXH_ERROR);
	// Handles null value and defaults.
	int64_t null = 0;
	/* In case of identical binary representation of the value,
	* we should hash the type as well. */
	SIType t = SI_TYPE(v);

	switch(v.type) {
	case T_NULL:
		XXH64_update(&state, &t, sizeof(t));
		XXH64_update(&state, &null, sizeof(null));
		break;
	case T_STRING:
		XXH64_update(&state, &t, sizeof(t));
		XXH64_update(&state, v.stringval, strlen(v.stringval));
		break;
	case T_INT64:
		// Change type to numeric.
		t = SI_NUMERIC;
		XXH64_update(&state, &t, sizeof(t));
		XXH64_update(&state, &v.longval, sizeof(v.longval));
		break;
	case T_BOOL:
		XXH64_update(&state, &t, sizeof(t));
		XXH64_update(&state, &v.longval, sizeof(v.longval));
		break;
	case T_DOUBLE: {
		t = SI_NUMERIC;
		XXH64_update(&state, &t, sizeof(t));
		// Check if the double value is actually an interger. If so, hash it as Long.
		int64_t casted = (int64_t) v.doubleval;
		double diff = v.doubleval - casted;
		if(diff != 0) XXH64_update(&state, &v.doubleval, sizeof(v.doubleval));
		else XXH64_update(&state, &casted, sizeof(casted));
		break;
	}
	case T_EDGE:
		return SIEdge_HashCode(v);
	case T_NODE:
		return SINode_HashCode(v);
	case T_ARRAY:
		return SIArray_HashCode(v);
	case T_PATH:
		return SIPath_HashCode(v);
	// TODO: Implement for Map and temporal types once we support them.
	default:
		assert(false);
	}
	XXH64_hash_t hashCode =  XXH64_digest(&state);
	return hashCode;
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

