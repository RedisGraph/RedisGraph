/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_merge.h"
#include "../../RG.h"
#include "../../errors.h"
#include "op_merge_create.h"
#include "../../query_ctx.h"
#include "../../schema/schema.h"
#include "shared/update_functions.h"
#include "../../util/rax_extensions.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "../execution_plan_build/execution_plan_modify.h"

/* Forward declarations. */
static OpResult MergeInit(OpBase *opBase);
static Record MergeConsume(OpBase *opBase);
static OpBase *MergeClone(const ExecutionPlan *plan, const OpBase *opBase);
static void MergeFree(OpBase *opBase);

//------------------------------------------------------------------------------
// ON MATCH / ON CREATE logic
//------------------------------------------------------------------------------
// Apply a set of updates to the given records.
static void _UpdateProperties(ResultSetStatistics *stats, rax *updates,
							  Record *records, uint record_count) {
	ASSERT(updates != NULL && record_count > 0);
	GraphContext *gc = QueryCtx_GetGraphCtx();
	// Lock everything.
	QueryCtx_LockForCommit();

	PendingUpdateCtx *pending_updates = array_new(PendingUpdateCtx, raxSize(updates));
	for(uint i = 0; i < record_count; i ++) {  // For each record to update
		Record r = records[i];
		// Evaluate update expressions.
		raxIterator it;
		raxStart(&it, updates);
		raxSeek(&it, "^", NULL, 0);
		while(raxNext(&it)) {
			EvalEntityUpdates(gc, &pending_updates, r, (char *)it.key, it.data, false);
			CommitUpdates(gc, stats, pending_updates);
			array_clear(pending_updates); // TODO freeing required?
		}
		raxStop(&it);
	}
	array_free(pending_updates);
}

//------------------------------------------------------------------------------
// Merge logic
//------------------------------------------------------------------------------
static inline Record _pullFromStream(OpBase *branch) {
	return OpBase_Consume(branch);
}

OpBase *NewMergeOp(const ExecutionPlan *plan, rax *on_match, rax *on_create) {

	/* Merge is an operator with two or three children. They will be created outside of here,
	 * as with other multi-stream operators (see CartesianProduct and ValueHashJoin). */
	OpMerge *op = rm_calloc(1, sizeof(OpMerge));
	op->stats = NULL;
	op->on_match = on_match;
	op->on_create = on_create;
	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_MERGE, "Merge", MergeInit, MergeConsume, NULL, NULL, MergeClone,
				MergeFree, true, plan);

	if(op->on_match) {
		// If we have ON MATCH directives, set the appropriate record IDs of entities to be updated.
		raxIterator it;
		raxStart(&it, op->on_match);
		raxSeek(&it, "^", NULL, 0);
		// Iterate over all ON MATCH expressions
		while(raxNext(&it)) {
			char *alias = (char *)it.key;
			EntityUpdateEvalCtx *ctx = it.data;
			// Set the record index for every entity modified by this operation
			ctx->record_idx = OpBase_Modifies((OpBase *)op, alias);
		}
		raxStop(&it);
	}

	if(op->on_create) {
		// If we have ON CREATE directives, set the appropriate record IDs of entities to be updated.
		raxIterator it;
		raxStart(&it, op->on_create);
		raxSeek(&it, "^", NULL, 0);
		// Iterate over all ON CREATE expressions
		while(raxNext(&it)) {
			char *alias = (char *)it.key;
			EntityUpdateEvalCtx *ctx = it.data;
			// Set the record index for every entity modified by this operation
			ctx->record_idx = OpBase_Modifies((OpBase *)op, alias);
		}
		raxStop(&it);
	}

	return (OpBase *)op;
}

// Modification of ExecutionPlan_LocateOp that only follows LHS child.
// Otherwise, the assumptions of Merge_SetStreams fail in MERGE..MERGE queries.
// Match and Create streams are always guaranteed to not branch (have any ops with multiple children).
static OpBase *_LocateOp(OpBase *root, OPType type) {
	if(!root) return NULL;
	if(root->type == type) return root;
	if(root->childCount > 0) return _LocateOp(root->children[0], type);
	return NULL;
}

