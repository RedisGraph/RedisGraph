/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./arithmetic_expression.h"

#include "../RG.h"
#include "funcs.h"
#include "rax.h"
#include "../errors.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/strcmp.h"
#include "../graph/graph.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../datatypes/temporal_value.h"
#include "../datatypes/array.h"
#include "../ast/ast_shared.h"

#include <ctype.h>

//------------------------------------------------------------------------------
// Forward declarations
//------------------------------------------------------------------------------

static AR_EXP_Result _AR_EXP_Evaluate(AR_ExpNode *root, const Record r,
		SIValue *result);

// Clear an op node internals, without freeing the node allocation itself.
static void _AR_EXP_FreeOpInternals(AR_ExpNode *op_node);

static AR_ExpNode *_AR_EXP_CloneOperand(AR_ExpNode *exp) {
	AR_ExpNode *clone = rm_calloc(1, sizeof(AR_ExpNode));
	clone->type = AR_EXP_OPERAND;
	switch(exp->operand.type) {
	case AR_EXP_CONSTANT:
		clone->operand.type = AR_EXP_CONSTANT;
		clone->operand.constant = SI_ShallowCloneValue(exp->operand.constant);
		break;
	case AR_EXP_VARIADIC:
		clone->operand.type = exp->operand.type;
		clone->operand.variadic.entity_alias = exp->operand.variadic.entity_alias;
		clone->operand.variadic.entity_alias_idx = exp->operand.variadic.entity_alias_idx;
		break;
	case AR_EXP_PARAM:
		clone->operand.type = AR_EXP_PARAM;
		clone->operand.param_name = exp->operand.param_name;
		break;
	case AR_EXP_BORROW_RECORD:
		clone->operand.type = AR_EXP_BORROW_RECORD;
		break;
	default:
		ASSERT(false);
		break;
	}
	return clone;
}

static AR_ExpNode *_AR_EXP_NewOpNode(const char *func_name, uint child_count) {
	ASSERT(func_name != NULL);

	size_t size = sizeof(AR_ExpNode);         // this node
	size += sizeof(SIValue)    * child_count; // arguments
	size += sizeof(AR_OpChild) * child_count; // child nodes array

	AR_ExpNode *node = rm_calloc(1, size);

	node->type           = AR_EXP_OP;
	node->op.argc        = child_count;
	node->op.func_name   = func_name;
	node->op.child_count = 0;

	return node;
}

static AR_ExpNode *_AR_EXP_CloneOp(AR_ExpNode *exp) {
	AR_ExpNode *clone = _AR_EXP_NewOpNode(exp->op.func_name, exp->op.child_count);
	/* If the function has private data, the function descriptor
	 * itself should be cloned. Otherwise, we can perform a direct assignment. */
	if(exp->op.f->privdata) clone->op.f = AR_CloneFuncDesc(exp->op.f);
	else clone->op.f = exp->op.f;

	for(uint i = 0; i < AR_GET_CHILD_COUNT(exp); i++) {
		AR_ExpNode *orig_child = AR_GET_CHILD_NODE(exp, i);
		clone->op.children[i].child = AR_EXP_Clone(orig_child);
		clone->op.children[i].arg_idx = AR_GET_CHILD_ARG_IDX(exp, i);
	}

	return clone;
}

AR_ExpNode *AR_EXP_NewOpNode(const char *func_name, uint child_count) {
	ASSERT(func_name != NULL);

	// retrieve function
	AR_FuncDesc *func = AR_GetFunc(func_name);
	ASSERT(func != NULL);

	bool aggregation = func->aggregate;
	if(aggregation) child_count++; // accomadate aggregation context

	AR_ExpNode *node = _AR_EXP_NewOpNode(func_name, child_count);
	node->op.f = func;

	if(aggregation) {
		AggregateCtx *ctx = rm_malloc(sizeof(AggregateCtx));
		ctx->hashSet = NULL;
		ctx->private_ctx = NULL;
		ctx->result = SI_NullVal();
		// Add the context to the function descriptor as the function's private data.
		node->op.f = AR_SetPrivateData(node->op.f, ctx);
	}

	return node;
}

