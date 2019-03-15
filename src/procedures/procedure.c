/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./procedure.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../util/triemap/triemap.h"

static TrieMap *__procedures = NULL;

// Procedure accepts a variable number of arguments.
#define PROCEDURE_VARIABLE_ARG_COUNT USHRT_MAX

// Procedure instance generator.
typedef ProcedureCtx* (*ProcGenerator)();
// Procedure step function.
typedef ProcedureResult (*ProcStep)(ProcedureCtx *ctx, Record r);
// Procedure function pointer.
typedef ProcedureResult (*ProcInvoke)(ProcedureCtx *ctx, const char **args);
// Procedure finalize function.
typedef ProcedureResult (*ProcFinalize)(ProcedureCtx *ctx);

static ProcedureCtx* _procCtxNew(const char *name,
                              uint argc,
                              char **output,
                              ProcStep fStep,
                              ProcInvoke fInvoke,
                              ProcFinalize fFinalize,
                              void *privateData) {

    ProcedureCtx *ctx = rm_malloc(sizeof(ProcedureCtx));
    ctx->argc = argc;
    ctx->name = name;
    ctx->Step = fStep;
    ctx->output = output;
    ctx->Invoke = fInvoke;
    ctx->Finalize = fFinalize;
    ctx->privateData = privateData;
    return ctx;
}

//------------------------------------------------------------------------------
// fulltext queryNodes
//------------------------------------------------------------------------------

// CALL db.index.fulltext.queryNodes(label, query)
ProcedureResult fulltextQueryNodeInvoke(ProcedureCtx *ctx, const char **args) {
    return PROCEDURE_OK;
}

ProcedureResult fulltextQueryNodeStep(ProcedureCtx *ctx, Record r) {
    return PROCEDURE_OK;
}

ProcedureResult fulltextQueryNodeFinalize(ProcedureCtx *ctx) {
    // Clean up.
    return PROCEDURE_OK;
}

ProcedureCtx* fulltextQueryNodeGen() {
    void *privateData = NULL;
    char **output = array_new(char*, 2);
    output[0] = "node";
    output[1] = "score";
    ProcedureCtx *ctx = _procCtxNew("db.index.fulltext.queryNodes",
                                        2,
                                        output,
                                        fulltextQueryNodeStep,
                                        fulltextQueryNodeInvoke,
                                        fulltextQueryNodeFinalize,
                                        privateData);

    return ctx;
}

//------------------------------------------------------------------------------
// fulltext createNodeIndex
//------------------------------------------------------------------------------

// CALL db.index.fulltext.createNodeIndex(index_name, label, attributes)
// CALL db.index.fulltext.createNodeIndex('books', ['Book'], ['title', 'authors'])

ProcedureResult fulltextCreateNodeIdxInvoke(ProcedureCtx *ctx, const char **args) {
    return PROCEDURE_OK;
}

ProcedureResult fulltextCreateNodeIdxStep(ProcedureCtx *ctx, Record r) {
    return PROCEDURE_OK;
}

ProcedureResult fulltextCreateNodeIdxFinalize(ProcedureCtx *ctx) {
    // Clean up.
    return PROCEDURE_OK;
}

ProcedureCtx* fulltextCreateNodeIdxGen() {
    void *privateData = NULL;
    char **output = array_new(char*, 0);
    ProcedureCtx *ctx = _procCtxNew("db.index.fulltext.createNodeIndex",
                                        PROCEDURE_VARIABLE_ARG_COUNT,
                                        output,
                                        fulltextCreateNodeIdxStep,
                                        fulltextCreateNodeIdxInvoke,
                                        fulltextCreateNodeIdxFinalize,
                                        privateData);

    return ctx;
}

static void _procRegister(const char *procedure, ProcGenerator gen) {
    TrieMap_Add(__procedures, procedure, strlen(procedure), gen, NULL);
}

// Register procedures.
void Proc_Register() {
    __procedures = NewTrieMap();
    // Register FullText Search generator.
    _procRegister("db.index.fulltext.createNodeIndex", fulltextQueryNodeGen);
    _procRegister("db.index.fulltext.queryNodes", fulltextCreateNodeIdxGen);
}
