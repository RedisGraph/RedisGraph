/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_create.h"
#include "RG.h"
#include "../../errors.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"

OpBase *NewCreateOp(const ExecutionPlan *plan, NodeCreateCtx *nodes, EdgeCreateCtx *edges) {
	OpCreate *op = rm_calloc(1, sizeof(OpCreate));
	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_CREATE, "Create", NULL, NULL, true, plan);

	op->nodes = nodes;
	op->edges = edges;

	uint node_blueprint_count = array_len(nodes);
	uint edge_blueprint_count = array_len(edges);

	// Construct the array of IDs this operation modifies
	for(uint i = 0; i < node_blueprint_count; i ++) {
		NodeCreateCtx *n = nodes + i;
		n->node_idx = OpBase_Modifies((OpBase *)op, n->alias);
	}
	for(uint i = 0; i < edge_blueprint_count; i ++) {
		EdgeCreateCtx *e = edges + i;
		e->edge_idx = OpBase_Modifies((OpBase *)op, e->alias);
		bool aware;
		UNUSED(aware);
		aware = RT_OpBase_Aware((OpBase *)op, e->src, &e->src_idx);
		ASSERT(aware == true);
		aware = RT_OpBase_Aware((OpBase *)op, e->dest, &e->dest_idx);
		ASSERT(aware == true);
	}

	return (OpBase *)op;
}