void AR_EXP_AddChild(AR_ExpNode *parent, AR_ExpNode *child) {
	ASSERT(parent != NULL);
	ASSERT(child != NULL);
	ASSERT(AR_EXP_IsOperation(parent));

	uint idx = parent->op.child_count;
	ASSERT(AR_GET_CHILD_NODE(parent, idx) == NULL);

	parent->op.children[idx].child = child;
	parent->op.children[idx].arg_idx = idx;
	parent->op.child_count++;
}

static inline AR_ExpNode *_AR_EXP_InitializeOperand(AR_OperandNodeType type) {
	AR_ExpNode *node = rm_calloc(1, sizeof(AR_ExpNode));
	node->type = AR_EXP_OPERAND;
	node->operand.type = type;
	return node;
}

AR_ExpNode *AR_EXP_NewVariableOperandNode(const char *alias) {
	AR_ExpNode *node = _AR_EXP_InitializeOperand(AR_EXP_VARIADIC);
	node->operand.variadic.entity_alias = alias;
	node->operand.variadic.entity_alias_idx = IDENTIFIER_NOT_FOUND;

	return node;
}

AR_ExpNode *AR_EXP_NewAttributeAccessNode(AR_ExpNode *entity,
										  const char *attr) {

	ASSERT(attr != NULL);
	ASSERT(entity != NULL);

	// use property index when possible, prop_idx is set to ATTRIBUTE_NOTFOUND
	// if the graph is not aware of it in which case we'll try to resolve
	// the property using its string representation

	GraphContext *gc = QueryCtx_GetGraphCtx();
	SIValue prop_idx = SI_LongVal(ATTRIBUTE_NOTFOUND);
	SIValue prop_name = SI_ConstStringVal((char *)attr);
	Attribute_ID idx = GraphContext_GetAttributeID(gc, attr);

	if(idx != ATTRIBUTE_NOTFOUND) prop_idx = SI_LongVal(idx);

	// entity is an expression which should be evaluated to a graph entity
	// attr is the name of the attribute we want to extract from entity
	AR_ExpNode *root = AR_EXP_NewOpNode("property", 3);
	AR_EXP_AddChild(root, entity);
	AR_EXP_AddChild(root, AR_EXP_NewConstOperandNode(prop_name));
	AR_EXP_AddChild(root, AR_EXP_NewConstOperandNode(prop_idx));

	return root;
}

AR_ExpNode *AR_EXP_NewConstOperandNode(SIValue constant) {
	AR_ExpNode *node = _AR_EXP_InitializeOperand(AR_EXP_CONSTANT);
	node->operand.constant = constant;
	return node;
}

AR_ExpNode *AR_EXP_NewParameterOperandNode(const char *param_name) {
	AR_ExpNode *node = _AR_EXP_InitializeOperand(AR_EXP_PARAM);
	node->operand.param_name = param_name;
	return node;
}

AR_ExpNode *AR_EXP_NewRecordNode() {
	return _AR_EXP_InitializeOperand(AR_EXP_BORROW_RECORD);
}

/* Pull up and incorporate scalar nodes into OP's arguments array 
 * this will save extra function calls retriving scalars into the OP 
 * 
 * Consider the expression A + 5
 *     +
 *   /   \
 *  A     5 
 *
 * this optimization will pull the right argument 5 into + args array 
 * such that numerous calls to evaluating this expression wouldn't have to 
 * repeatedly evaluate the constant scalar expression 5. */
void _AR_EXP_IncorporateScalars(AR_ExpNode *root) {
	// return if root isn't an operation node
	if(!AR_EXP_IsOperation(root)) return;

	for(int i = 0; i < AR_GET_CHILD_COUNT(root); i++) {
		AR_ExpNode *child = AR_GET_CHILD_NODE(root, i);
		if(AR_EXP_IsOperation(child)) {
		    // call recursively
			_AR_EXP_IncorporateScalars(child);
		} else if (AR_EXP_IsConstant(child)) {
			// incorporate constant into op's arguments array
			int arg_idx = AR_GET_CHILD_ARG_IDX(root, i);
			root->op.args[arg_idx] = child->operand.constant;

			// free child, shift left remaining children
			rm_free(child);
			for(int j = i; j < AR_GET_CHILD_COUNT(root) - 1; j++) {
				root->op.children[j] = root->op.children[j+1];
			}
			root->op.child_count--;
			i--; // revisit current index
		}
	}
}

