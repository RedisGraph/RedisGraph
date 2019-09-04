/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./arithmetic_expression.h"

#include "funcs.h"
#include "rax.h"
#include "./aggregate.h"
#include "../util/arr.h"
#include "./repository.h"
#include "../graph/graph.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../datatypes/temporal_value.h"
#include "../datatypes/array.h"

#include "assert.h"
#include <math.h>
#include <ctype.h>

/* Arithmetic function repository. */
static rax *__aeRegisteredFuncs = NULL;

static void inline _toLower(const char *str, char *lower, short *lower_len) {
	size_t str_len = strlen(str);
	/* Avoid overflow. */
	assert(*lower_len > str_len);

	/* Update lower len*/
	*lower_len = str_len;

	int i = 0;
	for(; i < str_len; i++) lower[i] = tolower(str[i]);
	lower[i] = 0;
}

/* Returns 1 if the operand is a numeric type, and 0 otherwise.
 * This also rejects NULL values. */
static inline int _validate_numeric(const SIValue v) {
	return SI_TYPE(v) & SI_NUMERIC;
}

/* Register an arithmetic function. */
static void _AR_RegFunc(char *func_name, short func_name_len, AR_Func func) {
	if(__aeRegisteredFuncs == NULL) {
		__aeRegisteredFuncs = raxNew();
	}

	raxInsert(__aeRegisteredFuncs, (unsigned char *)func_name, func_name_len, func, NULL);
}

/* Get arithmetic function. */
static AR_Func _AR_GetFunc(const char *func_name) {
	char lower_func_name[32] = {0};
	short lower_func_name_len = 32;
	_toLower(func_name, &lower_func_name[0], &lower_func_name_len);
	void *f = raxFind(__aeRegisteredFuncs, (unsigned char *)lower_func_name, lower_func_name_len);
	if(f != raxNotFound) {
		return f;
	}
	return NULL;
}

/* Update arithmetic expression variable node by setting node's property index.
 * when constructing an arithmetic expression we'll delay setting graph entity
 * attribute index to the first execution of the expression, this is due to
 * entity aliasing where we lose track over which entity is aliased, consider
 * MATCH (n:User) WITH n AS x RETURN x.name
 * When constructing the arithmetic expression x.name, we don't know
 * who X is referring to. */
static void _AR_EXP_UpdatePropIdx(AR_ExpNode *root, const Record r) {
	GraphContext *gc = GraphContext_GetFromTLS();
	root->operand.variadic.entity_prop_idx = GraphContext_GetAttributeID(gc,
																		 root->operand.variadic.entity_prop);
}

AR_ExpNode *AR_EXP_NewOpNode(const char *func_name, uint child_count) {
	assert(func_name);

	AR_ExpNode *node = rm_calloc(1, sizeof(AR_ExpNode));
	node->type = AR_EXP_OP;
	node->op.func_name = func_name;
	node->op.child_count = child_count;
	node->op.children = rm_malloc(child_count * sizeof(AR_ExpNode *));

	/* Determine function type. */
	AR_Func func = _AR_GetFunc(func_name);
	if(func != NULL) {
		node->op.f = func;
		node->op.type = AR_OP_FUNC;
	} else {
		/* Either this is an aggregation function
		 * or the requested function does not exists. */
		AggCtx *agg_func;
		Agg_GetFunc(func_name, &agg_func);

		/* TODO: handle Unknown function. */
		assert(agg_func != NULL);
		node->op.agg_func = agg_func;
		node->op.type = AR_OP_AGGREGATE;
	}

	return node;
}

AR_ExpNode *AR_EXP_NewVariableOperandNode(RecordMap *record_map, const char *alias,
										  const char *prop) {
	AR_ExpNode *node = rm_malloc(sizeof(AR_ExpNode));
	node->type = AR_EXP_OPERAND;
	node->operand.type = AR_EXP_VARIADIC;
	node->operand.variadic.entity_alias = alias;
	if(alias) {
		node->operand.variadic.entity_alias_idx = RecordMap_FindOrAddAlias(record_map, alias);
	} else {
		node->operand.variadic.entity_alias_idx = IDENTIFIER_NOT_FOUND;
	}
	node->operand.variadic.entity_prop = prop;
	node->operand.variadic.entity_prop_idx = ATTRIBUTE_NOTFOUND;

	return node;
}

AR_ExpNode *AR_EXP_NewConstOperandNode(SIValue constant) {
	AR_ExpNode *node = rm_malloc(sizeof(AR_ExpNode));
	node->type = AR_EXP_OPERAND;
	node->operand.type = AR_EXP_CONSTANT;
	node->operand.constant = constant;
	return node;
}

int AR_EXP_GetOperandType(AR_ExpNode *exp) {
	if(exp->type == AR_EXP_OPERAND) return exp->operand.type;
	return -1;
}

/* Compact tree by evaluating constant expressions
 * e.g. MINUS(X) where X is a constant number will be reduced to
 * a single node with the value -X
 * PLUS(MINUS(A), B) will be reduced to a single constant: B-A. */
bool AR_EXP_ReduceToScalar(AR_ExpNode **root) {
	if((*root)->type == AR_EXP_OPERAND) {
		if((*root)->operand.type == AR_EXP_CONSTANT) {
			// Root is already a constant
			return true;
		}
		// Root is variadic, no way to reduce.
		return false;
	} else {
		// root represents an operation.
		assert((*root)->type == AR_EXP_OP);

		if((*root)->op.type == AR_OP_FUNC) {
			/* See if we're able to reduce each child of root
			 * if so we'll be able to reduce root. */
			bool reduce_children = true;
			for(int i = 0; i < (*root)->op.child_count; i++) {
				if(!AR_EXP_ReduceToScalar((*root)->op.children + i)) {
					reduce_children = false;
					break;
				}
			}
			// Can't reduce root as one of its children is not a constant.
			if(!reduce_children) return false;

			// All child nodes are constants, reduce.
			SIValue v = AR_EXP_Evaluate(*root, NULL);
			AR_EXP_Free(*root);
			*root = AR_EXP_NewConstOperandNode(v);
			return true;
		}

		// Root is an aggregation function, can't reduce.
		return false;
	}
}

