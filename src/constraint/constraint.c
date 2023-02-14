/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "value.h"
#include "constraint.h"
#include "../util/arr.h"
#include "../util/thpool/pools.h"
#include "../graph/entities/attribute_set.h"

#include <stdatomic.h>

// constraint enforcement callback function
typedef bool (*EnforcementCB)
(
	Constraint c,
	const GraphEntity *e
);

// opaque structure representing a constraint
typedef struct _Constraint {
	uint n_attr;                   // number of fields
	ConstraintType t;              // constraint type
	EnforcementCB enforce;         // enforcement function
    Attribute_ID *attrs;           // enforced attributes
	const char **attr_names;       // enforced attribute names
    ConstraintStatus status;       // constraint status
    uint _Atomic pending_changes;  // number of pending changes
	EntityType et;                 // entity type
} _Constraint;

// constraint enforce context
typedef struct ConstraintEnforceCtx {
	Graph *g;      // graph object
	Constraint c;  // constraint to enforce
} ConstraintEnforceCtx;

// enforce constraint on all relevant nodes
static void _Constraint_EnforceNodes
(
	void *args
) {
	// check if constraint holds
	// scan through all entities governed by this constraint and enforce

	ASSERT(args != NULL);

	ConstraintEnforceCtx *ctx = (ConstraintEnforceCtx*)args;

	Constraint c = ctx->c;
	LabelID    l = c->l;
	Graph     *g = ctx->g;

	bool               holds      = true;   // constraint holds
	GrB_Index          rowIdx     = 0;      // current row being scanned
	int                enforced   = 0;      // #entities in current batch
	int                batch_size = 10000;  // #entities to enforce in one go
	RG_MatrixTupleIter it         = {0};    // matrix iterator

	while(holds && true) {
		// lock graph for reading
		Graph_AcquireReadLock(g);

		const RG_Matrix m = Graph_GetLabelMatrix(g, l);
		ASSERT(m != NULL);

		//----------------------------------------------------------------------
		// resume scanning from rowIdx
		//----------------------------------------------------------------------

		GrB_Info info;
		info = RG_MatrixTupleIter_attach(&it, m);
		ASSERT(info == GrB_SUCCESS);
		info = RG_MatrixTupleIter_iterate_range(&it, rowIdx, UINT64_MAX);
		ASSERT(info == GrB_SUCCESS);

		//----------------------------------------------------------------------
		// batch enforce nodes
		//----------------------------------------------------------------------

		EntityID id;
		while(enforced < batch_size &&
			  RG_MatrixTupleIter_next_BOOL(&it, &id, NULL, NULL) == GrB_SUCCESS)
		{
			Node n;
			Graph_GetNode(g, id, &n);
			if(!c->enforce(c, (GraphEntity*)&n)) {
				// found node which violate constraint	
				holds = false;
				break;
			}
			enforced++;
		}

		//----------------------------------------------------------------------
		// done with current batch
		//----------------------------------------------------------------------

		if(enforced != batch_size) {
			// iterator depleted, no more nodes to enforce
			break;
		} else {
			// release read lock
			Graph_ReleaseLock(g);

			// finished current batch
			RG_MatrixTupleIter_detach(&it);

			// continue next batch from row id+1
			// this is true because we're iterating over a diagonal matrix
			rowIdx = id + 1;
		}
	}

	RG_MatrixTupleIter_detach(&it);

	// update constraint status
	ConstraintStatus status = (holds) ? CT_ACTIVE : CT_FAILED;
	Constraint_SetStatus(c, status);

	// release read lock
	Graph_ReleaseLock(g);
}

// enforce constraint on all relevant edges
static void _Constraint_EnforceEdges
(
	void *args
) {
	ASSERT(args != NULL);

	ConstraintEnforceCtx *ctx = (ConstraintEnforceCtx*)args;

	Constraint c = ctx->c;
	LabelID    l = c->l;
	Graph     *g = ctx->g;

	GrB_Info  info;
	bool holds             = true;  // constraint holds
	EntityID  src_id       = 0;     // current processed row idx
	EntityID  dest_id      = 0;     // current processed column idx
	EntityID  edge_id      = 0;     // current processed edge id
	EntityID  prev_src_id  = 0;     // last processed row idx
	EntityID  prev_dest_id = 0;     // last processed column idx
	int       enforced     = 0;     // number of entities enforced in current batch
	int       batch_size   = 1000;  // max number of entities to enforce in one go
	RG_MatrixTupleIter it  = {0};

	while(holds && true) {
		// lock graph for reading
		Graph_AcquireReadLock(g);

		// reset number of enforced edges in batch
		enforced     = 0;
		prev_src_id  = src_id;
		prev_dest_id = dest_id;

		// fetch relation matrix
		const RG_Matrix m = Graph_GetRelationMatrix(g, l, false);
		ASSERT(m != NULL);

		//----------------------------------------------------------------------
		// resume scanning from previous row/col indices
		//----------------------------------------------------------------------

		info = RG_MatrixTupleIter_attach(&it, m);
		ASSERT(info == GrB_SUCCESS);
		info = RG_MatrixTupleIter_iterate_range(&it, src_id, UINT64_MAX);
		ASSERT(info == GrB_SUCCESS);

		// skip previously enforced edges
		while((info = RG_MatrixTupleIter_next_UINT64(&it, &src_id, &dest_id,
						&edge_id)) == GrB_SUCCESS &&
				src_id == prev_src_id &&
				dest_id <= prev_dest_id);

		// process only if iterator is on an active entry
		if(info != GrB_SUCCESS) {
			break;
		}

		//----------------------------------------------------------------------
		// batch enforce edges
		//----------------------------------------------------------------------

		do {
			Edge e;
			e.srcNodeID  = src_id;
			e.destNodeID = dest_id;
			e.relationID = l;

			if(SINGLE_EDGE(edge_id)) {
				Graph_GetEdge(g, edge_id, &e);
				if(!c->enforce(c, (GraphEntity*)&e)) {
					holds = false;
					break;
				}
			} else {
				EdgeID *edgeIds = (EdgeID *)(CLEAR_MSB(edge_id));
				uint edgeCount = array_len(edgeIds);

				for(uint i = 0; i < edgeCount; i++) {
					edge_id = edgeIds[i];
					Graph_GetEdge(g, edge_id, &e);
					if(!c->enforce(c, (GraphEntity*)&e)) {
						holds = false;
						break;
					}
				}
			}
			enforced++; // single/multi edge are counted similarly
		} while(enforced < batch_size &&
			  RG_MatrixTupleIter_next_UINT64(&it, &src_id, &dest_id, &edge_id)
				== GrB_SUCCESS && holds);

		//----------------------------------------------------------------------
		// done with current batch
		//----------------------------------------------------------------------

		if(enforced != batch_size) {
			// iterator depleted, no more edges to enforce
			break;
		} else {
			// finished current batch
			// release read lock
			Graph_ReleaseLock(g);
			RG_MatrixTupleIter_detach(&it);
		}
	}

	RG_MatrixTupleIter_detach(&it);

	// update constraint status
	ConstraintStatus status = (holds) ? CT_ACTIVE : CT_FAILED;
	Constraint_SetStatus(c, status);

	// release read lock
	Graph_ReleaseLock(g);
}