/* Compact tree by evaluating constant expressions
 * e.g. MINUS(X) where X is a constant number will be reduced to
 * a single node with the value -X
 * PLUS(MINUS(A), B) will be reduced to a single constant: B-A. */
bool _AR_EXP_ReduceToScalar(AR_ExpNode *root, bool reduce_params, SIValue *val) {
	if(val != NULL) *val = SI_NullVal();
	if(root->type == AR_EXP_OPERAND) {
		// In runtime, parameters are set so they can be evaluated
		if(reduce_params && AR_EXP_IsParameter(root)) {
			SIValue v = AR_EXP_Evaluate(root, NULL);
			if(val != NULL) *val = v;
			return true;
		}
		if(AR_EXP_IsConstant(root)) {
			// Root is already a constant
			if(val != NULL) *val = root->operand.constant;
			return true;
		}
		// Root is variadic, no way to reduce.
		return false;
	} else {
		// root represents an operation.
		ASSERT(AR_EXP_IsOperation(root));

		/* See if we're able to reduce each child of root
		 * if so we'll be able to reduce root. */
		bool reduce_children = true;
		for(int i = 0; i < AR_GET_CHILD_COUNT(root); i++) {
			if(!_AR_EXP_ReduceToScalar(AR_GET_CHILD_NODE(root,i), reduce_params, NULL)) {
				// Root reduce is not possible,
				// nevertheless continue reducing children.
				reduce_children = false;
			}
		}

		// Can't reduce root as one of its children is not a constant.
		if(!reduce_children) return false;

		// All child nodes are constants, make sure function is marked as reducible.
		AR_FuncDesc *func_desc = AR_GetFunc(root->op.func_name);
		ASSERT(func_desc != NULL);
		if(!func_desc->reducible) return false;

		// Evaluate function.
		SIValue v = AR_EXP_Evaluate(root, NULL);
		if(val != NULL) *val = v;
		if(SIValue_IsNull(v)) return false;

		// Reduce.
		// Clear children and function context.
		_AR_EXP_FreeOpInternals(root);
		// In-place update, set as constant.
		root->type = AR_EXP_OPERAND;
		root->operand.type = AR_EXP_CONSTANT;
		root->operand.constant = v;
		return true;
		// Root is an aggregation function, can't reduce.
		return false;
	}
}
bool AR_EXP_ReduceToScalar(AR_ExpNode *root, bool reduce_params, SIValue *val) {
	bool res = _AR_EXP_ReduceToScalar(root, reduce_params, val);
	_AR_EXP_IncorporateScalars(root);
	return res;
}

static bool _AR_EXP_ValidateInvocation(AR_FuncDesc *fdesc, SIValue *argv, uint argc) {
	SIType actual_type;
	SIType expected_type = T_NULL;

	// If the function accepts private data, reduce all user-facing counts by 1.
	int offset = (fdesc->privdata != NULL);

	// Make sure number of arguments is as expected.
	if(fdesc->min_argc > argc) {
		// Set the query-level error.
		ErrorCtx_SetError("Received %d arguments to function '%s', expected at least %d", argc - offset,
						  fdesc->name, fdesc->min_argc - offset);
		return false;
	}

	if(fdesc->max_argc < argc) {
		// Set the query-level error.
		ErrorCtx_SetError("Received %d arguments to function '%s', expected at most %d", argc - offset,
						  fdesc->name, fdesc->max_argc - offset);
		return false;
	}

	uint expected_types_count = array_len(fdesc->types);
	for(int i = 0; i < argc; i++) {
		actual_type = SI_TYPE(argv[i]);
		/* For a function that accepts a variable number of arguments.
		* the last specified type in fdesc->types is repeatable. */
		if(i < expected_types_count) {
			expected_type = fdesc->types[i];
		}
		if(!(actual_type & expected_type)) {
			/* TODO extend string-building logic to better express multiple acceptable types, like:
			 * RETURN 'a' * 2
			 * "Type mismatch: expected Float, Integer or Duration but was String" */
			Error_SITypeMismatch(argv[i], expected_type);
			return false;
		}
	}

	return true;
}

