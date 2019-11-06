/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_merge.h"
#include "../../query_ctx.h"
#include "../../schema/schema.h"
#include "../../arithmetic/arithmetic_expression.h"
#include <assert.h>

/* Forward declarations. */
static OpResult MergeInit(OpBase *opBase);
static Record MergeConsume(OpBase *opBase);
static void MergeFree(OpBase *opBase);

static void _AddProperties(OpMerge *op, Record r, GraphEntity *ge, PropertyMap *props) {
	for(int i = 0; i < props->property_count; i++) {
		SIValue val = AR_EXP_Evaluate(props->values[i], r);
		GraphEntity_AddProperty(ge, props->keys[i], val);
		SIValue_Free(&val);
	}

	if(op->stats) op->stats->properties_set += props->property_count;
}


static inline Record _pullFromStream(OpBase *branch) {
	return OpBase_Consume(branch);
}

static Record _pullFromBoundVariableStream(OpMerge *op) {
	return _pullFromStream(op->bound_variable_stream);
}

static Record _pullFromMatchStream(OpMerge *op, Record lhs_record) {
	// OpBase_PropagateReset(rhs);
	return _pullFromStream(op->match_stream);
}

static Record _createPattern(OpMerge *op, Record lhs_record) {
	return _pullFromStream(op->create_stream);
}

OpBase *NewMergeOp(const ExecutionPlan *plan, ResultSetStatistics *stats) {
	/* Merge is an Apply operator with three children, with the first potentially being NULL.
	 * They will be created outside of here, as with other Apply operators (see CartesianProduct
	 * and ValueHashJoin). */
	OpMerge *op = malloc(sizeof(OpMerge));
	op->stats = stats;
	op->records = NULL;
	op->match_argument_tap = NULL;
	op->create_argument_tap = NULL;
	// op->should_create_pattern = true;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_MERGE, "Merge", MergeInit, MergeConsume, NULL, NULL, MergeFree,
				plan);

	return (OpBase *)op;
}

static Record _handoff(OpMerge *op) {
	// TODO is all this function's work necessary?
	Record r = NULL;
	if(array_len(op->records)) r = array_pop(op->records);
	return r;
}

static Record MergeConsume(OpBase *opBase) {
	OpMerge *op = (OpMerge *)opBase;

	// Return mode, all data was consumed.
	if(op->records) return _handoff(op);

	// Consume mode.
	op->records = array_new(Record, 32);

	Record lhs_record = NULL;
	bool reading_bound_variables = true;
	bool should_create_pattern = true;
	while(reading_bound_variables) {
		Record rhs_record = NULL;
		// Try to get a record from the bound variable stream..
		if(op->bound_variable_stream) {
			lhs_record = _pullFromBoundVariableStream(op);
			if(lhs_record == NULL) return _handoff(op); // Depleted, switch to return mode.

			// Propagate record to the top of the Match and Create streams.
			// TODO if statements might be unnecessary
			if(op->match_argument_tap) ArgumentSetRecord(op->match_argument_tap, Record_Clone(lhs_record));
			if(op->create_argument_tap) ArgumentSetRecord(op->create_argument_tap, Record_Clone(lhs_record));
		} else {
			reading_bound_variables = false;
		}

		bool reading_matches = true;
		while(reading_matches) {
			// Retrieve Records from the Match stream until its depleted.
			rhs_record = _pullFromMatchStream(op, lhs_record);
			if(rhs_record == NULL) {
				reading_matches = false;
				break;
			}
			// Pattern was successfully matched.
			should_create_pattern = false;
			op->records = array_append(op->records, rhs_record);
		}

		if(should_create_pattern) {
			Record created_record = _createPattern(op, lhs_record); // TODO looks unsafe
			op->records = array_append(op->records, created_record);
		}
	}

	return _handoff(op);
}

static OpResult MergeInit(OpBase *opBase) {
	OpMerge *op = (OpMerge *)opBase;

	// If Merge has 2 children, there are no bound variables and thus no Arguments in the child streams.
	if(opBase->childCount == 2) {
		op->bound_variable_stream = NULL;
		op->match_stream = opBase->children[0];
		op->create_stream = opBase->children[1];
		return OP_OK;
	}

	// Otherwise, the first stream resolves bound variables.
	op->bound_variable_stream = opBase->children[0];
	op->match_stream = opBase->children[1];
	op->create_stream = opBase->children[2];

	// Find and store references to the Argument taps for the Match and Create streams.
	// The Match stream is populated by an Argument tap, store a reference to it.
	op->match_argument_tap = (Argument *)ExecutionPlan_LocateOp(op->match_stream, OPType_ARGUMENT);

	// If the create stream is populated by an Argument tap, store a reference to it.
	OpBase *create_stream = op->op.children[2];
	op->create_argument_tap = (Argument *)ExecutionPlan_LocateOp(op->create_stream, OPType_ARGUMENT);

	return OP_OK;
}

// Modification of ExecutionPlan_LocateOp that only follows LHS child.
// Otherwise, the assumptions of Merge_SetStreams fail in MERGE..MERGE queries.
// Match and Create streams are always guaranteed to not branch (have any ops with multiple children).
static OpBase *_LocateOp(OpBase *root, OPType type) {
	if(!root) return NULL;

	if(root->type & type) return root;

	if(root->childCount > 0) return _LocateOp(root->children[0], type);

	return NULL;
}

void Merge_SetStreams(OpBase *op) {
	/* Merge has 2 children if it is the first clause, and 3 otherwise.
	   The order of these children is critical:
	   - If there are 3 children, the first should resolve the Merge pattern's bound variables.
	   - The next (first if there are 2 children, second otherwise) should attempt to match the pattern.
	   - The last creates the pattern.
	*/
	OpBase *bound_variable_stream = NULL;
	OpBase *match_stream = NULL;
	OpBase *create_stream = NULL;
	if(op->childCount == 2) {
		// If we only have 2 streams, we simply need to determine which has a Create op.
		if(_LocateOp(op->children[0], OPType_CREATE)) {
			// If the Create op is in the first stream, swap the children.
			// Otherwise, the order is already correct.
			create_stream = op->children[0];
			op->children[0] = op->children[1];
			op->children[1] = create_stream;
		}
		return;
	}

	// Handling the three-stream case.
	for(int i = 0; i < op->childCount; i ++) {
		OpBase *child = op->children[i];
		bool child_has_argument = _LocateOp(child, OPType_ARGUMENT);
		// The bound variable stream is the only stream not populated by an Argument op.
		if(!bound_variable_stream && !child_has_argument) {
			bound_variable_stream = child;
			continue;
		}

		// The Create stream is the only stream with a Create op and Argument op.
		if(!create_stream && _LocateOp(child, OPType_CREATE) && child_has_argument) {
			create_stream = child;
			continue;
		}

		// The Match stream has an unknown set of operations, but is the only other stream
		// populated by an Argument op.
		if(!match_stream && child_has_argument) {
			match_stream = child;
			continue;
		}
	}

	assert(bound_variable_stream && match_stream && create_stream);

	// Migrate the children so that EXPLAIN calls print properly.
	op->children[0] = bound_variable_stream;
	op->children[1] = match_stream;
	op->children[2] = create_stream;
	OpMerge *op_merge = (OpMerge *)op;
	// op_merge->bound_variable_stream = op->children[0] = bound_variable_stream;
	// op_merge->match_stream = op->children[1] = match_stream;
	// op_merge->create_stream = op->children[2] = create_stream;
}

static void MergeFree(OpBase *ctx) {
}