SIValue AR_EXP_Evaluate(AR_ExpNode *root, const Record r) {
	SIValue result;
	if(root->type == AR_EXP_OP) {
		/* Aggregation function should be reduced by now.
		 * TODO: verify above statement. */
		if(root->op.type == AR_OP_AGGREGATE) {
			AggCtx *agg = root->op.agg_func;
			// The AggCtx will ultimately free its result.
			result = SI_ShareValue(agg->result);
		} else {
			/* Evaluate each child before evaluating current node. */
			SIValue sub_trees[root->op.child_count];
			for(int child_idx = 0; child_idx < root->op.child_count; child_idx++) {
				sub_trees[child_idx] = AR_EXP_Evaluate(root->op.children[child_idx], r);
			}
			/* Evaluate self. */
			result = root->op.f(sub_trees, root->op.child_count);

			// release memory if needed
			/* for example 'a'+'b'+'c'+'d' will evalute to
			          +(abcd)
			         /       \
			     +(ab)      +(cd)
			    /   \       /   \
			   a     b     c     d
			   the expressions 'ab', 'cd' which are calculated temporaty values
			   will be released once the calculation of 'abcd' is done
			*/
			for(int child_idx = 0; child_idx < root->op.child_count; child_idx++) {
				SIValue_Free(&sub_trees[child_idx]);
			}

		}
	} else {
		/* Deal with a constant node. */
		if(root->operand.type == AR_EXP_CONSTANT) {
			// The value is constant or has been computed elsewhere, and is shared with the caller.
			result = SI_ShareValue(root->operand.constant);
		} else {
			// Fetch entity property value.
			if(root->operand.variadic.entity_prop != NULL) {
				RecordEntryType t = Record_GetType(r, root->operand.variadic.entity_alias_idx);
				// Property requested on a scalar value.
				// TODO this should issue a TypeError
				if(t != REC_TYPE_NODE && t != REC_TYPE_EDGE) return SI_NullVal();

				GraphEntity *ge = Record_GetGraphEntity(r, root->operand.variadic.entity_alias_idx);
				if(root->operand.variadic.entity_prop_idx == ATTRIBUTE_NOTFOUND) {
					_AR_EXP_UpdatePropIdx(root, r);
				}
				SIValue *property = GraphEntity_GetProperty(ge, root->operand.variadic.entity_prop_idx);
				if(property == PROPERTY_NOTFOUND) {
					result = SI_NullVal();
				} else {
					// The value belongs to a graph property, and can be accessed safely during the query lifetime.
					result = SI_ConstValue(*property);
				}
			} else {
				// Alias doesn't necessarily refers to a graph entity,
				// it could also be a constant.
				int aliasIdx = root->operand.variadic.entity_alias_idx;
				// The value was not created here; share with the caller.
				result = SI_ShareValue(Record_Get(r, aliasIdx));
			}
		}
	}
	return result;
}

void AR_EXP_Aggregate(const AR_ExpNode *root, const Record r) {
	if(root->type == AR_EXP_OP) {
		if(root->op.type == AR_OP_AGGREGATE) {
			/* Process child nodes before aggregating. */
			SIValue sub_trees[root->op.child_count];
			int i = 0;
			for(; i < root->op.child_count; i++) {
				AR_ExpNode *child = root->op.children[i];
				sub_trees[i] = AR_EXP_Evaluate(child, r);
			}

			/* Aggregate. */
			AggCtx *agg = root->op.agg_func;
			agg->Step(agg, sub_trees, root->op.child_count);
		} else {
			/* Keep searching for aggregation nodes. */
			for(int i = 0; i < root->op.child_count; i++) {
				AR_ExpNode *child = root->op.children[i];
				AR_EXP_Aggregate(child, r);
			}
		}
	}
}

void AR_EXP_Reduce(const AR_ExpNode *root) {
	if(root->type == AR_EXP_OP) {
		if(root->op.type == AR_OP_AGGREGATE) {
			/* Reduce. */
			AggCtx *agg = root->op.agg_func;
			Agg_Finalize(agg);
		} else {
			/* Keep searching for aggregation nodes. */
			for(int i = 0; i < root->op.child_count; i++) {
				AR_ExpNode *child = root->op.children[i];
				AR_EXP_Reduce(child);
			}
		}
	}
}

void AR_EXP_CollectEntityIDs(AR_ExpNode *root, rax *record_ids) {
	if(root->type == AR_EXP_OP) {
		for(int i = 0; i < root->op.child_count; i ++) {
			AR_EXP_CollectEntityIDs(root->op.children[i], record_ids);
		}
	} else { // type == AR_EXP_OPERAND
		if(root->operand.type == AR_EXP_VARIADIC) {
			int record_idx = root->operand.variadic.entity_alias_idx;
			assert(record_idx != IDENTIFIER_NOT_FOUND);
			raxInsert(record_ids, (unsigned char *)&record_idx, sizeof(record_idx), NULL, NULL);
		}
	}
}

void AR_EXP_CollectAttributes(AR_ExpNode *root, rax *attributes) {
	if(root->type == AR_EXP_OP) {
		for(int i = 0; i < root->op.child_count; i ++) {
			AR_EXP_CollectAttributes(root->op.children[i], attributes);
		}
	} else { // type == AR_EXP_OPERAND
		if(root->operand.type == AR_EXP_VARIADIC) {
			const char *attr = root->operand.variadic.entity_prop;
			if(attr) raxInsert(attributes, (unsigned char *)attr, strlen(attr), NULL, NULL);
		}
	}
}

bool AR_EXP_ContainsAggregation(AR_ExpNode *root, AR_ExpNode **agg_node) {
	if(root->type == AR_EXP_OP && root->op.type == AR_OP_AGGREGATE) {
		if(agg_node != NULL) *agg_node = root;
		return true;
	}

	if(root->type == AR_EXP_OP) {
		for(int i = 0; i < root->op.child_count; i++) {
			AR_ExpNode *child = root->op.children[i];
			if(AR_EXP_ContainsAggregation(child, agg_node)) {
				return true;
			}
		}
	}

	return false;
}