static OpResult MergeInit(OpBase *opBase) {
	/* Merge has 2 children if it is the first clause, and 3 otherwise.
	 * - If there are 3 children, the first should resolve the Merge pattern's bound variables.
	 * - The next (first if there are 2 children, second otherwise) should attempt to match the pattern.
	 * - The last creates the pattern. */
	ASSERT(opBase->childCount == 2 || opBase->childCount == 3);
	OpMerge *op = (OpMerge *)opBase;
	op->stats = QueryCtx_GetResultSetStatistics();
	if(opBase->childCount == 2) {
		// If we only have 2 streams, we simply need to determine which has a MergeCreate op.
		if(_LocateOp(opBase->children[0], OPType_MERGE_CREATE)) {
			// If the Create op is in the first stream, swap the children.
			// Otherwise, the order is already correct.
			OpBase *tmp = opBase->children[0];
			opBase->children[0] = opBase->children[1];
			opBase->children[1] = tmp;
		}

		op->match_stream = opBase->children[0];
		op->create_stream = opBase->children[1];
		return OP_OK;
	}

	// Handling the three-stream case.
	for(int i = 0; i < opBase->childCount; i ++) {
		OpBase *child = opBase->children[i];

		bool child_has_merge = _LocateOp(child, OPType_MERGE);
		/* Nither Match stream and Create stream have a Merge op
		 * the bound variable stream will have a Merge op in-case of a merge merge query
		 * MERGE (a:A) MERGE (b:B)
		 * In which case the first Merge has yet to order its streams! */
		if(!op->bound_variable_stream && child_has_merge) {
			op->bound_variable_stream = child;
			continue;
		}

		bool child_has_argument = _LocateOp(child, OPType_ARGUMENT);
		// The bound variable stream is the only stream not populated by an Argument op.
		if(!op->bound_variable_stream && !child_has_argument) {
			op->bound_variable_stream = child;
			continue;
		}

		// The Create stream is the only stream with a MergeCreate op and Argument op.
		if(!op->create_stream && _LocateOp(child, OPType_MERGE_CREATE) && child_has_argument) {
			op->create_stream = child;
			continue;
		}

		// The Match stream has an unknown set of operations, but is the only other stream
		// populated by an Argument op.
		if(!op->match_stream && child_has_argument) {
			op->match_stream = child;
			continue;
		}
	}

	ASSERT(op->bound_variable_stream != NULL &&
		   op->match_stream != NULL &&
		   op->create_stream != NULL);

	// Migrate the children so that EXPLAIN calls print properly.
	opBase->children[0] = op->bound_variable_stream;
	opBase->children[1] = op->match_stream;
	opBase->children[2] = op->create_stream;

	// Find and store references to the Argument taps for the Match and Create streams.
	// The Match stream is populated by an Argument tap, store a reference to it.
	op->match_argument_tap = (Argument *)ExecutionPlan_LocateOp(op->match_stream, OPType_ARGUMENT);
	// If the create stream is populated by an Argument tap, store a reference to it.
	op->create_argument_tap = (Argument *)ExecutionPlan_LocateOp(op->create_stream,
																 OPType_ARGUMENT);
	// Set up an array to store records produced by the bound variable stream.
	op->input_records = array_new(Record, 1);

	return OP_OK;
}

static Record _handoff(OpMerge *op) {
	Record r = NULL;
	if(array_len(op->output_records)) r = array_pop(op->output_records);
	return r;
}

