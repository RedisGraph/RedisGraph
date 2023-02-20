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
struct _UniqueConstraint {
	uint n_attr;                   // number of fields
	ConstraintType t;              // constraint type
	EnforcementCB enforce;         // enforcement function
	int lbl;                       // enforced label/relationship-type
    Attribute_ID *attrs;           // enforced attributes
	const char **attr_names;       // enforced attribute names
    ConstraintStatus status;       // constraint status
    uint _Atomic pending_changes;  // number of pending changes
	GraphEntityType et;            // entity type
	Index idx;                     // supporting index
};

typedef struct _UniqueConstraint* UniqueConstraint;

// enforces unique constraint on given entity
// returns true if entity confirms with constraint false otherwise
static bool Constraint_EnforceUniqueEntity
(
	const Constraint c,   // constraint to enforce
	const GraphEntity *e  // enforced entity
) {
	// validations
	ASSERT(c != NULL);
	ASSERT(e != NULL);

	UniqueConstraint _c = (UniqueConstraint)c;

	Index   idx     = _c->idx;
	bool    holds   = false;  // return value none-optimistic
	RSIndex *rs_idx = Index_RSIndex(idx);

	//--------------------------------------------------------------------------
	// construct a RediSearch query locating entity
	//--------------------------------------------------------------------------

	// TODO: prefer to have the RediSearch query "template" constructed
	// once and reused for each entity

    SIType t;
    SIValue *v;
    uint i = 0;
    RSQNode *node = NULL;  // RediSearch query node
    RSQNode *root = NULL;  // root of RediSearch query tree
	RSQNode *nodes[_c->n_attr];
    RSResultsIterator *iter = NULL;
	const AttributeSet attributes = GraphEntity_GetAttributes(e);

    // create a RediSearch query
    for(i = 0; i < _c->n_attr; i++) {
		Attribute_ID attr_id = _c->attrs[i];
		const char *field = _c->attr_names[i];

		// get current attribute from entity
        v = AttributeSet_Get(attributes, attr_id);
		if(v == ATTRIBUTE_NOTFOUND) {
			// entity satisfies constraint in a vacuous truth manner
			holds = true;
			goto cleanup;
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
            RSQNode *child = RediSearch_CreateTagTokenNode(rs_idx, v->stringval);
	    	RediSearch_QueryNodeAddChild(node, child);
        } else {
            ASSERT(t & SI_NUMERIC || t == T_BOOL);
		    double d = SI_GET_NUMERIC((*v));
            node = RediSearch_CreateNumericNode(rs_idx, field, d, d, true, true);
        }

        ASSERT(node != NULL);
		nodes[i] = node;
    }

	//--------------------------------------------------------------------------
	// cancat filters
	//--------------------------------------------------------------------------

	root = node;
	if(_c->n_attr > 1) {
		// intersection query node
		root = RediSearch_CreateIntersectNode(rs_idx, false);
		for(uint i = 0; i < _c->n_attr; i++) {
			RediSearch_QueryNodeAddChild(root, nodes[i]);
		}
	}

	//--------------------------------------------------------------------------
	// query RediSearch index
	//--------------------------------------------------------------------------

	// constraint holds if there are no duplicates, 1 or less index matches
	int matches = 0;
    iter = RediSearch_GetResultsIterator(root, rs_idx);
	for(; matches < 2; matches++) {
		if(RediSearch_ResultsIteratorNext(iter, rs_idx, NULL) == NULL) {
			break;
		}
	}
	holds = (matches <= 1);

cleanup:
	if(iter != NULL) {
		RediSearch_ResultsIteratorFree(iter);
	} else {
		RediSearch_QueryNodeFree(root);
	}

    return holds;
}

Constraint Constraint_UniqueNew
(
	LabelID l,                // label/relation ID
	Attribute_ID *fields,     // enforced fields
	const char **attr_names,  // enforced attribute names
	uint n_fields,            // number of fields
	GraphEntityType et,       // entity type
	Index idx                 // index
) {
    UniqueConstraint c = rm_malloc(sizeof(struct _UniqueConstraint));

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