void _AR_EXP_ToString(const AR_ExpNode *root, char **str, size_t *str_size,
					  size_t *bytes_written) {
	/* Make sure there are at least 64 bytes in str. */
	if(*str == NULL) {
		*bytes_written = 0;
		*str_size = 128;
		*str = rm_calloc(*str_size, sizeof(char));
	}

	if((*str_size - strlen(*str)) < 64) {
		*str_size += 128;
		*str = rm_realloc(*str, sizeof(char) * *str_size);
	}
	/* Concat Op. */
	if(root->type == AR_EXP_OP) {
		/* Binary operation? */
		char binary_op = 0;

		if(strcmp(root->op.func_name, "ADD") == 0) binary_op = '+';
		else if(strcmp(root->op.func_name, "SUB") == 0) binary_op = '-';
		else if(strcmp(root->op.func_name, "MUL") == 0) binary_op = '*';
		else if(strcmp(root->op.func_name, "DIV") == 0)  binary_op = '/';

		if(binary_op) {
			_AR_EXP_ToString(root->op.children[0], str, str_size, bytes_written);

			/* Make sure there are at least 64 bytes in str. */
			if((*str_size - strlen(*str)) < 64) {
				*str_size += 128;
				*str = rm_realloc(*str, sizeof(char) * *str_size);
			}

			*bytes_written += sprintf((*str + *bytes_written), " %c ", binary_op);

			_AR_EXP_ToString(root->op.children[1], str, str_size, bytes_written);
		} else {
			/* Operation isn't necessarily a binary operation, use function call representation. */
			*bytes_written += sprintf((*str + *bytes_written), "%s(", root->op.func_name);

			for(int i = 0; i < root->op.child_count ; i++) {
				_AR_EXP_ToString(root->op.children[i], str, str_size, bytes_written);

				/* Make sure there are at least 64 bytes in str. */
				if((*str_size - strlen(*str)) < 64) {
					*str_size += 128;
					*str = rm_realloc(*str, sizeof(char) * *str_size);
				}
				if(i < (root->op.child_count - 1)) {
					*bytes_written += sprintf((*str + *bytes_written), ",");
				}
			}
			*bytes_written += sprintf((*str + *bytes_written), ")");
		}
	} else {
		// Concat Operand node.
		if(root->operand.type == AR_EXP_CONSTANT) {
			SIValue_ToString(root->operand.constant, str, str_size, bytes_written);
		} else {
			if(root->operand.variadic.entity_prop != NULL) {
				*bytes_written += sprintf(
									  (*str + *bytes_written), "%s.%s",
									  root->operand.variadic.entity_alias, root->operand.variadic.entity_prop);
			} else {
				*bytes_written += sprintf((*str + *bytes_written), "%s", root->operand.variadic.entity_alias);
			}
		}
	}
}

void AR_EXP_ToString(const AR_ExpNode *root, char **str) {
	size_t str_size = 0;
	size_t bytes_written = 0;
	*str = NULL;
	_AR_EXP_ToString(root, str, &str_size, &bytes_written);
}

static AR_ExpNode *_AR_EXP_CloneOperand(AR_ExpNode *exp) {
	AR_ExpNode *clone = rm_calloc(1, sizeof(AR_ExpNode));
	clone->type = AR_EXP_OPERAND;
	switch(exp->operand.type) {
	case AR_EXP_CONSTANT:
		clone->operand.type = AR_EXP_CONSTANT;
		clone->operand.constant = exp->operand.constant;
		break;
	case AR_EXP_VARIADIC:
		clone->operand.type = exp->operand.type;
		clone->operand.variadic.entity_alias = exp->operand.variadic.entity_alias;
		clone->operand.variadic.entity_alias_idx = exp->operand.variadic.entity_alias_idx;
		if(exp->operand.variadic.entity_prop) {
			clone->operand.variadic.entity_prop = exp->operand.variadic.entity_prop;
		}
		clone->operand.variadic.entity_prop_idx = exp->operand.variadic.entity_prop_idx;
		break;
	default:
		assert(false);
		break;
	}
	return clone;
}

static AR_ExpNode *_AR_EXP_CloneOp(AR_ExpNode *exp) {
	AR_ExpNode *clone = AR_EXP_NewOpNode(exp->op.func_name, exp->op.child_count);
	for(uint i = 0; i < exp->op.child_count; i++) {
		AR_ExpNode *child = AR_EXP_Clone(exp->op.children[i]);
		clone->op.children[i] = child;
	}
	return clone;
}

AR_ExpNode *AR_EXP_Clone(AR_ExpNode *exp) {
	AR_ExpNode *clone;
	switch(exp->type) {
	case AR_EXP_OPERAND:
		clone = _AR_EXP_CloneOperand(exp);
		break;
	case AR_EXP_OP:
		clone = _AR_EXP_CloneOp(exp);
		break;
	default:
		assert(false);
		break;
	}
	clone->resolved_name = exp->resolved_name;
	return clone;
}

void AR_EXP_Free(AR_ExpNode *root) {
	if(root->type == AR_EXP_OP) {
		for(int child_idx = 0; child_idx < root->op.child_count; child_idx++) {
			AR_EXP_Free(root->op.children[child_idx]);
		}
		rm_free(root->op.children);
		if(root->op.type == AR_OP_AGGREGATE) {
			AggCtx_Free(root->op.agg_func);
		}
	} else if(root->operand.type == AR_EXP_CONSTANT) {
		SIValue_Free(&root->operand.constant);
	}
	rm_free(root);
}

//==============================================================================
//=== Mathematical functions - numeric =========================================
//==============================================================================

SIValue AR_ADD(SIValue *argv, int argc) {
	// Don't modify input.
	SIValue result = SI_CloneValue(argv[0]);
	char buffer[512];
	char *string_arg = NULL;

	if(SIValue_IsNull(argv[0])) return SI_NullVal();
	for(int i = 1; i < argc; i++) {
		result = SIValue_Add(result, argv[i]);
	}
	return result;
}

SIValue AR_SUB(SIValue *argv, int argc) {
	SIValue result = argv[0];
	if(!_validate_numeric(result)) return SI_NullVal();

	for(int i = 1; i < argc; i++) {
		if(!_validate_numeric(argv[i])) return SI_NullVal();
		result = SIValue_Subtract(result, argv[i]);
	}
	return result;
}

SIValue AR_MUL(SIValue *argv, int argc) {
	SIValue result = argv[0];
	if(!_validate_numeric(result)) return SI_NullVal();

	for(int i = 1; i < argc; i++) {
		if(!_validate_numeric(argv[i])) return SI_NullVal();
		result = SIValue_Multiply(result, argv[i]);
	}
	return result;
}