/* Evaluating an expression tree constructs an array of SIValues.
 * Free all of these values, in case an intermediate node on the tree caused a heap allocation.
 * For example, in the expression:
 * a.first_name + toUpper(a.last_name)
 * the result of toUpper() is allocated within this tree, and will leak if not freed here. */
static inline void _AR_EXP_FreeResultsArray(SIValue *results, int count) {
	for(int i = 0; i < count; i ++) {
		SIValue_Free(results[i]);
	}
}

static AR_EXP_Result _AR_EXP_EvaluateFunctionCall(AR_ExpNode *node,
		const Record r, SIValue *result) {
	AR_EXP_Result res = EVAL_OK;

	int child_count = AR_GET_CHILD_COUNT(node);
	// Functions with private data will have it appended as an additional child.
	bool include_privdata = (node->op.f->privdata != NULL);
	if(include_privdata) child_count ++;

	bool param_found = false;
	for(int child_idx = 0; child_idx < AR_GET_CHILD_COUNT(node); child_idx++) {
		uint arg_idx = AR_GET_CHILD_ARG_IDX(node,child_idx);
		AR_ExpNode *child = AR_GET_CHILD_NODE(node, child_idx);

		// short stop, all remaining arguments been evaluated during compaction
		if(child == NULL) break;

		// evaluate child
		res = _AR_EXP_Evaluate(child, r, node->op.args + arg_idx);
		if(res == EVAL_ERR) return res;

		// mark if we've encountered a parameter for future replacement
		param_found |= (res == EVAL_FOUND_PARAM);
	}

	if(param_found) res = EVAL_FOUND_PARAM;

	// Add the function's private data, if any.
	// TODO: assign only once
	if(include_privdata) node->op.args[child_count - 1] = SI_PtrVal(node->op.f->privdata);
	//if(include_privdata) sub_trees[child_count - 1] = SI_PtrVal(node->op.f->privdata);

	// Validate before evaluation.
	if(!_AR_EXP_ValidateInvocation(node->op.f, node->op.args, child_count)) {
	//if(!_AR_EXP_ValidateInvocation(node->op.f, sub_trees, child_count)) {
		// The expression tree failed its validations and set an error message.
		res = EVAL_ERR;
		goto cleanup;
	}

	// Evaluate self.
	SIValue v = node->op.f->func(node->op.args, child_count);
	if(SIValue_IsNull(v) && ErrorCtx_EncounteredError()) {
		/* An error was encountered while evaluating this function,
		 * and has already been set in the QueryCtx.
		 * Exit with an error. */
		res = EVAL_ERR;
	}
	if(result) *result = v;

cleanup:
	for(int i = 0; i < AR_GET_CHILD_COUNT(node); i++) {
		uint arg_idx = AR_GET_CHILD_ARG_IDX(node, i);
		SIValue_Free(node->op.args[arg_idx]);
	}

	return res;
}

static bool _AR_EXP_UpdateEntityIdx(AR_OperandNode *node, const Record r) {
	if(!r) {
		// Set the query-level error.
		ErrorCtx_SetError("_AR_EXP_UpdateEntityIdx: No record was given to locate a value with alias %s",
						  node->variadic.entity_alias);
		return false;
	}
	int entry_alias_idx = Record_GetEntryIdx(r, node->variadic.entity_alias);
	if(entry_alias_idx == INVALID_INDEX) {
		// Set the query-level error.
		ErrorCtx_SetError("_AR_EXP_UpdateEntityIdx: Unable to locate a value with alias %s within the record",
						  node->variadic.entity_alias);
		return false;
	} else {
		node->variadic.entity_alias_idx = entry_alias_idx;
		return true;
	}
}

static AR_EXP_Result _AR_EXP_EvaluateVariadic(AR_ExpNode *node, const Record r, SIValue *result) {
	// Make sure entity record index is known.
	if(node->operand.variadic.entity_alias_idx == IDENTIFIER_NOT_FOUND) {
		if(!_AR_EXP_UpdateEntityIdx(&node->operand, r)) return EVAL_ERR;
	}

	int aliasIdx = node->operand.variadic.entity_alias_idx;

	// the value was not created here; share with the caller
	*result = SI_ShareValue(Record_Get(r, aliasIdx));

	return EVAL_OK;
}

