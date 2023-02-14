/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

// opaque structure representing a constraint
typedef struct _ExistsConstraint {
	LabelID l;                     // label/relation ID
	uint n_attr;                   // number of fields
	ConstraintType t;              // constraint type
	EnforcementCB enforce;         // enforcement function
    Attribute_ID *attrs;           // enforced attributes
	const char **attr_names;       // enforced attribute names
    ConstraintStatus status;       // constraint status
    uint _Atomic pending_changes;  // number of pending changes
	EntityType et;                 // entity type
} _ExistsConstraint;

// enforces mandatory constraint on given entity
static bool Constraint_EnforceMandatory
(
	Constraint c,         // constraint to enforce
	const GraphEntity *e  // enforced entity
) {
	// TODO: implement
	return false;
}

Constraint Constraint_ExistsNew
(
	LabelID l,                // label/relation ID
	Attribute_ID *fields,     // enforced fields
	const char **attr_names,  // enforced attribute names
	uint n_fields,            // number of fields
	EntityType et,            // entity type
) {
    ExistsConstraint c = rm_malloc(sizeof(_ExistsConstraint));

	// introduce constraint attributes
	c->attrs = rm_malloc(sizeof(Attribute_ID) * n_fields);
    memcpy(c->attrs, fields, sizeof(Attribute_ID) * n_fields);

	c->attr_names = rm_malloc(sizeof(char*) * n_fields);
    memcpy(c->attr_names, attr_names, sizeof(char*) * n_fields);

	// initialize constraint
	c->l               = l;
	c->t               = CT_EXISTS;
	c->et              = et;
	c->status          = CT_PENDING;
	c->n_attr          = n_fields;
	c->enforce         = Constraint_EnforceExists;
	c->pending_changes = ATOMIC_VAR_INIT(0);

	return c;
}