SIValue AR_DIV(SIValue *argv, int argc) {
	SIValue result = argv[0];
	if(!_validate_numeric(result)) return SI_NullVal();

	for(int i = 1; i < argc; i++) {
		if(!_validate_numeric(argv[i])) return SI_NullVal();
		result = SIValue_Divide(result, argv[i]);
	}
	return result;
}

/* TODO All AR_* functions need to emit appropriate failures when provided
 * with arguments of invalid types and handle multiple arguments. */

SIValue AR_ABS(SIValue *argv, int argc) {
	SIValue result = argv[0];
	if(!_validate_numeric(result)) return SI_NullVal();
	if(SI_GET_NUMERIC(argv[0]) < 0) return SIValue_Multiply(argv[0], SI_LongVal(-1));
	return argv[0];
}

SIValue AR_CEIL(SIValue *argv, int argc) {
	SIValue result = argv[0];
	if(!_validate_numeric(result)) return SI_NullVal();
	// No modification is required for non-decimal values
	if(SI_TYPE(result) == T_DOUBLE) result.doubleval = ceil(result.doubleval);

	return result;
}

SIValue AR_FLOOR(SIValue *argv, int argc) {
	SIValue result = argv[0];
	if(!_validate_numeric(result)) return SI_NullVal();
	// No modification is required for non-decimal values
	if(SI_TYPE(result) == T_DOUBLE) result.doubleval = floor(result.doubleval);

	return result;
}

SIValue AR_RAND(SIValue *argv, int argc) {
	return SI_DoubleVal((double)rand() / (double)RAND_MAX);
}

SIValue AR_ROUND(SIValue *argv, int argc) {
	SIValue result = argv[0];
	if(!_validate_numeric(result)) return SI_NullVal();
	// No modification is required for non-decimal values
	if(SI_TYPE(result) == T_DOUBLE) result.doubleval = round(result.doubleval);

	return result;
}

SIValue AR_SIGN(SIValue *argv, int argc) {
	if(!_validate_numeric(argv[0])) return SI_NullVal();
	int64_t sign = SIGN(SI_GET_NUMERIC(argv[0]));
	return SI_LongVal(sign);
}

//==============================================================================
//=== String functions =========================================================
//==============================================================================

SIValue AR_LEFT(SIValue *argv, int argc) {
	assert(argc == 2);
	if(SIValue_IsNull(argv[0])) return SI_NullVal();

	assert(SI_TYPE(argv[0]) == T_STRING);
	assert(SI_TYPE(argv[1]) == T_INT64);

	int64_t newlen = argv[1].longval;
	if(strlen(argv[0].stringval) <= newlen) {
		// No need to truncate this string based on the requested length
		return SI_DuplicateStringVal(argv[0].stringval);
	}
	char *left_str = rm_malloc((newlen + 1) * sizeof(char));
	strncpy(left_str, argv[0].stringval, newlen * sizeof(char));
	left_str[newlen] = '\0';
	return SI_TransferStringVal(left_str);
}

SIValue AR_LTRIM(SIValue *argv, int argc) {
	if(SIValue_IsNull(argv[0])) return SI_NullVal();

	assert(argc == 1 && SI_TYPE(argv[0]) == T_STRING);

	char *trimmed = argv[0].stringval;

	while(*trimmed == ' ') {
		trimmed ++;
	}

	return SI_DuplicateStringVal(trimmed);
}

SIValue AR_RIGHT(SIValue *argv, int argc) {
	assert(argc == 2);
	if(SIValue_IsNull(argv[0])) return SI_NullVal();
	assert(SI_TYPE(argv[0]) == T_STRING);
	assert(SI_TYPE(argv[1]) == T_INT64);

	int64_t newlen = argv[1].longval;
	int64_t start = strlen(argv[0].stringval) - newlen;

	if(start <= 0) {
		// No need to truncate this string based on the requested length
		return SI_DuplicateStringVal(argv[0].stringval);
	}
	return SI_DuplicateStringVal(argv[0].stringval + start);
}

SIValue AR_RTRIM(SIValue *argv, int argc) {
	if(SIValue_IsNull(argv[0])) return SI_NullVal();
	assert(argc == 1 && SI_TYPE(argv[0]) == T_STRING);

	char *str = argv[0].stringval;

	size_t i = strlen(str);
	while(i > 0 && str[i - 1] == ' ') {
		i --;
	}

	char *trimmed = rm_malloc((i + 1) * sizeof(char));
	strncpy(trimmed, str, i);
	trimmed[i] = '\0';

	return SI_TransferStringVal(trimmed);
}

SIValue _reverseString(SIValue value) {
	char *str = value.stringval;
	size_t str_len = strlen(str);
	char *reverse = rm_malloc((str_len + 1) * sizeof(char));

	int i = str_len - 1;
	int j = 0;
	while(i >= 0) {
		reverse[j++] = str[i--];
	}
	reverse[j] = '\0';
	return SI_TransferStringVal(reverse);
}

SIValue _reverseArray(SIValue value) {
	uint arrayLen = SIArray_Length(value);
	SIValue result = SI_Array(arrayLen);
	for(uint i = arrayLen - 1; i >= 0; i--) {
		SIArray_Append(&result, SIArray_Get(value, i));
	}
	return result;
}

SIValue AR_REVERSE(SIValue *argv, int argc) {
	SIValue value = argv[0];
	if(SIValue_IsNull(value)) return SI_NullVal();
	// In case of string.
	if(SI_TYPE(value) == T_STRING) return _reverseString(value);
	// In case of array.
	if(SI_TYPE(value) == T_ARRAY) return _reverseArray(value);

	// TODO: Runtime error - unsupported type.
	assert(false);
}

SIValue AR_SUBSTRING(SIValue *argv, int argc) {
	/*
	    argv[0] - original string
	    argv[1] - start position
	    argv[2] - length
	    If length is omitted, the function returns the substring starting at the position given by start and extending to the end of original.
	    If either start or length is null or a negative integer, an error is raised.
	    If start is 0, the substring will start at the beginning of original.
	    If length is 0, the empty string will be returned.
	*/
	assert(argc > 1);
	if(SIValue_IsNull(argv[0])) return SI_NullVal();
	assert(SI_TYPE(argv[1]) == T_INT64);

	char *original = argv[0].stringval;
	int64_t original_len = strlen(original);
	int64_t start = argv[1].longval;
	int64_t length;

	/* Make sure start doesn't overreach. */
	assert(start < original_len && start >= 0);

	if(argc == 2) {
		length = original_len - start;
	} else {
		assert(SI_TYPE(argv[2]) == T_INT64);
		length = argv[2].longval;
		assert(length >= 0);

		/* Make sure length does not overreach. */
		if(start + length > original_len) {
			length = original_len - start;
		}
	}

	char *substring = rm_malloc((length + 1) * sizeof(char));
	strncpy(substring, original + start, length);
	substring[length] = '\0';

	return SI_TransferStringVal(substring);
}

