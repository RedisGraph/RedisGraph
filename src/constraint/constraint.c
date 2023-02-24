/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "value.h"
#include "constraint.h"
#include "../util/arr.h"
#include "../index/indexer.h"
#include "../util/thpool/pools.h"
#include "../graph//graphcontext.h"
#include "../graph/entities/attribute_set.h"
#include "../graph/rg_matrix/rg_matrix_iter.h"

#include <stdatomic.h>

// Extern functions

// create a new unique constraint
extern Constraint Constraint_UniqueNew
(
	LabelID l,                // label/relation ID
	Attribute_ID *fields,     // enforced fields
	const char **attr_names,  // enforced attribute names
	uint n_fields,            // number of fields
	GraphEntityType et,       // entity type
	Index idx                 // index
);

// create a new exists constraint
extern Constraint Constraint_ExistsNew
(
	LabelID l,                // label/relation ID
	Attribute_ID *fields,     // enforced fields
	const char **attr_names,  // enforced attribute names
	uint n_fields,            // number of fields
	GraphEntityType et        // entity type
);

// opaque structure representing a constraint
typedef struct _Constraint {
	uint n_attr;                   // number of fields
	ConstraintType t;              // constraint type
	EnforcementCB enforce;         // enforcement function
	int lbl;                       // enforced label/relationship-type
    Attribute_ID *attrs;           // enforced attributes
	const char **attr_names;       // enforced attribute names
    ConstraintStatus status;       // constraint status
    uint _Atomic pending_changes;  // number of pending changes
	GraphEntityType et;            // entity type
} _Constraint;

// create a new constraint
Constraint Constraint_New
(
	struct GraphContext *gc,
	ConstraintType t,         // type of constraint
	LabelID l,                // label/relation ID
	Attribute_ID *fields,     // enforced fields
	const char **attr_names,  // enforced attribute names
	uint n_fields,            // number of fields
	GraphEntityType et        // entity type
) {
	ASSERT(t == CT_UNIQUE || t == CT_EXISTS);

	Constraint c = NULL;

	if(t == CT_UNIQUE) {
		// a unique constraints requires an index
		// try to get supporting index
		SchemaType st = (et == GETYPE_NODE) ? SCHEMA_NODE : SCHEMA_EDGE;
		Index idx = GraphContext_GetIndexByID((GraphContext*) gc, l, fields,
				n_fields, IDX_EXACT_MATCH, et);

		// supporting index is missing, can't create constraint
		if(idx == NULL) {
			return NULL;
		}

		// create a new unique constraint
		c = Constraint_UniqueNew(l, fields, attr_names, n_fields, et, idx);
	} else {
		// create a new exists constraint
		c = Constraint_ExistsNew(l, fields, attr_names, n_fields, et);
	}

	Constraint_SetStatus(c, CT_PENDING);

	return c;
}

// returns constraint's type
ConstraintType Constraint_GetType
(
	const Constraint c  // constraint to query
) {
	ASSERT(c != NULL);
	return c->t;
}

// returns constraint entity type
GraphEntityType Constraint_GetEntityType
(
	const Constraint c  // constraint to query
) {
	ASSERT(c != NULL);

	return c->et;
}

// returns constraint label/relationship-type
int Constraint_GetLabelID
(
	const Constraint c  // constraint to query
) {
	ASSERT(c != NULL);

	return c->lbl;
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
    ASSERT(c->pending_changes < 2);

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
void Constraint_Enforce
(
	Constraint c,            // constraint to enforce
	struct GraphContext *gc  // graph context
) {
	ASSERT(c != NULL);
	ASSERT(Constraint_GetStatus(c) == CT_PENDING);

	// mark constraint as pending
	Constraint_IncPendingChanges(c);

	// add constraint enforcement task
	Indexer_EnforceConstraint(c, (GraphContext*)gc);
}

// enforce constraint on all relevant nodes
void Constraint_EnforceNodes
(
	Constraint c,
	Graph *g
) {
	// check if constraint holds
	// scan through all entities governed by this constraint and enforce

	ASSERT(c != NULL);
	ASSERT(g != NULL);

	LabelID            l          = c->lbl;  // constraint label ID
	RG_MatrixTupleIter it         = {0};     // matrix iterator
	bool               holds      = true;    // constraint holds
	GrB_Index          rowIdx     = 0;       // current row being scanned
	int                enforced   = 0;       // #entities in current batch
	int                batch_size = 10000;   // #entities to enforce in one go

	while(holds) {
		// lock graph for reading
		Graph_AcquireReadLock(g);

		// constraint state changed, abort enforcement
		// this can happen if for example the following sequance is issued:
		// 1. CREATE CONSTRAINT...
		// 2. DELETE CONSTRAINT...
		if(Constraint_PendingChanges(c) > 1) {
			// TODO: Log?
			break;
		}

		const RG_Matrix m = Graph_GetLabelMatrix(g, l);
		ASSERT(m != NULL);

		// reset number of enforce nodes in batch
		enforced = 0;

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
void Constraint_EnforceEdges
(
	Constraint c,
	Graph *g
) {
	GrB_Info info;
	RG_MatrixTupleIter it = {0};

	// TODO: change to RelationID
	LabelID   r            = c->lbl;  // edge relationship type ID
	bool      holds        = true;    // constraint holds
	EntityID  src_id       = 0;       // current processed row idx
	EntityID  dest_id      = 0;       // current processed column idx
	EntityID  edge_id      = 0;       // current processed edge id
	EntityID  prev_src_id  = 0;       // last processed row idx
	EntityID  prev_dest_id = 0;       // last processed column idx
	int       enforced     = 0;       // number of entities enforced in current batch
	int       batch_size   = 1000;    // max number of entities to enforce in one go

	while(holds) {
		// lock graph for reading
		Graph_AcquireReadLock(g);

		// constraint state changed, abort enforcement
		// this can happen if for example the following sequance is issued:
		// 1. CREATE CONSTRAINT...
		// 2. DELETE CONSTRAINT...
		if(Constraint_PendingChanges(c) > 1) {
			// TODO: Log?
			break;
		}

		// reset number of enforced edges in batch
		enforced     = 0;
		prev_src_id  = src_id;
		prev_dest_id = dest_id;

		// fetch relation matrix
		const RG_Matrix m = Graph_GetRelationMatrix(g, r, false);
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
			e.relationID = r;

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
	Constraint *c
) {
	ASSERT(c != NULL);

	Constraint _c = *c;

    rm_free(_c->attrs);
	rm_free(_c->attr_names);
    rm_free(_c);

	*c = NULL;
}

