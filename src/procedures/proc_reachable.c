/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "./procedure.h"
#include "./query_ctx.h"
#include "../graph/graph.h"
#include "../algorithms/reachability.h"

typedef struct {
	const Graph *g;
	NodeID src;
	NodeID dest;
	bool first_call;
	SIValue *output;
} ReachableCtx;

static SIValue *Proc_Reachable_Step
(
	ProcedureCtx *ctx
) {
	ASSERT(ctx->privateData != NULL);

	ReachableCtx *pdata = (ReachableCtx *)ctx->privateData;

	// return if this is not the first call
	if(pdata->first_call == false) {
		return NULL;
	}
	pdata->first_call = false;

	bool res = reachable(pdata->src, pdata->dest, pdata->g);

	pdata->output[0] = SI_BoolVal(res);

	return pdata->output;
}

static ProcedureResult Proc_Reachable_Free
(
	ProcedureCtx *ctx
) {
	// clean up
	if(ctx->privateData != NULL) {
		ReachableCtx *pdata = ctx->privateData;
		array_free(pdata->output);
		rm_free(ctx->privateData);
	}

	return PROCEDURE_OK;
}

static ProcedureResult Proc_Reachable_Invoke
(
	ProcedureCtx *ctx,
	const SIValue *args,
	const char **yield
) {
	// validate inputs
	ASSERT(ctx  != NULL);
	ASSERT(args != NULL);

	if(array_len((SIValue *)args) != 2) {
		return PROCEDURE_ERR;
	}

	if(SI_TYPE(args[0]) != T_NODE ||  // source node
	   SI_TYPE(args[1]) != T_NODE) {  // destination node
		return PROCEDURE_ERR;
	}

	Graph *g    = QueryCtx_GetGraph();

	Node *src_node  = (Node*)args[0].ptrval;
	Node *dest_node = (Node*)args[1].ptrval;
	NodeID src      = ENTITY_GET_ID(src_node);
	NodeID dest     = ENTITY_GET_ID(dest_node);

	ReachableCtx *pdata = rm_malloc(sizeof(ReachableCtx));

	pdata->g          = g;
	pdata->src        = src;
	pdata->dest       = dest;
	pdata->output     = array_new(SIValue,  1);
	pdata->first_call = true;

	array_append(pdata->output, SI_ConstStringVal("reachable"));

	ctx->privateData = pdata;

	return PROCEDURE_OK;
}

ProcedureCtx *Proc_Reachable_Ctx(void) {
	// declare possible outputs
	ProcedureOutput *outputs = array_new(ProcedureOutput, 1);
	ProcedureOutput out_reachable = {.name = "reachable", .type = T_BOOL};
	array_append(outputs, out_reachable);

	ProcedureCtx *ctx = ProcCtxNew("algo.REACHABLE",
								   2,
								   outputs,
								   Proc_Reachable_Step,
								   Proc_Reachable_Invoke,
								   Proc_Reachable_Free,
								   NULL,
								   true);
	return ctx;
}

