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
// Perform necessary index updates.
static void _UpdateIndices(GraphContext *gc, Node *n) {
	Graph *g = gc->g;
	uint label_count = GraphContext_SchemaCount(gc, SCHEMA_NODE);
	if(label_count == 0) return;
	GrB_Index labels[label_count];

	label_count = Graph_GetNodeLabels(g, n, labels, label_count);

	for(uint i = 0; i < label_count; i++) {
		int label_id = labels[i];
		Schema *s = GraphContext_GetSchemaByID(gc, label_id, SCHEMA_NODE);
		if(Schema_HasIndices(s)) Schema_AddNodeToIndices(s, n);
	}
}

// Update the appropriate property on a graph entity.
static int _UpdateProperty(Record r, GraphEntity *ge, EntityUpdateEvalCtx *update_ctx) {
	int res = 1;
	SIValue new_value = AR_EXP_Evaluate(update_ctx->exp, r);

	// Emit an error and exit if we're trying to add an invalid type.
	if(!(SI_TYPE(new_value) & SI_VALID_PROPERTY_VALUE)) {
		res = 0;
		Error_InvalidPropertyValue();
		ErrorCtx_RaiseRuntimeException(NULL);
		goto cleanup;
	}

	// Try to get current property value.
	SIValue *old_value = GraphEntity_GetProperty(ge, update_ctx->attribute_id);

	if(old_value == PROPERTY_NOTFOUND) {
		// Adding a new property; do nothing if its value is NULL.
		if(SI_TYPE(new_value) == T_NULL) {
			res = 0;
			goto cleanup;
		}
		// Add new property.
		GraphEntity_AddProperty(ge, update_ctx->attribute_id, new_value);
	} else {
		// Update property.
		GraphEntity_SetProperty(ge, update_ctx->attribute_id, new_value);
	}

cleanup:
	SIValue_Free(new_value);
	return res;
}

// Apply a set of updates to the given records.
static void _UpdateProperties(ResultSetStatistics *stats, EntityUpdateEvalCtx *updates,
							  Record *records, uint record_count) {
	ASSERT(updates != NULL && record_count > 0);
	uint update_count = array_len(updates);
	uint failed_updates = 0;
	GraphContext *gc = QueryCtx_GetGraphCtx();
	// Lock everything.
	QueryCtx_LockForCommit();

	for(uint i = 0; i < record_count; i ++) {  // For each record to update
		Record r = records[i];
		for(uint j = 0; j < update_count; j ++) { // For each pending update.
			EntityUpdateEvalCtx *update_ctx = &updates[j];

			// Get the type of the entity to update. If the expected entity was not
			// found, make no updates but do not error.
			RecordEntryType t = Record_GetType(r, update_ctx->record_idx);
			if(t != REC_TYPE_NODE && t != REC_TYPE_EDGE) {
				failed_updates++;
				continue;
			}

			GraphEntity *ge = Record_GetGraphEntity(r, update_ctx->record_idx);

			int res = _UpdateProperty(r, ge, update_ctx); // Update the entity.
			if(res == 0) {
				failed_updates++;
				continue;
			}
			if(t == REC_TYPE_NODE) _UpdateIndices(gc, (Node *)ge); // Update indices if necessary.
		}
	}
	if(stats) stats->properties_set += (update_count * record_count) - failed_updates;
}

//------------------------------------------------------------------------------
// Merge logic
//------------------------------------------------------------------------------
static inline Record _pullFromStream(OpBase *branch) {
	return OpBase_Consume(branch);
}

OpBase *NewMergeOp(const ExecutionPlan *plan, EntityUpdateEvalCtx *on_match,
				   EntityUpdateEvalCtx *on_create) {

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
		uint on_match_count = array_len(op->on_match);
		for(uint i = 0; i < on_match_count; i ++) {
			op->on_match[i].record_idx = OpBase_Modifies((OpBase *)op, op->on_match[i].alias);
		}
	}

	if(op->on_create) {
		// If we have ON CREATE directives, set the appropriate record IDs of entities to be updated.
		uint on_create_count = array_len(op->on_create);
		for(uint i = 0; i < on_create_count; i ++) {
			op->on_create[i].record_idx = OpBase_Modifies((OpBase *)op, op->on_create[i].alias);
		}
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
	EntityUpdateEvalCtx *on_match;
	EntityUpdateEvalCtx *on_create;
	array_clone_with_cb(on_match, op->on_match, EntityUpdateEvalCtx_Clone);
	array_clone_with_cb(on_create, op->on_create, EntityUpdateEvalCtx_Clone);
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
		uint on_match_count = array_len(op->on_match);
		for(uint i = 0; i < on_match_count; i ++) {
			AR_EXP_Free(op->on_match[i].exp);
		}
		array_free(op->on_match);
		op->on_match = NULL;
	}

	if(op->on_create) {
		uint on_create_count = array_len(op->on_create);
		for(uint i = 0; i < on_create_count; i ++) {
			AR_EXP_Free(op->on_create[i].exp);
		}
		array_free(op->on_create);
		op->on_create = NULL;
	}
}