SIValue AR_TOLOWER(SIValue *argv, int argc) {
	assert(argc == 1);

	if(SIValue_IsNull(argv[0])) return SI_NullVal();
	char *original = argv[0].stringval;
	short lower_len = strlen(original) + 1;
	char *lower = rm_malloc((lower_len + 1) * sizeof(char));
	_toLower(original, lower, &lower_len);
	return SI_TransferStringVal(lower);
}

void _toUpper(const char *str, char *upper, size_t *upper_len) {
	size_t str_len = strlen(str);
	/* Avoid overflow. */
	assert(*upper_len > str_len);

	/* Update upper len*/
	*upper_len = str_len;

	int i = 0;
	for(; i < str_len; i++) upper[i] = toupper(str[i]);
	upper[i] = 0;
}

SIValue AR_TOUPPER(SIValue *argv, int argc) {
	assert(argc == 1);

	if(SIValue_IsNull(argv[0])) return SI_NullVal();
	char *original = argv[0].stringval;
	size_t upper_len = strlen(original) + 1;
	char *upper = rm_malloc((upper_len + 1) * sizeof(char));
	_toUpper(original, upper, &upper_len);
	return SI_TransferStringVal(upper);
}

SIValue AR_TOSTRING(SIValue *argv, int argc) {
	assert(argc == 1);

	if(SIValue_IsNull(argv[0])) return SI_NullVal();
	size_t len = SIValue_StringConcatLen(argv, 1);
	char *str = rm_malloc(len * sizeof(char));
	size_t bytesWritten = 0;
	SIValue_ToString(argv[0], &str, &len, &bytesWritten);
	return SI_TransferStringVal(str);
}

SIValue AR_TRIM(SIValue *argv, int argc) {
	if(SIValue_IsNull(argv[0])) return SI_NullVal();
	SIValue ltrim = AR_LTRIM(argv, argc);
	SIValue trimmed = AR_RTRIM(&ltrim, 1);
	return trimmed;
}

SIValue AR_CONTAINS(SIValue *argv, int argc) {
	assert(argc == 2);

	// No string contains null.
	if(SIValue_IsNull(argv[0]) || SIValue_IsNull(argv[1])) return SI_NullVal();

	// TODO: remove once we have runtime error handling.
	assert(SI_TYPE(argv[0]) == T_STRING && SI_TYPE(argv[1]) == T_STRING);

	const char *hay = argv[0].stringval;
	const char *needle = argv[1].stringval;

	// See if needle is in hay.
	bool found = (strstr(hay, needle) != NULL);
	return SI_BoolVal(found);
}

SIValue AR_STARTSWITH(SIValue *argv, int argc) {
	assert(argc == 2);

	// No string contains null.
	if(SIValue_IsNull(argv[0]) || SIValue_IsNull(argv[1])) return SI_NullVal();

	// TODO: remove once we have runtime error handling.
	assert(SI_TYPE(argv[0]) == T_STRING && SI_TYPE(argv[1]) == T_STRING);

	const char *str = argv[0].stringval;
	const char *sub_string = argv[1].stringval;
	size_t str_len = strlen(str);
	size_t sub_string_len = strlen(sub_string);

	// If sub-string is longer then string return quickly.
	if(sub_string_len > str_len) return SI_BoolVal(false);

	// Compare character by character, see if there's a match.
	for(int i = 0; i < sub_string_len; i++) {
		if(str[i] != sub_string[i]) return SI_BoolVal(false);
	}

	return SI_BoolVal(true);
}

SIValue AR_ENDSWITH(SIValue *argv, int argc) {
	assert(argc == 2);

	// No string contains null.
	if(SIValue_IsNull(argv[0]) || SIValue_IsNull(argv[1])) return SI_NullVal();

	// TODO: remove once we have runtime error handling.
	assert(SI_TYPE(argv[0]) == T_STRING && SI_TYPE(argv[1]) == T_STRING);

	const char *str = argv[0].stringval;
	const char *sub_string = argv[1].stringval;
	size_t str_len = strlen(str);
	size_t sub_string_len = strlen(sub_string);

	// If sub-string is longer then string return quickly.
	if(sub_string_len > str_len) return SI_BoolVal(false);

	// Advance str to the "end"
	str += (str_len - sub_string_len);
	// Compare character by character, see if there's a match.
	for(int i = 0; i < sub_string_len; i++) {
		if(str[i] != sub_string[i]) return SI_BoolVal(false);
	}

	return SI_BoolVal(true);
}

SIValue AR_ID(SIValue *argv, int argc) {
	assert(argc == 1);
	assert(SI_TYPE(argv[0]) & (T_NODE | T_EDGE));
	GraphEntity *graph_entity = (GraphEntity *)argv[0].ptrval;
	return SI_LongVal(ENTITY_GET_ID(graph_entity));
}

SIValue AR_LABELS(SIValue *argv, int argc) {
	assert(argc == 1);
	// TODO Introduce AST validations to prevent non-nodes
	// from reaching here
	if(SI_TYPE(argv[0]) != T_NODE) return SI_NullVal();

	char *label = "";
	Node *node = argv[0].ptrval;
	GraphContext *gc = GraphContext_GetFromTLS();
	Graph *g = gc->g;
	int labelID = Graph_GetNodeLabel(g, ENTITY_GET_ID(node));
	if(labelID != GRAPH_NO_LABEL) label = gc->node_schemas[labelID]->name;
	return SI_ConstStringVal(label);
}

SIValue AR_TYPE(SIValue *argv, int argc) {
	assert(argc == 1);
	// TODO Introduce AST validations to prevent non-edges
	// from reaching here
	if(SI_TYPE(argv[0]) != T_EDGE) return SI_NullVal();

	char *type = "";
	Edge *e = argv[0].ptrval;
	GraphContext *gc = GraphContext_GetFromTLS();
	int id = Graph_GetEdgeRelation(gc->g, e);
	if(id != GRAPH_NO_RELATION) type = gc->relation_schemas[id]->name;
	return SI_ConstStringVal(type);
}

