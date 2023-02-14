/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "../graph/graphcontext.h"
#include "constraint/constraint.h"

// constraint operation
typedef enum {
	CT_CREATE,  // create constraint
	CT_DELETE   // delete constraint
} ConstraintOp;

static int Constraint_Parse
(
	RedisModuleCtx *ctx,
	RedisModuleString **argv,
	int argc,
	RedisModuleString **graph_name,
	ConstraintOp *op,
	ConstraintType *ct,
	GraphEntityType *type,
	const char **label,
	long long *prop_count,
	RedisModuleString ***props
) {
	//--------------------------------------------------------------------------
	// get graph name
	//--------------------------------------------------------------------------

	*graph_name = *argv++;

	//--------------------------------------------------------------------------
	// get constraint operation CREATE/DEL
	//--------------------------------------------------------------------------

	const char *token = RedisModule_StringPtrLen(*argv++, NULL);
	if(strcasecmp(token, "CREATE") == 0) {
		*op = CT_CREATE;
	} else if(strcasecmp(token, "DEL") == 0) {
		*op = CT_DELETE;
	} else {
		RedisModule_ReplyWithError(ctx, "Invalid constraint operation");
		return REDISMODULE_ERR;
	}

	//--------------------------------------------------------------------------
	// get constraint type UNIQUE/MANDATORY
	//--------------------------------------------------------------------------

	token = RedisModule_StringPtrLen(*argv++, NULL);
	if(strcasecmp(token, "UNIQUE") == 0) {
		*ct = CT_UNIQUE;
	} else if(strcasecmp(token, "MANDATORY") == 0) {
		*ct = CT_MANDATORY;
	} else {
		RedisModule_ReplyWithError(ctx, "Invalid constraint type");
		return REDISMODULE_ERR; //currently only unique constraint is supported
	}

	//--------------------------------------------------------------------------
	// extract entity type NODE/EDGE
	//--------------------------------------------------------------------------

	token = RedisModule_StringPtrLen(*argv++, NULL);
	if(strcasecmp(token, "LABEL") == 0) {
		*type = GETYPE_NODE;
	} else if(strcasecmp(token, "RELTYPE") == 0) {
		*type = GETYPE_EDGE;
	} else {
		RedisModule_ReplyWithError(ctx, "Invalid constraint entity type");
		return REDISMODULE_ERR;
	}

	//--------------------------------------------------------------------------
	// extract label/relationship-type
	//--------------------------------------------------------------------------

	*label = RedisModule_StringPtrLen(*argv++, NULL);

	//--------------------------------------------------------------------------
	// extract properties
	//--------------------------------------------------------------------------

	token = RedisModule_StringPtrLen(*argv++, NULL);
	if(strcasecmp(token, "PROPERTIES") == 0) {
		if (RedisModule_StringToLongLong(*argv++, prop_count) != REDISMODULE_OK
				|| *prop_count < 1) {
			RedisModule_ReplyWithError(ctx, "Invalid property count");
			return REDISMODULE_ERR;
		}
	} else {
		RedisModule_ReplyWithError(ctx, "7th arg isn't PROPERTIES");
		return REDISMODULE_ERR;
	}

	// expecting last property to be the last command argument
	if(argc - 8 != *prop_count) {
		RedisModule_ReplyWithError(ctx, "Number of properties doesn't match property count");
		return REDISMODULE_ERR;
	}

	*props = argv;

	return REDISMODULE_OK;
}

// GRAPH.CONSTRAIN <key> DEL UNIQUE/MANDATORY LABEL/RELTYPE label PROPERTIES prop_count prop0, prop1...
static int _Constraint_Delete
(
	RedisModuleCtx *ctx,
	RedisModuleString *key,
	ConstraintType ct,
	GraphEntityType entity_type,
	const char *label,
	uint prop_count,
	const char **props
) {
	int rv = REDISMODULE_OK;  // optimistic
	Attribute_ID fields[prop_count];

	// get graph
	GraphContext *gc = GraphContext_Retrieve(ctx, key, false, false);
	if(!gc) {
		// graph doesn't exists
		return false;
	}

	// acquire graph write lock
	Graph_AcquireWriteLock(gc->g);

	// determine schema type
	SchemaType st = (entity_type == GETYPE_NODE) ? SCHEMA_NODE : SCHEMA_EDGE;

	// try to get schema
	Schema *s = GraphContext_GetSchema(gc, label, st);
	if(s == NULL) {
		RedisModule_ReplyWithError(ctx, "Unable to drop constraint on, no such constraint.");
		rv = REDISMODULE_ERR;
		goto cleanup;
	}

	for(uint i = 0; i < prop_count; i++) {
		const char *prop = props[i];

		// try to get property ID
		Attribute_ID id = GraphContext_GetAttributeID(gc, prop);

		// propery missing, reply with an error and return
		if(id == ATTRIBUTE_ID_NONE) {
			RedisModule_ReplyWithError(ctx, "Unable to drop constraint on, no such constraint.");
			rv = REDISMODULE_ERR;
			goto cleanup;
		}

		fields[i] = id;
	}

	// try to get constraint
	// TODO: accept constraint type from user
	Constraint c = Schema_GetConstraint(s, CT_UNIQUE, fields, prop_count);
	if(c == NULL) {
		RedisModule_ReplyWithError(ctx, "Unable to drop constraint on, no such constraint.");
		rv = REDISMODULE_ERR;
		goto cleanup;
	}

	Schema_RemoveConstraint(s, c);

cleanup:
	// release graph R/W lock
	Graph_ReleaseLock(gc->g);

	// decrease graph reference count
	GraphContext_DecreaseRefCount(gc);

	return rv;
}

