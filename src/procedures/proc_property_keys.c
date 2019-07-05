/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_property_keys.h"
#include "../value.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"

// CALL db.propertyKeys()

typedef struct {
    uint prop_id;       // Current property ID.
    GraphContext *gc;   // Graph context.
    SIValue *output;    // Output label.
} RelationsContext;

ProcedureResult Proc_PropKeysInvoke(ProcedureCtx *ctx, const char **args) {
    if(array_len(args) != 0) return PROCEDURE_ERR;

    RelationsContext *pdata = rm_malloc(sizeof(RelationsContext));
    pdata->prop_id = 0;
    pdata->gc = GraphContext_GetFromTLS();
    pdata->output = array_new(SIValue, 2);
    pdata->output = array_append(pdata->output, SI_ConstStringVal("propertyKey"));
    pdata->output = array_append(pdata->output, SI_ConstStringVal("")); // Place holder.

    ctx->privateData = pdata;
    return PROCEDURE_OK;
}

SIValue* Proc_PropKeysStep(ProcedureCtx *ctx) {
    assert(ctx->privateData);

    RelationsContext *pdata = (RelationsContext*)ctx->privateData;

    // Depleted?
    if(pdata->prop_id >= GraphContext_AttributeCount(pdata->gc))
    return NULL;

    // Get attribute name.
    char *name = (char*)GraphContext_GetAttributeString(pdata->gc, pdata->prop_id++);
    pdata->output[1] = SI_ConstStringVal(name);
    return pdata->output;
}

ProcedureResult Proc_PropKeysFree(ProcedureCtx *ctx) {
    // Clean up.
    if(ctx->privateData) {
        RelationsContext *pdata = ctx->privateData;
        array_free(pdata->output);
        rm_free(ctx->privateData);
    }

    return PROCEDURE_OK;
}

ProcedureCtx* Proc_PropKeysCtx() {
    void *privateData = NULL;
    ProcedureOutput **outputs = array_new(ProcedureOutput*, 1);
    ProcedureOutput *output = rm_malloc(sizeof(ProcedureOutput));
    output->name = "propertyKey";
    output->type = T_CONSTSTRING;

    outputs = array_append(outputs, output);
    ProcedureCtx *ctx = ProcCtxNew("db.propertyKeys",
                                    0,
                                    outputs,
                                    Proc_PropKeysStep,
                                    Proc_PropKeysInvoke,
                                    Proc_PropKeysFree,
                                    privateData);
    return ctx;
}