SIValue AR_EXISTS(SIValue *argv, int argc) {
	assert(argc == 1);
	/* MATCH (n) WHERE EXISTS(n.name) RETURN n
	 * In case n.name does not exists
	 * SIValue representing NULL is returned.
	 * if n.name exists its value can not be NULL. */
	if(SIValue_IsNull(argv[0])) return SI_BoolVal(0);
	return SI_BoolVal(1);
}

//==============================================================================
//=== Conditional flow functions ===============================================
//==============================================================================

/* Case When
 * Case Value [When Option i Then Result i] Else Default end */
SIValue AR_CASEWHEN(SIValue *argv, int argc) {
	/* Expecting Value and Default.
	 * Assuming values maintain original specified order. */
	assert(argc > 1);

	int alternatives = argc - 1;
	SIValue d = argv[argc - 1];

	if((argc % 2) == 0) {
		/* Simple form:
		 * argv[0] - Value
		 * argv[i] - Option i
		 * argv[i+1] - Result i
		 * argv[argc-1] - Default
		 *
		 * Evaluate alternatives in order, return first alternatives which
		 * is equals to Value. */
		SIValue v = argv[0];
		for(int i = 1; i < alternatives; i += 2) {
			SIValue a = argv[i];
			if(SIValue_Compare(v, a) == 0) {
				// Return Result i.
				return argv[i + 1];
			}
		}
	} else {
		/* Generic form:
		 * argv[i] - Option i
		 * argv[i+1] - Result i
		 * arg[argc-1] - Default
		 *
		 * Evaluate alternatives in order, return first alternatives which
		 * is not NULL. */
		for(int i = 0; i < alternatives; i += 2) {
			SIValue a = argv[i];
			if(!SIValue_IsNull(a)) {
				// Return Result i.
				return argv[i + 1];
			}
		}
	}

	//Did not match against any Option return default.
	return d;
}

SIValue _AR_NodeDegree(SIValue *argv, int argc, GRAPH_EDGE_DIR dir) {
	assert(argc >= 1 && SI_TYPE(argv[0]) == T_NODE);

	Node *n = (Node *)argv[0].ptrval;
	Edge *edges = array_new(Edge, 0);
	GraphContext *gc = GraphContext_GetFromTLS();

	if(argc > 1) {
		// We're interested in specific relationship type(s).
		for(int i = 1; i < argc; i++) {
			// relationship type should be specified as a string.
			assert(SI_TYPE(argv[i]) == T_STRING);
			const char *label = argv[i].stringval;

			// Make sure relationship exists.
			Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_EDGE);
			if(!s) continue;

			// Accumulate edges.
			Graph_GetNodeEdges(gc->g, n, dir, s->id, &edges);
		}
	} else {
		// Get all relations, regardless of their type.
		Graph_GetNodeEdges(gc->g, n, dir, GRAPH_NO_RELATION, &edges);
	}

	SIValue res = SI_LongVal(array_len(edges));
	array_free(edges);
	return res;
}

SIValue AR_INCOMEDEGREE(SIValue *argv, int argc) {
	return _AR_NodeDegree(argv, argc, GRAPH_EDGE_DIR_INCOMING);
}

SIValue AR_OUTGOINGDEGREE(SIValue *argv, int argc) {
	return _AR_NodeDegree(argv, argc, GRAPH_EDGE_DIR_OUTGOING);
}

SIValue AR_AND(SIValue *argv, int argc) {
	assert(argc > 1);
	bool res = true;

	for(int i = 0; i < argc; i++) {
		SIValue v = argv[i];

		if(SIValue_IsNull(v)) return SI_NullVal();
		assert((SI_TYPE(v) & T_BOOL));

		res &= v.longval;
	}

	return SI_BoolVal(res);
}

SIValue AR_OR(SIValue *argv, int argc) {
	assert(argc > 1);
	bool res = false;

	for(int i = 0; i < argc; i++) {
		SIValue v = argv[i];

		if(SIValue_IsNull(v)) return SI_NullVal();
		assert((SI_TYPE(v) & T_BOOL));

		res |= v.longval;
	}

	return SI_BoolVal(res);
}

SIValue AR_XOR(SIValue *argv, int argc) {
	assert(argc == 2);

	SIValue a = argv[0];
	SIValue b = argv[1];

	if(SIValue_IsNull(a)) return SI_NullVal();
	assert((SI_TYPE(a) & T_BOOL));
	if(SIValue_IsNull(b)) return SI_NullVal();
	assert((SI_TYPE(b) & T_BOOL));

	bool res = a.longval != b.longval;
	return SI_BoolVal(res);
}

SIValue AR_NOT(SIValue *argv, int argc) {
	assert(argc == 1);
	SIValue a = argv[0];
	if(SIValue_IsNull(a)) return SI_NullVal();

	if(SI_TYPE(a) & (SI_NUMERIC | T_BOOL)) return SI_BoolVal(!SI_GET_NUMERIC(a));
	// String, Node, Edge, Ptr all evaluate to true.
	return SI_BoolVal(false);
}

SIValue AR_GT(SIValue *argv, int argc) {
	assert(argc == 2);
	SIValue a = argv[0];
	SIValue b = argv[1];

	if(SIValue_IsNull(a) || SIValue_IsNull(b)) return SI_NullVal();

	assert(SI_TYPE(a) == SI_TYPE(b));

	switch(SI_TYPE(a)) {
	case T_STRING:
		return SI_BoolVal(SIValue_Compare(a, b) > 0);
	case T_INT64:
	case T_DOUBLE:
		return SI_BoolVal(SI_GET_NUMERIC(a) > SI_GET_NUMERIC(b));
	default:
		assert(false);
	}
}

SIValue AR_GE(SIValue *argv, int argc) {
	assert(argc == 2);
	SIValue a = argv[0];
	SIValue b = argv[1];

	if(SIValue_IsNull(a) || SIValue_IsNull(b)) return SI_NullVal();

	assert(SI_TYPE(a) == SI_TYPE(b));
	//Type mismatch: expected Float, Integer, Point, String, Date, Time, LocalTime, LocalDateTime or DateTime
	// but was Node (line 1, column 22 (offset: 21)) "match (n),(m) return n > m" ^

	switch(SI_TYPE(a)) {
	case T_STRING:
		return SI_BoolVal(SIValue_Compare(a, b) >= 0);
	case T_INT64:
	case T_DOUBLE:
		return SI_BoolVal(SI_GET_NUMERIC(a) >= SI_GET_NUMERIC(b));
	default:
		assert(false);
	}
}