static inline int _cmp_Attribute_ID
(
	const void *a,
	const void *b
) {
	const Attribute_ID *_a = a;
	const Attribute_ID *_b = b;
	return *_a - *_b;
}


// GRAPH.CONSTRAIN <key> CREATE UNIQUE/MANDATORY LABEL/RELTYPE label PROPERTIES prop_count prop0, prop1...
static bool _Constraint_Create
(
	RedisModuleCtx *ctx,          // redis module context
	RedisModuleString *key,       // graph key to operate on
	ConstraintType ct,            // constraint type
	GraphEntityType entity_type,  // entity type
	const char *label,            // label / rel-type
	long long prop_count,         // properties count
	const char **props            // properties
) {

	// get or create graph
	GraphContext *gc = GraphContext_Retrieve(ctx, key, false, true);
	if(gc == NULL) {
		return false;	
	}

	//--------------------------------------------------------------------------
	// convert attribute name to attribute ID
	//--------------------------------------------------------------------------

	Attribute_ID attr_ids[prop_count];
	for(uint i = 0; i < prop_count; i++) {
		attr_ids[i] = FindOrAddAttribute(gc, props[i]);
	}

	//--------------------------------------------------------------------------
	// check for duplicates
	//--------------------------------------------------------------------------

	// sort the properties for an easy comparison later
	bool duplicates = false;
	qsort(attr_ids, prop_count, sizeof(Attribute_ID), _cmp_Attribute_ID);
	for(uint i = 0; i < fields_count - 1; i++) {
		if(attr_ids[i] != attr_ids[i+1]) {
			duplicates = true;
		}
	}

	if(duplicates) {
		// TODO: perform rollback
		return false;
	}

	Graph *g = GraphContext_GetGraph(gc);

	// determine schema type
	SchemaType st = (entity_type == GETYPE_NODE) ? SCHEMA_NODE : SCHEMA_EDGE;

	// acquire graph write lock
	Graph_AcquireWriteLock(gc->g);

	// TODO: remove duplicates.

	Constraint c = NULL;
	if(ct == CT_UNIQUE) {
		c = Constraint_UniqueNew();
		c = GraphContext_AddConstraint(gc, ct, st, label, props, prop_count);
	} else {
		c = Constraint_ExistsNew();
	}

	// TODO: add constraint to graphcontext / schema

	// release graph R/W lock
	Graph_ReleaseLock(g);

	// constraint already exists
	if(c == NULL) { 
		// decrease graph reference count
		GraphContext_DecreaseRefCount(gc);

		RedisModule_ReplyWithError(ctx, "Constraint already exists");
	} else {
		Constraint_Enforce(c, g);
	}

	return c != NULL;
}

// command handler for GRAPH.CONSTRAIN command
// GRAPH.CONSTRAIN <key> CREATE UNIQUE/MANDATORY LABEL/RELTYPE label PROPERTIES prop_count prop0, prop1...
// GRAPH.CONSTRAIN <key> DEL UNIQUE/MANDATORY LABEL/RELTYPE label PROPERTIES prop_count prop0, prop1...
int Graph_Constraint
(
	RedisModuleCtx *ctx,
	RedisModuleString **argv,
	int argc
) {
	if(argc < 9) {
		return RedisModule_WrongArity(ctx);
	}

	ConstraintOp op;
	ConstraintType ct;
	const char *label;
	long long prop_count;
	RedisModuleString **props;
	GraphEntityType entity_type;
	RedisModuleString *key_name;

	//--------------------------------------------------------------------------
	// parse command arguments
	//--------------------------------------------------------------------------

	int res = Constraint_Parse(ctx, argv+1, argc-1, &key_name, &op, &ct,
			&entity_type, &label, &prop_count, &props);

	// command parsing error, abort
	if(res != REDISMODULE_OK) {
		return REDISMODULE_ERR;
	}

	// extract constraint properties
	const char *props_cstr[prop_count];
	for(int i = 0; i < prop_count; i++) {
		props_cstr[i] = RedisModule_StringPtrLen(props[i], NULL);
	}

	// determine schema type
	SchemaType schema_type;
	if(entity_type == GETYPE_NODE) {
		schema_type = SCHEMA_NODE;
	} else {
		schema_type = SCHEMA_EDGE;
	}

	bool success = false;
	GraphContext *gc = NULL;

	if(op == CT_CREATE) {
		// try to create constraint
		success = _Constraint_Create(ctx, key_name, ct, entity_type, label,
				prop_count, props_cstr);
	} else {
		// CT_DELETE
		success = _Constraint_Delete(ctx, key_name, ct, entity_type, label,
				prop_count, props_cstr);
	}

	if(success == true) {
		RedisModule_ReplyWithSimpleString(ctx, "OK");
		RedisModule_ReplicateVerbatim(ctx);
		return REDISMODULE_OK;
	}

	return REDISMODULE_ERR;    
}

