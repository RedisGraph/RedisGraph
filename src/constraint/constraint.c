/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "value.h"
#include "constraint.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "redisearch_api.h"
#include "../index/index.h"
#include "../schema/schema.h"
#include "../util/thpool/pools.h"
#include "../graph/entities/attribute_set.h"

#include <stdatomic.h>

// constraint enforcement callback function
typedef bool (*EnforcementCB)(const Constraint *c, const GraphEntity * e);

// opaque structure representing a constraint
typedef struct _Constraint {
	uint n_fields;                 // number of fields
	const Schema *schema;          // constraint schema
	ConstraintType t;              // constraint type
	EnforcementCB enforce;         // enforcement function
    Attribute_ID *attributes;      // sorted array of attr IDs
    ConstraintStatus status;       // constraint status
    uint _Atomic pending_changes;  // number of pending changes
} _Constraint;

// constraint enforce context
typedef struct ConstraintEnforceCtx {
	Graph *g;            // graph object
	const Constraint c;  // constraint to enforce
} ConstraintEnforceCtx;

// enforces mandatory constraint on given entity
static bool Constraint_EnforceMandatory
(
	const Constraint c,   // constraint to enforce
	const GraphEntity *e  // enforced entity
) {
	// TODO: implement
	return false;
}

// enforces unique constraint on given entity
static bool Constraint_EnforceUniqueEntity
(
	const Constraint c,   // constraint to enforce
	const GraphEntity *e  // enforced entity
) {
	// validations
	ASSERT(c != NULL);
	ASSERT(e != NULL);

	bool     res    = false;  // return value none-optimistic
	Index   idx     = Schema_GetIndex(c->schema, NULL, IDX_EXACT_MATCH);
	RSIndex *rs_idx = Index_RSIndex(idx);

	//--------------------------------------------------------------------------
	// construct a RediSearch query locating entity
	//--------------------------------------------------------------------------

	// TODO: prefer to have the RediSearch query "template" constructed
	// once and reused for each entity

    SIType t;
    SIValue *v;
    RSQNode *node = NULL;  // RediSearch query node
	const AttributeSet attributes = GraphEntity_GetAttributes(e);

    // convert constraint into a RediSearch query
    RSQNode *rs_query_node = RediSearch_CreateIntersectNode(rs_idx, false);
    ASSERT(rs_query_node != NULL);

    for(uint i = 0; i < array_len(c->attributes); i++) {
		Attribute_ID attr_id = c->attributes[i];

		// get current attribute from entity
        v = AttributeSet_Get(attributes, attr_id);
		if(v == ATTRIBUTE_NOTFOUND) {
			// entity satisfies constraint in a vacuous truth manner
			RediSearch_QueryNodeFree(rs_query_node);
			res = true;
			break;
		}

		// create RediSearch query node according to entity attr type
        t = SI_TYPE(*v);
		// TODO: waste full!
		const char *field = GraphContext_GetAttributeString(gc, attr_id);

	    if(!(t & SI_INDEXABLE)) {
			assert("check with Neo implementations" && false);
	    	// none indexable type, consult with the none indexed field
	    	node = RediSearch_CreateTagNode(rs_idx, INDEX_FIELD_NONE_INDEXED);
	    	RSQNode *child = RediSearch_CreateTokenNode(rs_idx,
	    			INDEX_FIELD_NONE_INDEXED, field);

	    	RediSearch_QueryNodeAddChild(node, child);
	    } else if(t == T_STRING) {
            node = RediSearch_CreateTagNode(rs_idx, field);
            RSQNode *child = RediSearch_CreateTokenNode(rs_idx, field, v->stringval);
	    	RediSearch_QueryNodeAddChild(node, child);
        } else {
            ASSERT(t & SI_NUMERIC || t == T_BOOL);
		    double d = SI_GET_NUMERIC((*v));
            node = RediSearch_CreateNumericNode(rs_idx, field, d, d, true, true);
        }

        ASSERT(node != NULL);
		// TODO: validate that if there's only one child
		// there's no performance penalty
        RediSearch_QueryNodeAddChild(rs_query_node, node);
    }

	//--------------------------------------------------------------------------
	// query RediSearch index
	//--------------------------------------------------------------------------

	// TODO: it is ok for 'RediSearch_ResultsIteratorNext' to return NULL
	// in which case the enforced entity satisfies
    RSResultsIterator *iter = RediSearch_GetResultsIterator(rs_query_node, rs_idx);
    const void* ptr = RediSearch_ResultsIteratorNext(iter, rs_idx, NULL);
	RediSearch_ResultsIteratorFree(iter);

	// if ptr == NULL
	// then there's no pre-existing entity which conflicts with given entity
	// constaint holds!
	//
	// otherwise ptr != NULL
	// there's already an existing entity which conflicts with given entity
	// constaint does NOT hold!

    return (ptr == NULL);
}

