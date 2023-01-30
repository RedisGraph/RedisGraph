/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "op_merge.h"
#include "../../RG.h"
#include "../../errors.h"
#include "op_merge_create.h"
#include "../../query_ctx.h"
#include "../../schema/schema.h"
#include "../../util/rax_extensions.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "../execution_plan_build/execution_plan_modify.h"

// forward declarations
static OpResult MergeInit(OpBase *opBase);
static Record MergeConsume(OpBase *opBase);
static OpBase *MergeClone(const ExecutionPlan *plan, const OpBase *opBase);
static void MergeFree(OpBase *opBase);

//------------------------------------------------------------------------------
// ON MATCH / ON CREATE logic
//------------------------------------------------------------------------------

// apply a set of updates to the given records
static void _UpdateProperties
(
	PendingUpdateCtx **node_pending_updates,
	PendingUpdateCtx **edge_pending_updates,
	ResultSetStatistics *stats,
	raxIterator updates,
	Record *records,
	uint record_count
) {
	ASSERT(record_count > 0);
	GraphContext *gc = QueryCtx_GetGraphCtx();

	for(uint i = 0; i < record_count; i ++) {  // for each record to update
		Record r = records[i];
		// evaluate update expressions
		raxSeek(&updates, "^", NULL, 0);
		while(raxNext(&updates)) {
			EntityUpdateEvalCtx *ctx = updates.data;
			EvalEntityUpdates(gc, node_pending_updates, edge_pending_updates,
					r, ctx, true);
		}
	}
}

//------------------------------------------------------------------------------
// Merge logic
//------------------------------------------------------------------------------
static inline Record _pullFromStream
(
	OpBase *branch
) {
	return OpBase_Consume(branch);
}

static void _InitializeUpdates
(
	OpMerge *op,
	rax *updates,
	raxIterator *it
) {
	// if we have ON MATCH / ON CREATE directives
	// set the appropriate record IDs of entities to be updated
	raxStart(it, updates);
	raxSeek(it, "^", NULL, 0);
	// iterate over all expressions
	while(raxNext(it)) {
		EntityUpdateEvalCtx *ctx = it->data;
		// set the record index for every entity modified by this operation
		ctx->record_idx = OpBase_Modifies((OpBase *)op, ctx->alias);
	}

}

// free node and edge pending updates
static inline void _free_pending_updates
(
	OpMerge *op
) {
	if(op->node_pending_updates) {
		uint pending_updates_count = array_len(op->node_pending_updates);
		for(uint i = 0; i < pending_updates_count; i++) {
			PendingUpdateCtx *pending_update = op->node_pending_updates + i;
			AttributeSet_Free(&pending_update->attributes);
		}
		array_free(op->node_pending_updates);
		op->node_pending_updates = NULL;
	}

	if(op->edge_pending_updates) {
		uint pending_updates_count = array_len(op->edge_pending_updates);
		for(uint i = 0; i < pending_updates_count; i++) {
			PendingUpdateCtx *pending_update = op->edge_pending_updates + i;
			AttributeSet_Free(&pending_update->attributes);
		}
		array_free(op->edge_pending_updates);
		op->edge_pending_updates = NULL;
	}
}

OpBase *NewMergeOp
(
	const ExecutionPlan *plan,
	rax *on_match,
	rax *on_create
) {
	// merge is an operator with two or three children
	// they will be created outside of here
	// as with other multi-stream operators
	// (see CartesianProduct and ValueHashJoin)
	OpMerge *op = rm_calloc(1, sizeof(OpMerge));

	op->stats                = NULL;
	op->on_match             = on_match;
	op->on_create            = on_create;
	op->node_pending_updates = NULL;
	op->edge_pending_updates = NULL;
	
	// set our Op operations
	OpBase_Init((OpBase *)op, OPType_MERGE, "Merge", MergeInit, MergeConsume,
			NULL, NULL, MergeClone, MergeFree, true, plan);

	if(op->on_match) _InitializeUpdates(op, op->on_match, &op->on_match_it);
	if(op->on_create) _InitializeUpdates(op, op->on_create, &op->on_create_it);

	return (OpBase *)op;
}

// modification of ExecutionPlan_LocateOp that only follows LHS child
// Otherwise, the assumptions of Merge_SetStreams fail in MERGE..MERGE queries
// Match and Create streams are always guaranteed to not branch
// (have any ops with multiple children)
static OpBase *_LocateOp
(
	OpBase *root,
	OPType type
) {
	OpBase *ret;

	if(!root) {
		ret = NULL;
	}
	else if(root->type == type) {
		ret = root;
	}
	else if(root->childCount > 0) {
		ret = _LocateOp(root->children[0], type);
	} else {
		ret = NULL;
	}

	return ret;
}