SIValue AR_LT(SIValue *argv, int argc) {
	assert(argc == 2);
	SIValue a = argv[0];
	SIValue b = argv[1];

	if(SIValue_IsNull(a) || SIValue_IsNull(b)) return SI_NullVal();

	assert(SI_TYPE(a) == SI_TYPE(b));
	//Type mismatch: expected Float, Integer, Point, String, Date, Time, LocalTime, LocalDateTime or DateTime
	// but was Node (line 1, column 22 (offset: 21)) "match (n),(m) return n > m" ^

	switch(SI_TYPE(a)) {
	case T_STRING:
		return SI_BoolVal(SIValue_Compare(a, b) < 0);
	case T_INT64:
	case T_DOUBLE:
		return SI_BoolVal(SI_GET_NUMERIC(a) < SI_GET_NUMERIC(b));
	default:
		assert(false);
	}
}

SIValue AR_LE(SIValue *argv, int argc) {
	assert(argc == 2);
	SIValue a = argv[0];
	SIValue b = argv[1];

	if(SIValue_IsNull(a) || SIValue_IsNull(b)) return SI_NullVal();

	assert(SI_TYPE(a) == SI_TYPE(b));
	//Type mismatch: expected Float, Integer, Point, String, Date, Time, LocalTime, LocalDateTime or DateTime
	// but was Node (line 1, column 22 (offset: 21)) "match (n),(m) return n > m" ^

	switch(SI_TYPE(a)) {
	case T_STRING:
		return SI_BoolVal(SIValue_Compare(a, b) <= 0);
	case T_INT64:
	case T_DOUBLE:
		return SI_BoolVal(SI_GET_NUMERIC(a) <= SI_GET_NUMERIC(b));
	default:
		assert(false);
	}
}

SIValue AR_EQ(SIValue *argv, int argc) {
	assert(argc == 2);
	SIValue a = argv[0];
	SIValue b = argv[1];

	int res = SIValue_Compare(a, b);

	// if res == DISJOINT => type mismatch

	if(res == COMPARED_NULL) return SI_NullVal();
	return SI_BoolVal(res == 0);
}

SIValue AR_NE(SIValue *argv, int argc) {
	assert(argc == 2);
	SIValue a = argv[0];
	SIValue b = argv[1];

	if(SIValue_IsNull(a) || SIValue_IsNull(b)) return SI_NullVal();

	assert(SI_TYPE(a) == SI_TYPE(b));
	//Type mismatch: expected Float, Integer, Point, String, Date, Time, LocalTime, LocalDateTime or DateTime
	// but was Node (line 1, column 22 (offset: 21)) "match (n),(m) return n > m" ^

	switch(SI_TYPE(a)) {
	case T_STRING:
		return SI_BoolVal(SIValue_Compare(a, b) != 0);
	case T_INT64:
	case T_DOUBLE:
		return SI_BoolVal(SI_GET_NUMERIC(a) != SI_GET_NUMERIC(b));
	default:
		assert(false);
	}
}

//==============================================================================
//=== List functions ===========================================================
//==============================================================================

/* Create a list from a given squence of values.
   "RETURN [1, '2', True, null]" */
SIValue AR_TOLIST(SIValue *argv, int argc) {
	SIValue array = SI_Array(argc);
	for(int i = 0; i < argc; i++) {
		SIArray_Append(&array, argv[i]);
	}
	return array;
}

/* Returns a value in a specific index in an array.
   Valid index range is [-arrayLen, arrayLen).
   Invalid index will return null.
   "RETURN [1, 2, 3][0]" will yeild 1 */
SIValue AR_SUBSCRIPT(SIValue *argv, int argc) {
	assert(argc == 2 && argv[0].type == T_ARRAY && argv[1].type == T_INT64);
	SIValue list = argv[0];
	int32_t index = (int32_t)argv[1].longval;
	uint32_t arrayLen = SIArray_Length(list);
	// given a negativ index, the accses is calculated as arrayLen+index
	uint32_t absIndex = abs(index);
	// index range can be [-arrayLen, arrayLen) (lower bound inclusive, upper exclusive)
	// this is because 0 = arrayLen+(-arrayLen)
	if((index < 0 && absIndex > arrayLen) || (index > 0 && absIndex >= arrayLen)) return SI_NullVal();
	index = index >= 0 ? index : arrayLen - absIndex;
	SIValue res = SIArray_Get(list, index);
	// clone is in case for nested heap allocated values returned from the array
	return SI_CloneValue(res);
}

/* Return a sub array from an array given a range of indices.
   Valid indices ragne is [-arrayLen, arrayLen).
   If range start value is bigger then range end value an empty list will be returnd.
   If indices are still integers but not in the valid range, only values within the valid range
   will be returned.
   If one of the indices is null, null will be returnd.
   "RETURN [1, 2, 3][0..1]" will yield [1, 2] */
SIValue AR_SLICE(SIValue *argv, int argc) {
	assert(argc == 3 && argv[0].type == T_ARRAY);
	if(argv[0].type == T_NULL || argv[1].type == T_NULL || argv[2].type == T_NULL) return SI_NullVal();
	assert(argv[1].type == T_INT64 && argv[2].type == T_INT64);
	SIValue array = argv[0];

	// get array length
	uint32_t arrayLen = SIArray_Length(array);

	// get start and end index
	SIValue start = argv[1];
	int32_t startIndex = (int32_t)start.longval;
	SIValue end = argv[2];
	int32_t endIndex = (int32_t)end.longval;

	// if negative index, calculate offset from end
	if(startIndex < 0) startIndex = arrayLen - abs(startIndex);
	// if offset from the end is out of bound, start at 0
	if(startIndex < 0) startIndex = 0;

	// if negative index, calculate offset from end
	if(endIndex < 0) endIndex = arrayLen - abs(endIndex);
	// if index out of bound, end at arrayLen
	if(((int32_t)arrayLen) < endIndex) endIndex = arrayLen;
	// cant go in reverse
	if(endIndex <= startIndex) {
		return SI_EmptyArray();
	}

	SIValue subArray = SI_Array(endIndex - startIndex);
	for(uint i = startIndex; i < endIndex; i++) {
		SIArray_Append(&subArray, SIArray_Get(array, i));
	}
	return subArray;
}

