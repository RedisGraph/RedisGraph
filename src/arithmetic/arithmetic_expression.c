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
#include "../query_ctx.h"
#include "../graph/graph.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../datatypes/temporal_value.h"

#include <ctype.h>
#include <assert.h>

/* Update arithmetic expression variable node by setting node's property index.
 * when constructing an arithmetic expression we'll delay setting graph entity
 * attribute index to the first execution of the expression, this is due to
 * entity aliasing where we lose track over which entity is aliased, consider
 * MATCH (n:User) WITH n AS x RETURN x.name
 * When constructing the arithmetic expression x.name, we don't know
 * who X is referring to. */
static void _AR_EXP_UpdatePropIdx(AR_ExpNode *root, const Record r) {
	GraphContext *gc = QueryCtx_GetGraphCtx();
	root->operand.variadic.entity_prop_idx = GraphContext_GetAttributeID(gc,
																		 root->operand.variadic.entity_prop);
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

AR_ExpNode *AR_EXP_NewOpNode(const char *func_name, uint child_count) {
	assert(func_name);

	AR_ExpNode *node = rm_calloc(1, sizeof(AR_ExpNode));
	node->type = AR_EXP_OP;
	node->op.func_name = func_name;
	node->op.child_count = child_count;
	node->op.children = rm_malloc(child_count * sizeof(AR_ExpNode *));

	/* Determine function type. */
	AR_FuncDesc *func = AR_GetFunc(func_name);
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

static bool _AR_EXP_ValidateInvocation(AR_FuncDesc *fdesc, SIValue *argv, uint argc, char **error) {
	SIType actual_type;
	SIType expected_type;

	// Make sure number of arguments is as expected.
	if(fdesc->argc != VAR_ARG_LEN) {
		if(fdesc->argc != argc) {
			asprintf(error, "Too many parameters for function '%s'", fdesc->name);
			return false;
		}
		// Make sure each argument is of the expected type.
		for(int i = 0; i < argc; i++) {
			actual_type = SI_TYPE(argv[i]);
			expected_type = fdesc->types[i];

			if(!(SI_TYPE(argv[i]) & fdesc->types[i])) {
				const char *actual_type_str = SIType_ToString(actual_type);
				const char *expected_type_str = SIType_ToString(expected_type);
				asprintf(error, "Type mismatch: expected %s but was %s ", expected_type_str, actual_type_str);
				return false;
			}
		}
	} else {
		/* Function accepts a variable number of arguments.
		 * the last specified type in fdesc->types is repeatable. */
		uint expected_types_count = array_len(fdesc->types);
		for(int i = 0; i < argc; i++) {
			actual_type = SI_TYPE(argv[i]);
			if(i < expected_types_count) {
				expected_type = fdesc->types[i];
			}
			if(!(actual_type & expected_type)) {
				const char *actual_type_str = SIType_ToString(actual_type);
				const char *expected_type_str = SIType_ToString(expected_type);
				asprintf(error, "Type mismatch: expected %s but was %s ", expected_type_str, actual_type_str);
				return false;
			}
		}
	}

	return true;
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
				SIValue v = AR_EXP_Evaluate(root->op.children[child_idx], r);
				if(SIValue_IsError(v)) return v;
				sub_trees[child_idx] = v;
			}

			/* Validate before evaluation. */
			char *error;
			if(!_AR_EXP_ValidateInvocation(root->op.f, sub_trees, root->op.child_count, &error)) {
				// Clean up.
				for(int child_idx = 0; child_idx < root->op.child_count; child_idx++) {
					SIValue_Free(sub_trees + child_idx);
				}
				pthread_exit(error);
			}
			/* Evaluate self. */
			result = root->op.f->func(sub_trees, root->op.child_count);
			/* Free any SIValues that were allocated while evaluating this tree. */
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
				if(!(t && (REC_TYPE_NODE | REC_TYPE_EDGE))) {
					char *error;
					SIValue v = Record_GetScalar(r, root->operand.variadic.entity_alias_idx);
					asprintf(&error, "Type mismatch: expected a map but was %s", SIType_ToString(SI_TYPE(v)));
					pthread_exit(error);
				}

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
			size_t len = SIValue_ToString(root->operand.constant, (*str + *bytes_written), 64);
			*bytes_written += len;
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
