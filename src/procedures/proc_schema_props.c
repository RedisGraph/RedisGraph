/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_schema_props.h"

SIValue *Proc_Schema_PropertiesStep(ProcedureCtx *ctx, SchemaType schema_type) {
	assert(ctx->privateData);

	SchemaPropCtx *pdata = (SchemaPropCtx *)ctx->privateData;
	GraphContext *gc = pdata->gc;

	// Depleted?
	if(!pdata->schema) return NULL;

	uint attr_count = Schema_AttributeCount(pdata->schema);
	while(pdata->attr_idx >= attr_count) {
		pdata->attr_idx = 0;
		// Try to get a new schema.
		pdata->schema = GraphContext_GetSchemaByID(gc, pdata->schema->id + 1, schema_type);
		if(!pdata->schema) return NULL;
		attr_count = Schema_AttributeCount(pdata->schema);
	}

	// Get schema name.
	char *schema_name = (char *)Schema_GetName(pdata->schema);
	// Extract attributes from schema.
	Attribute_ID schema_attributes[attr_count];
	Schema_GetAttributes(pdata->schema, schema_attributes, attr_count);

	// Set outputs.
	pdata->output[1] = SI_ConstStringVal(schema_name);
	pdata->output[3] = SI_ConstStringVal((char *)GraphContext_GetAttributeString(gc,
																				 schema_attributes[pdata->attr_idx]));

	// Update current attribute index.
	pdata->attr_idx++;

	return pdata->output;
}
