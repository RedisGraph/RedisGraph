/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_ctx.h"
#include "../schema/schema.h"
#include "../graph/graphcontext.h"

/* Common functionality and data type for both `proc_schema_edge_props` and `proc_schema_node_props` */

typedef struct {
	uint attr_idx;      // Current attribute ID.
	Schema *schema;     // Current schema.
	GraphContext *gc;   // Graph context.
	SIValue *output;    // Output.
} SchemaPropCtx;

// Common step function for both edge and node schema properties.
SIValue *Proc_Schema_PropertiesStep(ProcedureCtx *ctx, SchemaType schema_type);