static AR_EXP_Result _AR_EXP_EvaluateParam(AR_ExpNode *node, SIValue *result) {
	rax *params = QueryCtx_GetParams();
	AR_ExpNode *param_node = raxFind(params, (unsigned char *)node->operand.param_name,
									 strlen(node->operand.param_name));
	if(param_node == raxNotFound) {
		// Set the query-level error.
		ErrorCtx_SetError("Missing parameters");
		return EVAL_ERR;
	}
	// In place replacement;
	node->operand.type = AR_EXP_CONSTANT;
	node->operand.constant = SI_ShareValue(param_node->operand.constant);
	*result = node->operand.constant;
	return EVAL_FOUND_PARAM;
}

static inline AR_EXP_Result _AR_EXP_EvaluateBorrowRecord(AR_ExpNode *node, const Record r,
														 SIValue *result) {
	// Wrap the current Record in an SI pointer.
	*result = SI_PtrVal(r);
	return EVAL_OK;
}

/* Evaluate an expression tree,
 * placing the calculated value in 'result'
 * and returning whether an error occurred during evaluation. */
static AR_EXP_Result _AR_EXP_Evaluate(AR_ExpNode *root, const Record r,
		SIValue *result) {
	AR_EXP_Result res = EVAL_OK;
	switch(root->type) {
	case AR_EXP_OP:
		return _AR_EXP_EvaluateFunctionCall(root, r, result);
	case AR_EXP_OPERAND:
		switch(root->operand.type) {
		case AR_EXP_CONSTANT:
			// The value is constant or has been computed elsewhere, and is shared with the caller.
			*result = SI_ShareValue(root->operand.constant);
			return res;
		case AR_EXP_VARIADIC:
			return _AR_EXP_EvaluateVariadic(root, r, result);
		case AR_EXP_PARAM:
			return _AR_EXP_EvaluateParam(root, result);
		case AR_EXP_BORROW_RECORD:
			return _AR_EXP_EvaluateBorrowRecord(root, r, result);
		default:
			ASSERT(false && "Invalid expression type");
		}
	default:
		ASSERT(false && "Unknown expression type");
	}

	return res;
}

SIValue AR_EXP_Evaluate(AR_ExpNode *root, const Record r) {
	SIValue result;
	AR_EXP_Result res = _AR_EXP_Evaluate(root, r, &result);

	if(res == EVAL_ERR) {
		ErrorCtx_RaiseRuntimeException(NULL);  // Raise an exception if we're in a run-time context.
		return SI_NullVal(); // Otherwise return NULL; the query-level error will be emitted after cleanup.
	}

	// At least one param node was encountered during evaluation,
	// tree should be parameters free, try reducing the tree.
	if(res == EVAL_FOUND_PARAM) AR_EXP_ReduceToScalar(root, true, NULL);
	return result;
}

void AR_EXP_SetDistinct(AR_ExpNode *root) {
	ASSERT(root != NULL);
	ASSERT(AR_AGGREGATION_NODE(root));

    AggregateCtx *ctx = (AggregateCtx*)root->op.f->privdata;
	ctx->hashSet = Set_New();
}

void AR_EXP_Aggregate(AR_ExpNode *root, const Record r) {
	if(AR_EXP_IsOperation(root)) {
		if(root->op.f->aggregate == true) {
			AR_EXP_Result res = _AR_EXP_EvaluateFunctionCall(root, r, NULL);
			if(res == EVAL_ERR) {
				ErrorCtx_RaiseRuntimeException(NULL);  // Raise an exception if we're in a run-time context.
				return;
			}
		} else {
			// keep searching for aggregation nodes
			for(int i = 0; i < AR_GET_CHILD_COUNT(root); i++) {
				AR_ExpNode *child = AR_GET_CHILD_NODE(root, i);
				AR_EXP_Aggregate(child, r);
			}
		}
	}
}