// returns constraint's type
ConstraintType Constraint_GetType
(
	const Constraint c  // constraint to query
) {
	ASSERT(c != NULL);
	return c->t;
}

// returns constraint status
ConstraintStatus Constraint_GetStatus
(
	const Constraint c
) {
	ASSERT(c != NULL);
	return c->status;
}

// set constraint status
// status can change from:
// 1. CT_PENDING to CT_ACTIVE
// 2. CT_PENDING to CT_FAILED
void Constraint_SetStatus
(
	Constraint c,            // constraint to update
	ConstraintStatus status  // new status
) {
	// validations
	// validate state transition
	ASSERT(c->status == CT_PENDING);
	ASSERT(status == CT_ACTIVE || status == CT_FAILED);

	// assuming under lock
    c->status = status;
}

// returns a shallow copy of constraint attributes
uint Constraint_GetAttributes
(
	const Constraint c,             // constraint from which to get attributes
	const Attribute_ID **attr_ids,  // array of constraint attribute IDs
	const char ***attr_names        // array of constraint attribute names
) {
	ASSERT(c != NULL);

	if(attr_ids != NULL) {
		*attr_ids = c->attrs;
	}

	if(attr_names != NULL) {
		*attr_names = c->attr_names;
	}

	return c->n_attr;
}

// checks if constraint enforces attribute
bool Constraint_ContainsAttribute
(
	Constraint c,         // constraint to query
	Attribute_ID attr_id  // enforced attribute
) {
	for(int i = 0; i < c->n_attr; i++) {
		if(c->attrs[i] == attr_id) {
			return true;
		}
    }

    return false;
}

// returns number of pending changes
int Constraint_PendingChanges
(
	const Constraint c  // constraint to inquery
) {
	// validations
	ASSERT(c != NULL);

	// the number of pending changes of a constraint can not be more than 2
	// CREATE + DROP will yield 2 pending changes, a dropped constraint can not
	// be dropped again
	//
	// CREATE + CREATE will result in two different constraints each with its
	// own pending changes
    ASSERT(c->pending_changes <= 2);

	return c->pending_changes;
}

// increment number of pending changes
void Constraint_IncPendingChanges
(
	Constraint c  // constraint to update
) {
	ASSERT(c != NULL);

	// one can't create or drop the same constraint twice
	// see comment at Constraint_PendingChanges
    ASSERT(c->pending_changes > 0);
    ASSERT(c->pending_changes <= 2);

	// update constraint status to pending
	c->status = CT_PENDING;

	// atomic increment
	c->pending_changes++;
}

// decrement number of pending changes
void Constraint_DecPendingChanges
(
	Constraint c  // constraint to update
) {
	ASSERT(c != NULL);

	// one can't create or drop the same constraint twice
	// see comment at Constraint_PendingChanges
    ASSERT(c->pending_changes > 0);
    ASSERT(c->pending_changes <= 2);

	// atomic decrement
	c->pending_changes--;
}

// tries to enforce constraint
// sets constraint status as pending
void Constraint_Enforce
(
	Constraint c,  // constraint to enforce
	Graph *g
) {
	ASSERT(c  != NULL);
	ASSERT(g != NULL);

	// mark constraint as pending
	Constraint_IncPendingChanges(c);

	// build enforce context
	ConstraintEnforceCtx *ctx = rm_malloc(sizeof(ConstraintEnforceCtx));
	ctx->c = c;
	ctx->g = g;

	if(c->et == GETYPE_NODE) {
		ThreadPools_AddWorkReader(_Constraint_EnforceNodes, (void*)ctx, true);
	} else {
		ThreadPools_AddWorkReader(_Constraint_EnforceEdges, (void*)ctx, true);
	}
}

// enforce constraint on entity
// returns true if entity satisfies the constraint
// false otherwise
bool Constraint_EnforceEntity
(
	Constraint c,             // constraint to enforce
	const GraphEntity *e      // enforced entity
) {
	ASSERT(c != NULL);
	ASSERT(e != NULL);

	return c->enforce(c, e);
}

void Constraint_Free
(
	Constraint c
) {
	ASSERT(c != NULL);

    rm_free(c->attrs);
	rm_free(c->attr_names);
    rm_free(c);
}