static Record MergeConsume(OpBase *opBase) {
	OpMerge *op = (OpMerge *)opBase;

	// Return mode, all data was consumed.
	if(op->output_records) return _handoff(op);

	// Consume mode.
	op->output_records = array_new(Record, 32);
	// If we have a bound variable stream, pull from it and store records until depleted.
	if(op->bound_variable_stream) {
		Record input_record;
		while((input_record = _pullFromStream(op->bound_variable_stream))) {
			op->input_records = array_append(op->input_records, input_record);
		}
	}

	bool must_create_records = false;
	bool reading_matches = true;
	uint match_count = 0;
	// Match mode: attempt to resolve the pattern for every record from the bound variable
	// stream, or once if we have no bound variables.
	while(reading_matches) {
		Record lhs_record = NULL;
		if(op->input_records) {
			// If we had bound variables but have depleted our input records,
			// we're done pulling from the Match stream.
			if(array_len(op->input_records) == 0) break;

			// Pull a new input record.
			lhs_record = array_pop(op->input_records);
			// Propagate record to the top of the Match stream.
			// (Must clone the Record, as it will be freed in the Match stream.)
			Argument_AddRecord(op->match_argument_tap, OpBase_CloneRecord(lhs_record));
		} else {
			// This loop only executes once if we don't have input records resolving bound variables.
			reading_matches = false;
		}

		bool should_create_pattern = true;
		Record rhs_record;
		// Retrieve Records from the Match stream until it's depleted.
		while((rhs_record = _pullFromStream(op->match_stream))) {
			// Pattern was successfully matched.
			should_create_pattern = false;
			op->output_records = array_append(op->output_records, rhs_record);
			match_count++;
		}

		if(should_create_pattern) {
			/* Transfer the LHS record to the Create stream to build once we finish reading.
			 * We don't need to clone the record, as it won't be accessed again outside that stream,
			 * but we must make sure its elements are access-safe, as the input stream will be freed
			 * before entities are created. */
			if(lhs_record) {
				Record_PersistScalars(lhs_record);
				Argument_AddRecord(op->create_argument_tap, lhs_record);
				lhs_record = NULL;
			}
			Record r = _pullFromStream(op->create_stream);
			UNUSED(r);
			ASSERT(r == NULL); // Don't expect returned records
			must_create_records = true;
		}

		// Free the LHS Record if we haven't transferred it to the Create stream.
		if(lhs_record) OpBase_DeleteRecord(lhs_record);
	}

	// Explicitly free the read streams in case either holds an index read lock.
	if(op->bound_variable_stream) OpBase_PropagateFree(op->bound_variable_stream);
	OpBase_PropagateFree(op->match_stream);

	if(must_create_records) {
		// Commit all pending changes on the Create stream.
		MergeCreate_Commit(op->create_stream);
		// We only need to pull the created records if we're returning results or performing updates on creation.
		if(op->stats || op->on_create) {
			// Pull all records from the Create stream.
			if(!op->output_records) op->output_records = array_new(Record, 32);
			uint create_count = 0;
			Record created_record;
			while((created_record = _pullFromStream(op->create_stream))) {
				op->output_records = array_append(op->output_records, created_record);
				create_count ++;
			}
			// If we are setting properties with ON CREATE, execute updates on the just-added Records.
			if(op->on_create) {
				_UpdateProperties(op->stats, op->on_create, op->output_records + match_count, create_count);
			}
		}
	}

	// If we are setting properties with ON MATCH, execute all pending updates.
	if(op->on_match && match_count > 0)
		_UpdateProperties(op->stats, op->on_match, op->output_records, match_count);

	QueryCtx_UnlockCommit(&op->op); // Release the lock.

	return _handoff(op);
}

static OpBase *MergeClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_MERGE);
	OpMerge *op = (OpMerge *)opBase;
	rax *on_match = NULL;
	rax *on_create = NULL;
	if(op->on_match) on_match = raxCloneWithCallback(op->on_match, (void *(*)(void *))UpdateCtx_Clone);
	if(op->on_create) on_create = raxCloneWithCallback(op->on_create,
														   (void *(*)(void *))UpdateCtx_Clone);
	return NewMergeOp(plan, on_match, on_create);
}

static void MergeFree(OpBase *opBase) {
	OpMerge *op = (OpMerge *)opBase;
	if(op->input_records) {
		uint input_count = array_len(op->input_records);
		for(uint i = 0; i < input_count; i ++) {
			OpBase_DeleteRecord(op->input_records[i]);
		}
		array_free(op->input_records);
		op->input_records = NULL;
	}

	if(op->output_records) {
		uint output_count = array_len(op->output_records);
		for(uint i = 0; i < output_count; i ++) {
			OpBase_DeleteRecord(op->output_records[i]);
		}
		array_free(op->output_records);
		op->output_records = NULL;
	}

	if(op->on_match) {
		raxFreeWithCallback(op->on_match, (void(*)(void *))UpdateCtx_Free);
		op->on_match = NULL;
	}

	if(op->on_create) {
		raxFreeWithCallback(op->on_create, (void(*)(void *))UpdateCtx_Free);
		op->on_create = NULL;
	}
}

