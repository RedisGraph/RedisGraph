/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "proc_list_constraints.h"
#include "RG.h"
#include "../value.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../constraint/constraint.h"
#include "../schema/schema.h"
#include "../datatypes/map.h"
#include "../datatypes/array.h"

typedef struct {
	SIValue *out;               // outputs
	int node_schema_id;         // current node schema ID
	int edge_schema_id;         // current edge schema ID
    int constraint_id;          // current constraint ID
	ConstraintType type;        // current constraint type to retrieve
	GraphContext *gc;           // graph context
	SIValue *yield_type;        // yield constraint type
	SIValue *yield_label;       // yield constraint label
	SIValue *yield_properties;  // yield constraint properties
	SIValue *yield_entity_type; // yield constraint entity type
	SIValue *yield_status;      // yield constraint status
	SIValue *yield_info;        // yield info
} ConstraintsContext;

static void _process_yield
(
	ConstraintsContext *ctx,
	const char **yield
) {
	ctx->yield_type        = NULL;
	ctx->yield_info        = NULL;
	ctx->yield_label       = NULL;
	ctx->yield_status      = NULL;
	ctx->yield_language    = NULL;
	ctx->yield_stopwords   = NULL;
	ctx->yield_properties  = NULL;
	ctx->yield_entity_type = NULL;

	int idx = 0;
	for(uint i = 0; i < array_len(yield); i++) {
		if(strcasecmp("type", yield[i]) == 0) {
			ctx->yield_type = ctx->out + idx;
			idx++;
			continue;
		}

		if(strcasecmp("label", yield[i]) == 0) {
			ctx->yield_label = ctx->out + idx;
			idx++;
			continue;
		}

		if(strcasecmp("properties", yield[i]) == 0) {
			ctx->yield_properties = ctx->out + idx;
			idx++;
			continue;
		}

		if(strcasecmp("entitytype", yield[i]) == 0) {
			ctx->yield_entity_type = ctx->out + idx;
			idx++;
			continue;
		}

		if(strcasecmp("status", yield[i]) == 0) {
			ctx->yield_status = ctx->out + idx;
			idx++;
			continue;
		}

		if(strcasecmp("info", yield[i]) == 0) {
			ctx->yield_info = ctx->out + idx;
			idx++;
			continue;
		}
	}
}

// CALL db.constraints()
ProcedureResult Proc_ConstraintsInvoke
(
	ProcedureCtx *ctx,
	const SIValue *args,
	const char **yield
) {

	ASSERT(ctx   != NULL);
	ASSERT(args  != NULL);
	ASSERT(yield != NULL);

	// TODO: introduce invoke validation, similar to arithmetic expressions
	// expecting no arguments
	uint arg_count = array_len((SIValue *)args);
	if(arg_count != 0) return PROCEDURE_ERR;

	GraphContext *gc = QueryCtx_GetGraphCtx();

	ConstraintsContext *pdata = rm_malloc(sizeof(ConstraintsContext));

	pdata->gc             = gc;
	pdata->out            = array_new(SIValue, 8);
	pdata->type           = IDX_EXACT_MATCH;
	pdata->node_schema_id = GraphContext_SchemaCount(gc, SCHEMA_NODE) - 1;
	pdata->edge_schema_id = GraphContext_SchemaCount(gc, SCHEMA_EDGE) - 1;
    pdata->constraint_id  = 0;

	_process_yield(pdata, yield);

	ctx->privateData = pdata;

	return PROCEDURE_OK;
}