void _AR_EXP_Finalize(AR_ExpNode *root) {
	//--------------------------------------------------------------------------
	// finalize aggregation node
	//--------------------------------------------------------------------------

	if(AR_AGGREGATION_NODE(root)) {
		AR_Finalize(root->op.f);
		SIValue v = Aggregate_GetResult(root->op.f->privdata);

		// free node internals
		_AR_EXP_FreeOpInternals(root);

		// replace root with constant node
		root->type             = AR_EXP_OPERAND;
		root->operand.type     = AR_EXP_CONSTANT;
		root->operand.constant = v;

		// return, aggregation nodes cannot contain nested aggregation nodes
		return;
	}

	//--------------------------------------------------------------------------
	// recursively traverse child nodes
	//--------------------------------------------------------------------------

	if(AR_EXP_IsOperation(root)) {
		for(int i = 0; i < AR_GET_CHILD_COUNT(root); i++) {
			AR_ExpNode *child = AR_GET_CHILD_NODE(root, i);
			_AR_EXP_Finalize(child);
		}
	}
}

SIValue AR_EXP_Finalize(AR_ExpNode *root, const Record r) {
	ASSERT(root != NULL);

	_AR_EXP_Finalize(root);
	return AR_EXP_Evaluate(root, r);
}

void AR_EXP_CollectEntities(AR_ExpNode *root, rax *aliases) {
	if(AR_EXP_IsOperation(root)) {
		for(int i = 0; i < AR_GET_CHILD_COUNT(root); i ++) {
			AR_EXP_CollectEntities(AR_GET_CHILD_NODE(root, i), aliases);
		}
	} else { // type == AR_EXP_OPERAND
		if(root->operand.type == AR_EXP_VARIADIC) {
			const char *entity = root->operand.variadic.entity_alias;
			raxInsert(aliases, (unsigned char *)entity, strlen(entity), NULL, NULL);
		}
	}
}

void AR_EXP_CollectAttributes(AR_ExpNode *root, rax *attributes) {
	if(AR_EXP_IsOperation(root)) {
		if(RG_STRCMP(root->op.func_name, "property") == 0) {
			AR_ExpNode *arg = AR_GET_CHILD_NODE(root, 1);
			ASSERT(AR_EXP_IsConstant(arg));
			ASSERT(SI_TYPE(arg->operand.constant) == T_STRING);

			const char *attr = arg->operand.constant.stringval;
			raxInsert(attributes, (unsigned char *)attr, strlen(attr), NULL, NULL);
		}

		// continue scanning expression
		for(int i = 0; i < AR_GET_CHILD_COUNT(root); i ++) {
			AR_EXP_CollectAttributes(AR_GET_CHILD_NODE(root, i), attributes);
		}
	}
}

bool AR_EXP_ContainsAggregation(AR_ExpNode *root) {
	if(AR_AGGREGATION_NODE(root)) return true;

	if(AR_EXP_IsOperation(root)) {
		for(int i = 0; i < AR_GET_CHILD_COUNT(root); i++) {
			AR_ExpNode *child = AR_GET_CHILD_NODE(root, i);
			if(AR_EXP_ContainsAggregation(child)) return true;
		}
	}

	return false;
}

bool AR_EXP_ContainsFunc(const AR_ExpNode *root, const char *func) {
	if(root == NULL) return false;
	if(AR_EXP_IsOperation(root)) {
		if(strcasecmp(root->op.func_name, func) == 0) return true;
		for(int i = 0; i < AR_GET_CHILD_COUNT(root); i++) {
			if(AR_EXP_ContainsFunc(AR_GET_CHILD_NODE(root, i), func)) return true;
		}
	}
	return false;
}

bool inline AR_EXP_IsConstant(const AR_ExpNode *exp) {
	return exp->type == AR_EXP_OPERAND && exp->operand.type == AR_EXP_CONSTANT;
}

bool inline AR_EXP_IsParameter(const AR_ExpNode *exp) {
	return exp->type == AR_EXP_OPERAND && exp->operand.type == AR_EXP_PARAM;
}

bool inline AR_EXP_IsOperation(const AR_ExpNode *exp) {
	return exp->type == AR_EXP_OP;
}

bool AR_EXP_IsAttribute(const AR_ExpNode *exp, char **attr) {
	ASSERT(exp != NULL);

	// an arithmetic expression performs attribute extraction
	// if it applys the "property" function, in which case the left-handside
	// child represents the graph entity from which we access the attribute
	// while the right-handside represents the attribute name

	if(exp->type != AR_EXP_OP) return false;
	if(RG_STRCMP(exp->op.func_name, "property") != 0) return false;

	if(attr != NULL) {
		AR_ExpNode *r = AR_GET_CHILD_NODE(exp, 1);
		ASSERT(AR_EXP_IsConstant(r));
		SIValue v = r->operand.constant;
		ASSERT(SI_TYPE(v) == T_STRING);
		*attr = v.stringval;
	}

	return true;
}