static OpResult MergeInit
(
	OpBase *opBase
) {
	// merge has 2 children if it is the first clause, and 3 otherwise
	// - if there are 3 children
	//   the first should resolve the Merge pattern's bound variables
	// - the next (first if there are 2 children, second otherwise)
	//   should attempt to match the pattern
	// - the last creates the pattern
	ASSERT(opBase->childCount == 2 || opBase->childCount == 3);
	OpMerge *op = (OpMerge *)opBase;
	op->stats = QueryCtx_GetResultSetStatistics();
	if(opBase->childCount == 2) {
		// if we only have 2 streams
		// we simply need to determine which has a MergeCreate op
		if(_LocateOp(opBase->children[0], OPType_MERGE_CREATE)) {
			// if the Create op is in the first stream, swap the children
			// otherwise, the order is already correct
			OpBase *tmp = opBase->children[0];
			opBase->children[0] = opBase->children[1];
			opBase->children[1] = tmp;
		}

		op->match_stream = opBase->children[0];
		op->create_stream = opBase->children[1];
		return OP_OK;
	}

	// handling the three-stream case
	for(int i = 0; i < opBase->childCount; i ++) {
		OpBase *child = opBase->children[i];

		bool child_has_merge = _LocateOp(child, OPType_MERGE);
		// neither Match stream and Create stream have a Merge op
		// the bound variable stream will have a Merge op in-case of a merge merge query
		// MERGE (a:A) MERGE (b:B)
		// In which case the first Merge has yet to order its streams!
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

	// migrate the children so that EXPLAIN calls print properly
	opBase->children[0] = op->bound_variable_stream;
	opBase->children[1] = op->match_stream;
	opBase->children[2] = op->create_stream;

	// find and store references to the:
	// Argument taps for the Match and Create streams
	// the Match stream is populated by an Argument tap
	// store a reference to it
	op->match_argument_tap =
		(Argument *)ExecutionPlan_LocateOp(op->match_stream, OPType_ARGUMENT);

	// if the create stream is populated by an Argument tap, store a reference to it.
	op->create_argument_tap =
		(Argument *)ExecutionPlan_LocateOp(op->create_stream, OPType_ARGUMENT);

	// set up an array to store records produced by the bound variable stream
	op->input_records = array_new(Record, 1);

	return OP_OK;
}

static Record _handoff
(
	OpMerge *op
) {
	Record r = NULL;
	if(array_len(op->output_records)) {
		r = array_pop(op->output_records);
	}
	return r;
}

static Record MergeConsume
(
	OpBase *opBase
) {
	OpMerge *op = (OpMerge *)opBase;

	//--------------------------------------------------------------------------
	// handoff
	//--------------------------------------------------------------------------

	// return mode, all data was consumed
	if(op->output_records) {
		return _handoff(op);
	}

	//--------------------------------------------------------------------------
	// consume bound stream
	//--------------------------------------------------------------------------

	op->output_records = array_new(Record, 32);
	// if we have a bound variable stream
	// pull from it and store records until depleted
	if(op->bound_variable_stream) {
		Record input_record;
		while((input_record = _pullFromStream(op->bound_variable_stream))) {
			array_append(op->input_records, input_record);
		}
	}

	//--------------------------------------------------------------------------
	// match pattern
	//--------------------------------------------------------------------------

	uint match_count         = 0;
	bool reading_matches     = true;
	bool must_create_records = false;
	// match mode: attempt to resolve the pattern for every record from
	// the bound variable stream, or once if we have no bound variables
	while(reading_matches) {
		Record lhs_record = NULL;
		if(op->input_records) {
			// if we had bound variables but have depleted our input records,
			// we're done pulling from the Match stream
			if(array_len(op->input_records) == 0) {
				break;
			}

			// pull a new input record
			lhs_record = array_pop(op->input_records);
			// propagate record to the top of the Match stream
			// (must clone the Record, as it will be freed in the Match stream)
			Argument_AddRecord(op->match_argument_tap, OpBase_CloneRecord(lhs_record));
		} else {
			// this loop only executes once if we don't have input records
			// resolving bound variables
			reading_matches = false;
		}

		Record rhs_record;
		bool should_create_pattern = true;
		// retrieve Records from the Match stream until it's depleted
		while((rhs_record = _pullFromStream(op->match_stream))) {
			// pattern was successfully matched
			should_create_pattern = false;
			array_append(op->output_records, rhs_record);
			match_count++;
		}

		if(should_create_pattern) {
			// transfer the LHS record to the Create stream
			// to build once we finish reading
			// we don't need to clone the record
			// as it won't be accessed again outside that stream
			// but we must make sure its elements are access-safe
			// as the input stream will be freed
			// before entities are created
			if(lhs_record) {
				Record_PersistScalars(lhs_record);
				Argument_AddRecord(op->create_argument_tap, lhs_record);
				lhs_record = NULL;
			}
			Record r = _pullFromStream(op->create_stream);
			UNUSED(r);
			ASSERT(r == NULL); // don't expect returned records
			must_create_records = true;
		}

		// free the LHS Record if we haven't transferred it to the Create stream
		if(lhs_record) {
			OpBase_DeleteRecord(lhs_record);
		}
	}

	//--------------------------------------------------------------------------
	// compute updates and create
	//--------------------------------------------------------------------------

	// explicitly free the read streams in case either holds an index read lock
	if(op->bound_variable_stream) {
		OpBase_PropagateReset(op->bound_variable_stream);
	}
	OpBase_PropagateReset(op->match_stream);

	op->node_pending_updates = array_new(PendingUpdateCtx, 0);
	op->edge_pending_updates = array_new(PendingUpdateCtx, 0);

	// if we are setting properties with ON MATCH, compute all pending updates
	if(op->on_match && match_count > 0) {
		_UpdateProperties(&op->node_pending_updates, &op->edge_pending_updates,
			op->stats, op->on_match_it, op->output_records, match_count);
	}

	if(must_create_records) {
		// commit all pending changes on the Create stream
		// 'MergeCreate_Commit' acquire write lock!
		// write lock is released further down
		MergeCreate_Commit(op->create_stream);

		// we only need to pull the created records if we're returning results
		// or performing updates on creation
		// TODO: isn't op->stats always != NULL ?
		if(op->stats || op->on_create) {
			// pull all records from the Create stream
			uint create_count = 0;
			Record created_record;
			while((created_record = _pullFromStream(op->create_stream))) {
				array_append(op->output_records, created_record);
				create_count++;
			}

			// if we are setting properties with ON CREATE
			// compute all pending updates
			// TODO: note we're under lock at this point! is there a way
			// to compute these changes before locking ?
			if(op->on_create) {
				_UpdateProperties(&op->node_pending_updates,
					&op->edge_pending_updates, op->stats, op->on_create_it,
					op->output_records + match_count, create_count);
			}
		}
	}

	//--------------------------------------------------------------------------
	// update
	//--------------------------------------------------------------------------

	if(array_len(op->node_pending_updates) > 0 ||
	   array_len(op->edge_pending_updates) > 0) {
		GraphContext *gc = QueryCtx_GetGraphCtx();
		// lock everything
		QueryCtx_LockForCommit(); {
			CommitUpdates(gc, op->stats, op->node_pending_updates, ENTITY_NODE);
			CommitUpdates(gc, op->stats, op->edge_pending_updates, ENTITY_EDGE);
		}
	}

	//--------------------------------------------------------------------------
	// free updates
	//--------------------------------------------------------------------------

	_free_pending_updates(op);

	return _handoff(op);
}

static OpBase *MergeClone
(
	const ExecutionPlan *plan,
	const OpBase *opBase
) {
	ASSERT(opBase->type == OPType_MERGE);

	OpMerge *op    = (OpMerge *)opBase;
	rax *on_match  = NULL;
	rax *on_create = NULL;

	if(op->on_match) on_match = raxCloneWithCallback(op->on_match,
			(void *(*)(void *))UpdateCtx_Clone);

	if(op->on_create) on_create = raxCloneWithCallback(op->on_create,
			(void *(*)(void *))UpdateCtx_Clone);

	return NewMergeOp(plan, on_match, on_create);
}

static void MergeFree
(
	OpBase *opBase
) {
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

	_free_pending_updates(op);

	if(op->on_match) {
		raxFreeWithCallback(op->on_match, (void(*)(void *))UpdateCtx_Free);
		op->on_match = NULL;
		raxStop(&op->on_match_it);
	}

	if(op->on_create) {
		raxFreeWithCallback(op->on_create, (void(*)(void *))UpdateCtx_Free);
		op->on_create = NULL;
		raxStop(&op->on_create_it);
	}
}