static bool _EmitConstraint
(
	ConstraintsContext *ctx,
	const Schema *s,
	ConstraintType type,
    int constraint_id
) {
	Constraint c = Schema_GetConstraints(s);
    ASSERT(c != NULL);
    if(array_len(c) <= constraint_id) return false;
    Constraint constraint = &c[constraint_id];

	//--------------------------------------------------------------------------
	// constraint entity type
	//--------------------------------------------------------------------------

	if(ctx->yield_entity_type != NULL) {
		if(s->type == SCHEMA_NODE) {
			*ctx->yield_entity_type = SI_ConstStringVal("NODE");
		} else {
			*ctx->yield_entity_type = SI_ConstStringVal("RELATIONSHIP");
		}
	}

	//--------------------------------------------------------------------------
	// constraint status
	//--------------------------------------------------------------------------

	if(ctx->yield_status != NULL) {
		if(constraint->status == CT_ACTIVE) {
			*ctx->yield_status = SI_ConstStringVal("OPERATIONAL");
		} else if (constraint->status == CT_PENDING) {
			*ctx->yield_status = SI_ConstStringVal("UNDER CONSTRUCTION");
		} else {
            *ctx->yield_status = SI_ConstStringVal("FAILED");
        }
	}

	//--------------------------------------------------------------------------
	// constraint type
	//--------------------------------------------------------------------------

	if(ctx->yield_type != NULL) {
		if(type == CT_UNIQUE) {
			*ctx->yield_type = SI_ConstStringVal("unique");
		} else {
			*ctx->yield_type = SI_ConstStringVal("mandatory");
		}
	}

	//--------------------------------------------------------------------------
	// constraint label
	//--------------------------------------------------------------------------

	if(ctx->yield_label) {
		*ctx->yield_label = SI_ConstStringVal((char *)Schema_GetName(s));
	}

	//--------------------------------------------------------------------------
	// constraint fields
	//--------------------------------------------------------------------------

	if(ctx->yield_properties) {
		uint fields_count        = array_len(c->attributes);
		const ConstAttrData *fields = Constraint_GetAttributes(c);
		*ctx->yield_properties   = SI_Array(fields_count);

		for(uint i = 0; i < fields_count; i++) {
			SIArray_Append(ctx->yield_properties,
						   SI_ConstStringVal((char *)fields[i].attribute_name));
		}
	}

	return true;
}

static SIValue *Schema_Step
(
	int *schema_id,
	SchemaType t,
	ConstraintsContext *pdata
) {
	Schema *s = NULL;

	// loop over all schemas from last to first
	while(*schema_id >= 0) {
		s = GraphContext_GetSchemaByID(pdata->gc, *schema_id, t);
		if(!Schema_HasConstraints(s)) {
			// no constraints found, continue to the next schema
			(*schema_id)--;
			continue;
		}

		// populate constraint data if one is found
		bool found = _EmitConstraint(pdata, s, pdata->type, pdata->constraint_id);

        if(found) {
            pdata->constraint_id++;
            return pdata->out;
        } else {
            pdata->constraint_id = 0;
            (*schema_id)--;
        }
	}

	return NULL;
}

SIValue *Proc_ConstraintsStep
(
	ProcedureCtx *ctx
) {
	ASSERT(ctx->privateData != NULL);

	SIValue *res;
	ConstraintsContext *pdata = ctx->privateData;

	res = Schema_Step(&pdata->node_schema_id, SCHEMA_NODE, pdata);
	if(res != NULL) return res;

	return Schema_Step(&pdata->edge_schema_id, SCHEMA_EDGE, pdata);
}

ProcedureResult Proc_ConstraintsFree
(
	ProcedureCtx *ctx
) {
	// clean up
	if(ctx->privateData) {
		ConstraintsContext *pdata = ctx->privateData;
		array_free(pdata->out);
		rm_free(pdata);
	}

	return PROCEDURE_OK;
}

ProcedureCtx *Proc_ConstraintsCtx() {
	void *privateData = NULL;
	ProcedureOutput output;
	ProcedureOutput *outputs = array_new(ProcedureOutput, 8);

	// constraint type (unique / mandatory)
	output = (ProcedureOutput) {
		.name = "type", .type = T_STRING
	};
	array_append(outputs, output);

	// constraint label
	output = (ProcedureOutput) {
		.name = "label", .type = T_STRING
	};
	array_append(outputs, output);

	// constraint properties
	output = (ProcedureOutput) {
		.name = "properties", .type = T_ARRAY
	};
	array_append(outputs, output);

	// constraint entity type (node / relationship)
	output = (ProcedureOutput) {
		.name = "entitytype", .type = T_STRING
	};
	array_append(outputs, output);

	// constraint status (operational / under construction/ failed)
	output = (ProcedureOutput) {
		.name = "status", .type = T_STRING
	};
	array_append(outputs, output);

	ProcedureCtx *ctx = ProcCtxNew("db.constraints",
								   0,
								   outputs,
								   Proc_ConstraintsStep,
								   Proc_ConstraintsInvoke,
								   Proc_ConstraintsFree,
								   privateData,
								   true);
	return ctx;
}