bool AR_EXP_ReturnsBoolean(const AR_ExpNode *exp) {
	ASSERT(exp != NULL && exp->type != AR_EXP_UNKNOWN);

	// If the node does not represent a constant, assume it returns a boolean.
	// TODO We can add greater introspection in the future if required.
	if(AR_EXP_IsOperation(exp)) return true;

	// Operand node, return true if it is a boolean or NULL constant.
	if(exp->operand.type == AR_EXP_CONSTANT) {
		return (SI_TYPE(exp->operand.constant) & (T_BOOL | T_NULL));
	}

	// Node is a variable or parameter, whether it evaluates to boolean cannot be determined now.
	return true;
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
	if(AR_EXP_IsOperation(root)) {
		/* Binary operation? */
		char binary_op = 0;

		if(strcmp(root->op.func_name, "ADD") == 0) binary_op = '+';
		else if(strcmp(root->op.func_name, "SUB") == 0) binary_op = '-';
		else if(strcmp(root->op.func_name, "MUL") == 0) binary_op = '*';
		else if(strcmp(root->op.func_name, "DIV") == 0)  binary_op = '/';

		if(binary_op) {
			_AR_EXP_ToString(AR_GET_CHILD_NODE(root, 0), str, str_size, bytes_written);

			/* Make sure there are at least 64 bytes in str. */
			if((*str_size - strlen(*str)) < 64) {
				*str_size += 128;
				*str = rm_realloc(*str, sizeof(char) * *str_size);
			}

			*bytes_written += sprintf((*str + *bytes_written), " %c ", binary_op);

			_AR_EXP_ToString(AR_GET_CHILD_NODE(root, 1), str, str_size, bytes_written);
		} else {
			/* Operation isn't necessarily a binary operation, use function call representation. */
			*bytes_written += sprintf((*str + *bytes_written), "%s(", root->op.func_name);

			for(int i = 0; i < AR_GET_CHILD_COUNT(root); i++) {
				_AR_EXP_ToString(AR_GET_CHILD_NODE(root, i), str, str_size, bytes_written);

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
			*bytes_written += sprintf((*str + *bytes_written), "%s", root->operand.variadic.entity_alias);
		}
	}
}

void AR_EXP_ToString(const AR_ExpNode *root, char **str) {
	size_t str_size = 0;
	size_t bytes_written = 0;
	*str = NULL;
	_AR_EXP_ToString(root, str, &str_size, &bytes_written);
}

// Generate a heap-allocated name for an arithmetic expression.
// This routine is only used to name ORDER BY expressions.
char *AR_EXP_BuildResolvedName(AR_ExpNode *root) {
	char *name = NULL;
	AR_EXP_ToString(root, &name);
	return name;
}

AR_ExpNode *AR_EXP_Clone(AR_ExpNode *exp) {
	if(exp == NULL) return NULL;

	AR_ExpNode *clone = NULL;

	switch(exp->type) {
	case AR_EXP_OPERAND:
		clone = _AR_EXP_CloneOperand(exp);
		break;
	case AR_EXP_OP:
		clone = _AR_EXP_CloneOp(exp);
		break;
	default:
		ASSERT(false);
		break;
	}

	clone->resolved_name = exp->resolved_name;

	return clone;
}

static inline void _AR_EXP_FreeOpInternals(AR_ExpNode *op_node) {
	if(op_node->op.f->bfree) {
		op_node->op.f->bfree(op_node->op.f->privdata); // Free the function's private data.
		rm_free(op_node->op.f); // The function descriptor itself is an allocation in this case.
	}
	for(int i = 0; i < AR_GET_CHILD_COUNT(op_node); i++) {
		AR_EXP_Free(AR_GET_CHILD_NODE(op_node, i));
	}
}

inline void AR_EXP_Free(AR_ExpNode *root) {
	if(AR_EXP_IsOperation(root)) {
		_AR_EXP_FreeOpInternals(root);
	} else if(AR_EXP_IsConstant(root)) {
		SIValue_Free(root->operand.constant);
	}
	rm_free(root);
}

