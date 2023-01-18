#include "RG.h"
#include "constraint.h"
#include "../util/arr.h"
#include "../graph/entities/attribute_set.h"
#include "../index/index.h"
#include "redisearch_api.h"
#include "../index/index.h"
#include "../schema/schema.h"
#include "value.h"
#include "../index/indexer.h"
#include "../query_ctx.h"
#include <stdatomic.h>

// constraint enforcement callback function
typedef bool (EnforcementCB)(const Constraint *c, const GraphEntity * e);

// opaque structure representing a constraint
typedef struct _Constraint {
	Schema *schema;                // constraint schema
	ConstraintType t;              // constraint type
	EnforcementCB enforce;         // enforcement function
    Attribute_ID *attributes;      // sorted array of attr IDs
    ConstraintStatus status;       // constraint status
    uint _Atomic pending_changes;  // number of pending changes
} _Constraint;

// enforces unique constraint on given entity
static bool Constraint_EnforceUniqueEntity
(
	const Constraint c,   // constraint to enforce
	const GraphEntity *e  // enforced entity
) {
	// validations
	ASSERT(c != NULL);
	ASSERT(e != NULL);

	bool  res  = false;  // return value none-optimistic
	Index *idx = Schema_GetIndex(c->s, _, _);

	//--------------------------------------------------------------------------
	// construct a RediSearch query locating entity
	//--------------------------------------------------------------------------

	// TODO: prefer to have the RediSearch query "template" constructed
	// once and reused for each entity

    SIType t;
    SIValue *v;
    RSQNode *node = NULL;  // RediSearch query node

    // convert constraint into a RediSearch query
    RSQNode *rs_query_node = RediSearch_CreateIntersectNode(idx, false);
    ASSERT(rs_query_node != NULL);

    for(uint i = 0; i < array_len(c->attributes); i++) {
		Attribute_ID attr_id = c->attributes + i;

		// get current attribute from entity
        v = AttributeSet_Get(attributes, attr_id);
		if(v == ATTRIBUTE_NOTFOUND) {
			// entity satisfies constraint in a vacuous truth manner
			// TODO: clean up...
			res = true;
			break;
		}

		// create RediSearch query node according to entity attr type
        t = SI_TYPE(*v);

	    if(!(t & SI_INDEXABLE)) {
			assert("check with Neo implementations" && false);
	    	// none indexable type, consult with the none indexed field
	    	node = RediSearch_CreateTagNode(idx, INDEX_FIELD_NONE_INDEXED);
	    	RSQNode *child = RediSearch_CreateTokenNode(idx,
	    			INDEX_FIELD_NONE_INDEXED, field);

	    	RediSearch_QueryNodeAddChild(node, child);
	    } else if(t == T_STRING) {
            node = RediSearch_CreateTagNode(idx, field);
            RSQNode *child = RediSearch_CreateTokenNode(idx, field, v->stringval);
	    	RediSearch_QueryNodeAddChild(node, child);
        } else {
            ASSERT(t & SI_NUMERIC || t == T_BOOL);
		    double d = SI_GET_NUMERIC((*v));
            node = RediSearch_CreateNumericNode(idx, field, d, d, true, true);
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
    RSResultsIterator *iter = RediSearch_GetResultsIterator(rs_query_node, idx);
    const void* ptr = RediSearch_ResultsIteratorNext(iter, idx, NULL);
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

// create a new constraint
Constraint Constraint_New
(
	Attribute_ID *attrs,  // enforced attributes
	uint n_attr,          // number of attributes
	const Schema *s,      // constraint schema
	ConstraintType t      // constraint type
) {
	// TODO: support CT_MANDATORY
	ASSERT(t == CT_UNIQUE);

    Constraint c = rm_malloc(sizeof(_Constraint));

	// introduce constraint attributes
	c->attributes = array_newlen(Attribute_ID,  n_fields);
    memcpy(c->attributes, fields, sizeof(Attribute_ID) * id_count);

	// initialize constraint
	c->t               = t;
	c->schema          = s;
	c->status          = CT_PENDING;
	c->pending_changes = ATOMIC_VAR_INIT(0);

	// set enforce function pointer
	if(t == CT_UNIQUE) {
		c->enforce = Constraint_EnforceUniqueEntity;
	} else {
		c->enforce = Constraint_EnforceMandatory;
	}

	return c;
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
	const Constraint c  // constraint from which to get attributes
) {
	ASSERT(c != NULL);

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
	Constraint *c,
	const GraphEntity *e,
) {
	ASSERT(c != NULL);
	ASSERT(e != NULL);

	return c->enforce(e);
}

// executed under write lock
void Constraint_Drop_Index
(
	Constraint c,
	GraphContext *gc,
	bool should_drop_constraint
) {
	// constraint was not satisfied, free it and remove it's index

	SchemaType schema_type = (c->entity_type == GETYPE_NODE) ? SCHEMA_NODE : SCHEMA_EDGE;
	Schema *s = GraphContext_GetSchema((GraphContext*)gc, c->label, schema_type);
	ASSERT(s);

    Index idx = Schema_GetIndex(s, &(c->attributes[0].id), IDX_EXACT_MATCH);
    ASSERT(idx != NULL);
	for(int i = 1; i < array_len(c->attributes); i++) {
        ASSERT(idx == Schema_GetIndex(s, &(c->attributes[i].id), IDX_EXACT_MATCH));
    }

    bool index_changed = false;
	// remove all index fields
	for(int i = 0; i < array_len(c->attributes); i++) {
		int res = GraphContext_DeleteIndex((GraphContext*)gc, schema_type, c->label, c->attributes[i].attribute_name,
			IDX_EXACT_MATCH, true);
		ASSERT(res != INDEX_FAIL); // index should exist

		if(res == INDEX_OK) {
            index_changed = true;
        }
	}

    if(index_changed) {
        if(Index_FieldsCount(idx) > 0) {
            Indexer_PopulateIndexOrConstraint((GraphContext*)gc, idx, NULL);
        } else {
            Indexer_DropIndexOrConstraint(idx, NULL);
        }
    }

    if(should_drop_constraint) {
        Constraint_IncPendingChanges(c);
        Indexer_DropIndexOrConstraint(NULL, c);
    }
}

bool Has_Constraint_On_Attribute(Constraint *constraints, Attribute_ID attr_id) {
    for(int i = 0; i < array_len(constraints); i++) {
        Constraint c = constraints[i];
        for(int j = 0; j < array_len(c->attributes); j++) {
            if(c->attributes[j].id == attr_id) {
                return true;
            }
        }
    }

    return false;
}

typedef enum {
	CT_CREATE,
	CT_DELETE
} ConstraintOp;


// returns constraint graph entity type
GraphEntityType Constraint_GraphEntityType
(
	const Constraint c
) {
	ASSERT(c != NULL);

	return c->entity_type;
}

void Constraint_free(Constraint c) {
    for(int i = 0; i < array_len(c->attributes); i++) {
        rm_free(c->attributes[i].attribute_name);
    }
    array_free(c->attributes);
    rm_free(c->label);
    rm_free(c);
}