static void _Constraint_EnforceNodes
(
	void *args
) {
	// check if constraint holds
	// scan through all entities governed by this constraint and enforce

	ASSERT(args != NULL);

	ConstraintEnforceCtx *ctx = (ConstraintEnforceCtx*)args;

	Graph      *g = ctx->g;
	Constraint  c = ctx->c;

	const Schema *s = c->s;

	Index idx = GraphContext_GetIndexByID(gc, Schema_GetID(s), NULL,
			IDX_EXACT_MATCH, SCHEMA_NODE);

	bool               holds      = true;   // constraint holds
	GrB_Index          rowIdx     = 0;      // current row being scanned
	int                enforced   = 0;      // #entities in current batch
	int                batch_size = 10000;  // #entities to enforce in one go
	RG_MatrixTupleIter it         = {0};    // matrix iterator

	while(holds && true) {
		// lock graph for reading
		Graph_AcquireReadLock(g);

		const RG_Matrix m = Graph_GetLabelMatrix(g, Schema_GetID(s));
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
			if(!Constraint_EnforceEntity(c, (GraphEntity*)&n)) {
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

static void _Constraint_EnforceEdges
(
	void *args
) {
	ASSERT(args != NULL);

	ConstraintEnforceCtx *ctx = (ConstraintEnforceCtx*)args;

	Graph      *g = ctx->g;
	const Constraint  c = ctx->c;

	const Schema *s = c->s;

	Index idx = Schema_GetIndex(s, NULL, IDX_EXACT_MATCH);

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
		const RG_Matrix m = Graph_GetRelationMatrix(g, Schema_GetID(s), false);
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
			e.relationID = Schema_GetID(s);

			if(SINGLE_EDGE(edge_id)) {
				Graph_GetEdge(g, edge_id, &e);
				if(!Constraint_EnforceEntity(c, (GraphEntity*)&e)) {
					holds = false;
					break;
				}
			} else {
				EdgeID *edgeIds = (EdgeID *)(CLEAR_MSB(edge_id));
				uint edgeCount = array_len(edgeIds);

				for(uint i = 0; i < edgeCount; i++) {
					edge_id = edgeIds[i];
					Graph_GetEdge(g, edge_id, &e);
					if(!Constraint_EnforceEntity(c, (GraphEntity*)&e)) {
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


// create a new constraint
Constraint Constraint_New
(
	Attribute_ID *fields,    // enforced fields
	uint n_fields,           // number of fields
	const struct Schema *s,  // constraint schema
	ConstraintType t         // constraint type
) {
	// TODO: support CT_MANDATORY
	ASSERT(t == CT_UNIQUE || t == CT_MANDATORY);

    Constraint c = rm_malloc(sizeof(struct _Constraint));

	// introduce constraint attributes
	c->attributes = rm_malloc(sizeof(Attribute_ID) * n_fields);
    memcpy(c->attributes, fields, sizeof(Attribute_ID) * n_fields);

	// initialize constraint
	c->t               = t;
	c->schema          = s;
	c->status          = CT_PENDING;
	c->n_fields        = n_fields;
	c->pending_changes = ATOMIC_VAR_INIT(0);

	// set enforce function pointer
	if(t == CT_UNIQUE) {
		c->enforce = Constraint_EnforceUniqueEntity;
	} else {
		c->enforce = Constraint_EnforceMandatory;
	}

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

// tries to enforce constraint
// will create indicies if required
// sets constraint status as pending
void Constraint_Enforce
(
	const Constraint c,  // constraint to enforce
	Graph *g
) {
	ASSERT(c  != NULL);
	ASSERT(g  != NULL);

	// mark constraint as pending
	Constraint_IncPendingChanges(c);

	// build enforce context
	ConstraintEnforceCtx *ctx = rm_malloc(sizeof(ConstraintEnforceCtx));
	ctx->c = c;
	ctx->g = g;

	if(Schema_GetType(c->schema) == SCHEMA_NODE) {
		ThreadPools_AddWorkReader(_Constraint_EnforceNodes, (void*)ctx);
	} else {
		ThreadPools_AddWorkReader(_Constraint_EnforceEdges, (void*)ctx);
	}
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
const Attribute_ID *Constraint_GetAttributes
(
	const Constraint c,  // constraint from which to get attributes
	uint *n              // length of returned array
) {
	ASSERT(c != NULL);
	ASSERT(n != NULL);

	*n = c->n_fields;
	return (const Attribute_ID*)c->attributes;
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

// enforce constraint on entity
// returns true if entity satisfies the constraint
// false otherwise
bool Constraint_EnforceEntity
(
	const Constraint c,   // constraint to enforce
	const GraphEntity *e  // enforced entity
) {
	ASSERT(c != NULL);
	ASSERT(e != NULL);

	return c->enforce(e);
}

// checks if constraint enforces attribute
bool Constraint_EnforceAttribute
(
	Constraint c,         // constraint to query
	Attribute_ID attr_id  // enforced attribute
) {
	for(int i = 0; i < c->n_fields; i++) {
		if(c->attributes[i] == attr_id) {
			return true;
		}
    }

    return false;
}

void Constraint_free
(
	Constraint c
) {
	ASSERT(c != NULL);

    rm_free(c->attributes);
    rm_free(c);
}

