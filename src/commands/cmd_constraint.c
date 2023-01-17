/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "constraint/constraint.h"

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

static int Constraint_Delete
(
	RedisModuleCtx *ctx,
	RedisModuleString *key,
	ConstraintType ct,
	GraphEntityType entity_type,
	const char *label,
	long long prop_count,
	const RedisModuleString **props
) {
	int rv = REDISMODULE_OK;  // optimistic

	// acquire graph write lock
	Graph_AcquireWriteLock(gc->g);

	// try to get schema
	SchemaType schema_type;
	if(entity_type == GETYPE_NODE) {
		schema_type = SCHEMA_NODE;
	} else {
		schema_type = SCHEMA_EDGE;
	}

	// try to get schema
	Schema *s = GraphContext_GetSchema(gc, label, schema_type);
	if(s == NULL) {
		RedisModule_ReplyWithError(ctx, "Trying to delete constraint from non existing label");
		rv = REDISMODULE_ERR;
		goto cleanup;
	}

	// init array of AttrInfo
	// one for each property marked for removal
	AttrInfo fields[prop_count];
	for(int i = 0; i < prop_count; i++) {
		const char *prop = props[i];

		// try to get property ID
		Attribute_ID id = GraphContext_GetAttributeID(gc, prop);

		// propery missing, reply with an error and return
		if(id == ATTRIBUTE_ID_NONE) {
			RedisModule_ReplyWithError(ctx, "Property name not found");
			rv = REDISMODULE_ERR;
			goto _out;
		}

		fields[i].id = id;
		fields[i].attribute_name = prop;
	}

	// try to get constraint
	Constraint c = Schema_GetConstraint(s, fields, prop_count);
	if(!c) {
		RedisModule_ReplyWithError(ctx, "Constraint not found");
		rv = REDISMODULE_ERR;
		goto _out;
	}

	Schema_RemoveConstraint(s, c);
	Constraint_Drop_Index(c, (struct GraphContext *)gc, true);
}

static int Constraint_Create
(
	RedisModuleCtx *ctx,
	RedisModuleString *key,
	ConstraintType ct,
	GraphEntityType entity_type,
	const char *label,
	long long prop_count,
	const RedisModuleString **props
) {
	int rv = REDISMODULE_OK;  // optimistic

	// get or create graph
	GraphContext *gc = GraphContext_Retrieve(ctx, key, false, true);
	if(!gc) {
		RedisModule_ReplyWithError(ctx, "Invalid graph name passed as argument");
		return REDISMODULE_ERR;
	}

	// acquire graph write lock
	Graph_AcquireWriteLock(gc->g);

	c = GraphContext_AddUniqueConstraint(gc, entity_type, label, props,
			prop_count);

	// constraint already exists
	if(c == NULL) { 
		RedisModule_ReplyWithError(ctx, "Constraint already exists");
		rv = REDISMODULE_ERR;
		goto cleanup;
	}

cleanup:
	// release graph R/W lock
	Graph_ReleaseLock(gc->g);

	// decrease graph reference count
	GraphContext_DecreaseRefCount(gc);
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
	RedisModuleString *rs_graph_name;

	int rv = REDISMODULE_OK;
	
	//--------------------------------------------------------------------------
	// parse command arguments
	//--------------------------------------------------------------------------

	int res = Constraint_Parse(ctx, argv+1, argc-1, &rs_graph_name,
			&entity_type, &label, &props, &prop_count, &op, &ct); 

	// command parsing error, abort
	if(res != REDISMODULE_OK) {
		return REDISMODULE_ERR;
	}

	// extract constraint properties
	const char *props_cstr[prop_count];
	for(int i = 0; i < prop_count; i++) {
		props_cstr[i] = RedisModule_StringPtrLen(props[i], NULL);
	}

	GraphContext *gc = GraphContext_Retrieve(ctx, rs_graph_name, false, false);
	if(!gc) {
		RedisModule_ReplyWithError(ctx, "Invalid graph name passed as argument");
		return REDISMODULE_ERR;
	}

	// determine schema type
	SchemaType schema_type;
	if(entity_type == GETYPE_NODE) {
		schema_type = SCHEMA_NODE;
	} else {
		schema_type = SCHEMA_EDGE;
	}

	// acquire graph write lock
	Graph_AcquireWriteLock(gc->g);
	Schema *s = GraphContext_GetSchema(gc, label, schema_type);
	if(op == CT_DELETE && !s) {
		RedisModule_ReplyWithError(ctx, "Trying to delete constraint from non existing label");
		rv = REDISMODULE_ERR;
		goto _out;
	}

	Index idx = NULL;
	Constraint c = NULL;

	if(op == CT_CREATE) {
		rv = Constraint_Create(ctx, key, ct, entity_type, label, prop_count,
				props_cstr);
	} else { // CT_DELETE
		rv = Constraint_Delete(ctx, key, ct, entity_type, label, prop_count,
				props_cstr);

	}

	if(rv == REDISMODULE_OK) {
		RedisModule_ReplyWithSimpleString(ctx, "OK");
		RedisModule_ReplicateVerbatim(ctx);
	}

	return rv;    
}

