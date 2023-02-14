/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "constraint.h"
#include "../index/index.h"
#include "redisearch_api.h"
#include "../graph/entities/attribute_set.h"

#include <stdatomic.h>

// opaque structure representing a constraint
typedef struct _UniqueConstraint {
	uint n_attr;                   // number of fields
	ConstraintType t;              // constraint type
	EnforcementCB enforce;         // enforcement function
	int lbl;                       // enforced label/relationship-type
    Attribute_ID *attrs;           // enforced attributes
	const char **attr_names;       // enforced attribute names
    ConstraintStatus status;       // constraint status
    uint _Atomic pending_changes;  // number of pending changes
	EntityType et;                 // entity type
	Index idx                      // supporting index
} _UniqueConstraint;

// enforces unique constraint on given entity
// returns true if entity confirms with constraint false otherwise
static bool Constraint_EnforceUniqueEntity
(
	UniqueConstraint c,   // constraint to enforce
	const GraphEntity *e  // enforced entity
) {
	// validations
	ASSERT(c != NULL);
	ASSERT(e != NULL);

	bool    res     = false;  // return value none-optimistic
	Index   idx     = c->idx;
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

    for(uint i = 0; i < array_len(c->attrs); i++) {
		Attribute_ID attr_id = c->attrs[i];
		const char *field = c->attr_names[i];

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

Constraint Constraint_UniqueNew
(
	LabelID l,                // label/relation ID
	Attribute_ID *fields,     // enforced fields
	const char **attr_names,  // enforced attribute names
	uint n_fields,            // number of fields
	EntityType et,            // entity type
	Index idx                 // index
) {
    UniqueConstraint c = rm_malloc(sizeof(_UniqueConstraint));

	// introduce constraint attributes
	c->attrs = rm_malloc(sizeof(Attribute_ID) * n_fields);
    memcpy(c->attrs, fields, sizeof(Attribute_ID) * n_fields);

	c->attr_names = rm_malloc(sizeof(char*) * n_fields);
    memcpy(c->attr_names, attr_names, sizeof(char*) * n_fields);

	// initialize constraint
	c->t               = CT_UNIQUE;
	c->et              = et;
	c->lbl             = l;
	c->idx             = idx;
	c->status          = CT_PENDING;
	c->n_attr          = n_fields;
	c->enforce         = Constraint_EnforceUniqueEntity;
	c->pending_changes = ATOMIC_VAR_INIT(0);

	return (Constraint)c;
}