/* Create a new list of integers in the range of [start, end]. If a step was given
   the step between two consecutive list members will be this step.
   If step was not suppllied, it will be default as 1
   "RETURN range(3,8,2)" will yield [3, 5, 7] */
SIValue AR_RANGE(SIValue *argv, int argc) {
	assert(argc > 1 && argc <= 3 && argv[0].type == T_INT64 && argv[1].type == T_INT64);
	int64_t start = argv[0].longval;
	int64_t end = argv[1].longval;
	int64_t interval = 1;
	if(argc == 3) {
		assert(argv[2].type == T_INT64);
		interval = argv[2].longval;
	}
	SIValue array = SI_Array(1 + (end - start) / interval);
	for(; start <= end; start += interval) {
		SIArray_Append(&array, SI_LongVal(start));
	}
	return array;
}

/* Checks if a value is in a given list.
   "RETURN 3 IN [1, 2, 3]" will return true */
SIValue AR_IN(SIValue *argv, int argc) {
	assert(argc == 2 && argv[1].type == T_ARRAY);
	SIValue lookupValue = argv[0];
	SIValue lookupList = argv[1];
	// indicate if there was a null comparison during the array scan
	bool comparedNull = false;
	uint arrayLen = SIArray_Length(lookupList);
	for(uint i = 0; i < arrayLen; i++) {
		int compareValue = SIValue_Compare(lookupValue, SIArray_Get(lookupList, i));
		if(compareValue == 0) return SI_BoolVal(true);
		if(compareValue == COMPARED_NULL) comparedNull = true;
	}
	// if there was a null comparison return null, other wise return false as the lookup item did not found
	return comparedNull ? SI_NullVal() : SI_BoolVal(false);
}

/* Return a list/string/map/path size.
   "RETURN size([1, 2, 3])" will return 3
   TODO: when map and path are implemented, add their functonality */
SIValue AR_SIZE(SIValue *argv, int argc) {
	assert(argc == 1);
	SIValue value = argv[0];
	switch(value.type) {
	case T_ARRAY:
		return SI_LongVal(SIArray_Length(value));
	case T_STRING:
		return SI_LongVal(strlen(value.stringval));
	default:
		assert(false);
	}
}

/* Return the first member of a list.
   "RETURN head([1, 2, 3])" will return 1 */
SIValue AR_HEAD(SIValue *argv, int argc) {
	assert(argc == 1);
	SIValue value = argv[0];
	if(value.type == T_NULL) return SI_NullVal();
	assert(value.type == T_ARRAY);
	uint arrayLen = SIArray_Length(value);
	if(arrayLen == 0) return SI_NullVal();
	return SIArray_Get(value, 0);
}

/* Return a sublist of a list, which contains all the values withiout the first value.
   "RETURN tail([1, 2, 3])" will return [2, 3] */
SIValue AR_TAIL(SIValue *argv, int argc) {
	assert(argc == 1);
	SIValue value = argv[0];
	if(value.type == T_NULL) return SI_NullVal();
	assert(value.type == T_ARRAY);
	uint arrayLen = SIArray_Length(value);
	SIValue array = SI_Array(arrayLen);
	if(arrayLen < 2) return array;
	for(uint i = 1; i < arrayLen; i++) {
		SIArray_Append(&array, SIArray_Get(value, i));
	}
	return array;
}

//==============================================================================
//=== Temporal functions =======================================================
//==============================================================================

SIValue AR_TIMESTAMP(SIValue *argv, int argc) {
	return SI_LongVal(TemporalValue_NewTimestamp());
}

bool AR_FuncExists(const char *func_name) {
	char lower_func_name[32] = {0};
	short lower_func_name_len = 32;
	_toLower(func_name, &lower_func_name[0], &lower_func_name_len);
	void *f = raxFind(__aeRegisteredFuncs, (unsigned char *)lower_func_name, lower_func_name_len);
	return (f != raxNotFound);
}

void AR_RegisterFuncs() {
	struct RegFunc {
		const char *func_name;
		AR_Func func_ptr;
	};

	struct RegFunc functions[49] = {
		{"add", AR_ADD}, {"sub", AR_SUB}, {"mul", AR_MUL}, {"div", AR_DIV}, {"abs", AR_ABS}, {"ceil", AR_CEIL},
		{"floor", AR_FLOOR}, {"rand", AR_RAND}, {"round", AR_ROUND}, {"sign", AR_SIGN}, {"left", AR_LEFT},
		{"reverse", AR_REVERSE}, {"right", AR_RIGHT}, {"ltrim", AR_LTRIM}, {"rtrim", AR_RTRIM}, {"substring", AR_SUBSTRING},
		{"tolower", AR_TOLOWER}, {"toupper", AR_TOUPPER}, {"tostring", AR_TOSTRING}, {"trim", AR_TRIM}, {"contains", AR_CONTAINS},
		{"starts with", AR_STARTSWITH}, {"ends with", AR_ENDSWITH}, {"id", AR_ID}, {"labels", AR_LABELS}, {"type", AR_TYPE}, {"exists", AR_EXISTS},
		{"timestamp", AR_TIMESTAMP}, {"and", AR_AND}, {"or", AR_OR}, {"xor", AR_XOR}, {"not", AR_NOT}, {"gt", AR_GT}, {"ge", AR_GE},
		{"lt", AR_LT}, {"le", AR_LE}, {"eq", AR_EQ}, {"neq", AR_NE}, {"case", AR_CASEWHEN}, {"indegree", AR_INCOMEDEGREE},
		{"outdegree", AR_OUTGOINGDEGREE},
		{"tolist", AR_TOLIST}, {"subscript", AR_SUBSCRIPT}, {"slice", AR_SLICE}, {"range", AR_RANGE}, {"in", AR_IN},
		{"size", AR_SIZE}, {"head", AR_HEAD}, {"tail", AR_TAIL}
	};

	char lower_func_name[32] = {0};
	short lower_func_name_len = 32;

	for(int i = 0; i < 49; i++) {
		_toLower(functions[i].func_name, &lower_func_name[0], &lower_func_name_len);
		_AR_RegFunc(lower_func_name, lower_func_name_len, functions[i].func_ptr);
		lower_func_name_len = 32;
	}
}
