/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "../query_ctx.h"
#include "../graph/graph_hub.h"
#include "../undo_log/undo_log.h"
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
		*ct = CT_EXISTS;
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
	RedisModuleCtx *ctx,    // redis module context
	RedisModuleString *key, // graph key to operate on
	ConstraintType ct,      // constraint type
	GraphEntityType et,     // entity type
	const char *lbl,        // label / rel-type
	uint16_t n,             // properties count
	const char **props      // properties
) {
	bool res = true;

	// get or create graph
	GraphContext *gc = GraphContext_Retrieve(ctx, key, false, true);
	if(gc == NULL) {
		return false;	
	}

	Graph *g = GraphContext_GetGraph(gc);

	// acquire graph write lock
	Graph_AcquireWriteLock(gc->g);

	//--------------------------------------------------------------------------
	// convert attribute name to attribute ID
	//--------------------------------------------------------------------------

	Attribute_ID attr_ids[n];
	for(uint i = 0; i < n; i++) {
		attr_ids[i] = FindOrAddAttribute(gc, props[i]);
	}

	//--------------------------------------------------------------------------
	// make sure schema exists
	//--------------------------------------------------------------------------

	SchemaType st = (et == GETYPE_NODE) ? SCHEMA_NODE : SCHEMA_EDGE;
	Schema *s = GraphContext_GetSchema(gc, lbl, st);
	if(s == NULL) {
		s = AddSchema(gc, lbl, st);
	}
	int s_id = Schema_GetID(s);

	//--------------------------------------------------------------------------
	// check for duplicates
	//--------------------------------------------------------------------------

	// sort the properties for an easy comparison later
	bool dups = false;
	qsort(attr_ids, n, sizeof(Attribute_ID), _cmp_Attribute_ID);
	for(uint i = 0; i < n - 1; i++) {
		if(attr_ids[i] != attr_ids[i+1]) {
			dups = true;
			break;
		}
	}

	// duplicates found, faile operation
	if(dups) {
		res = false;
		goto cleanup;
	}

	// re-construct attribute IDs array
	// must be aligned with attribute names array
	for(uint i = 0; i < n; i++) {
		attr_ids[i] = GraphContext_GetAttributeID(gc, props[i]);
	}

	//--------------------------------------------------------------------------
	// check if constraint already exists
	//--------------------------------------------------------------------------

	Constraint c = Schema_GetConstraint(s, ct, attr_ids, n);

	if(c != NULL) {
		// constraint already exists
		if(Constraint_GetStatus(c) != CT_FAILED) {
			// constraint is either operational or being constructed
			res = false;
			goto cleanup;
		} else {
			// previous constraint creation had failed
			// remove constrain from schema
			bool constraint_removed = Schema_RemoveConstraint(s, c);
			ASSERT(constraint_removed == true);

			// free failed constraint
			Constraint_Free(&c);
		}
	}
	
	//--------------------------------------------------------------------------
	// create constraint
	//--------------------------------------------------------------------------

	if(ct == CT_UNIQUE) {
		// unique constraint requires the existance of an exact match index
		// to support the constraint
		Index idx = GraphContext_GetIndexByID(gc, s_id, attr_ids,
				IDX_EXACT_MATCH, st);

		// index missing
		if(idx == NULL) {
			res = false;
			goto cleanup;
		}

		c = Constraint_UniqueNew(s_id, attr_ids, props, n, et, idx);
	} else {
		c = Constraint_ExistsNew(s_id, attr_ids, props, n, et);
	}

	// add constraint to schema
	Schema_AddConstraint(s, c);

cleanup:

	// operation failed perform clean up
	if(res == false) {
		UndoLog undolog = QueryCtx_GetUndoLog();
		UndoLog_Rollback(undolog);
	}

	// release graph R/W lock
	Graph_ReleaseLock(g);

	// constraint already exists
	if(res == false) { 
		// decrease graph reference count
		GraphContext_DecreaseRefCount(gc);

		// TODO: give additional information to caller
		RedisModule_ReplyWithError(ctx, "Constraint creation failed");
	} else {
		// constraint creation succeeded, enforce constraint
		Constraint_Enforce(c, g);
	}

	return res;
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

