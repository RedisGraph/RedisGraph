/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./procedure.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../util/triemap/triemap.h"

static TrieMap *__procedures = NULL;

// Procedure accepts a variable number of arguments.
#define PROCEDURE_VARIABLE_ARG_COUNT UINT_MAX

// Procedure instance generator.
typedef ProcedureCtx* (*ProcGenerator)();
// Procedure step function.
typedef SIValue* (*ProcStep)(ProcedureCtx *ctx);
// Procedure function pointer.
typedef ProcedureResult (*ProcInvoke)(ProcedureCtx *ctx, char **args);
// Procedure free resources.
typedef ProcedureResult (*ProcFree)(ProcedureCtx *ctx);

static ProcedureCtx* _procCtxNew(const char *name,
                              unsigned int argc,
                              char **output,
                              ProcStep fStep,
                              ProcInvoke fInvoke,
                              ProcFree fFree,
                              void *privateData) {

    ProcedureCtx *ctx = rm_malloc(sizeof(ProcedureCtx));
    ctx->argc = argc;
    ctx->name = name;
    ctx->Step = fStep;
    ctx->output = output;
    ctx->Invoke = fInvoke;
    ctx->Free = fFree;
    ctx->privateData = privateData;
    return ctx;
}

//------------------------------------------------------------------------------
// fulltext queryNodes
//------------------------------------------------------------------------------

typedef struct {
    Node n;
    Graph *g;
    bool stop;
    SIValue *output;
} QueryNodeContext;

// CALL db.idx.fulltext.queryNodes(label, query)
ProcedureResult fulltextQueryNodeInvoke(ProcedureCtx *ctx, char **args) {
    QueryNodeContext *pdata = rm_malloc(sizeof(QueryNodeContext));
    pdata->output = array_new(SIValue, 4);
    pdata->stop = false;
    pdata->g = GraphContext_GetFromTLS()->g;
    pdata->output = array_append(pdata->output, SI_ConstStringVal("node"));
    pdata->output = array_append(pdata->output, SI_Node(&pdata->n));
    pdata->output = array_append(pdata->output, SI_ConstStringVal("score"));
    pdata->output = array_append(pdata->output, SI_DoubleVal(0.0));
    ctx->privateData = pdata;
    return PROCEDURE_OK;
}

SIValue* fulltextQueryNodeStep(ProcedureCtx *ctx) {
    assert(ctx->privateData);

    QueryNodeContext *pdata = (QueryNodeContext*)ctx->privateData;

    // Temporary!
    if(pdata->stop) return NULL;
    pdata->stop = true;

    Node *n = &pdata->n;
    Graph_GetNode(pdata->g, 0, n);

    pdata->output[1] = SI_Node(n);
    pdata->output[3] = SI_DoubleVal(12.34);
    return pdata->output;
}

ProcedureResult fulltextQueryNodeFree(ProcedureCtx *ctx) {
    // Clean up.
    if(ctx->privateData) {
        QueryNodeContext *pdata = ctx->privateData;
        array_free(pdata->output);
        rm_free(ctx->privateData);
    }
    return PROCEDURE_OK;
}

ProcedureCtx* fulltextQueryNodeGen() {
    void *privateData = NULL;
    char **output = array_new(char*, 2);
    output = array_append(output, "node");
    output = array_append(output, "score");
    ProcedureCtx *ctx = _procCtxNew("db.idx.fulltext.queryNodes",
                                    2,
                                    output,
                                    fulltextQueryNodeStep,
                                    fulltextQueryNodeInvoke,
                                    fulltextQueryNodeFree,
                                    privateData);
    return ctx;
}

//------------------------------------------------------------------------------
// fulltext createNodeIndex
//------------------------------------------------------------------------------

// CALL db.idx.fulltext.createNodeIndex(index_name, label, attributes)
// CALL db.idx.fulltext.createNodeIndex('books', ['Book'], ['title', 'authors'])

ProcedureResult fulltextCreateNodeIdxInvoke(ProcedureCtx *ctx, char **args) {
    if(array_len(args) < 2) return PROCEDURE_ERR;
    
    char *label = args[0];
    for(int i = 0; i < array_len(args); i++) {
        // Create full-text index.
    }
    return PROCEDURE_OK;
}

SIValue* fulltextCreateNodeIdxStep(ProcedureCtx *ctx) {
    return NULL;
}

ProcedureResult fulltextCreateNodeIdxFree(ProcedureCtx *ctx) {
    // Clean up.
    return PROCEDURE_OK;
}

ProcedureCtx* fulltextCreateNodeIdxGen() {
    void *privateData = NULL;
    char **output = array_new(char*, 0);
    ProcedureCtx *ctx = _procCtxNew("db.idx.fulltext.createNodeIndex",
                                    PROCEDURE_VARIABLE_ARG_COUNT,
                                    output,
                                    fulltextCreateNodeIdxStep,
                                    fulltextCreateNodeIdxInvoke,
                                    fulltextCreateNodeIdxFree,
                                    privateData);

    return ctx;
}

static void _procRegister(const char *procedure, ProcGenerator gen) {
    TrieMap_Add(__procedures, (char*)procedure, strlen(procedure), gen, NULL);
}

// Register procedures.
void Proc_Register() {
    __procedures = NewTrieMap();
    // Register FullText Search generator.
    _procRegister("db.idx.fulltext.createNodeIndex", fulltextCreateNodeIdxGen);
    _procRegister("db.idx.fulltext.queryNodes", fulltextQueryNodeGen);
}

ProcedureCtx* Proc_Get(const char *proc_name) {
    if(!__procedures) return NULL;
    ProcGenerator gen = TrieMap_Find(__procedures, (char*)proc_name, strlen(proc_name));
    if(gen == TRIEMAP_NOTFOUND) return NULL;
    ProcedureCtx *ctx = gen();
    return ctx;
}

ProcedureResult Proc_Invoke(ProcedureCtx *proc, char **args) {
    assert(proc);
    if(proc->argc != PROCEDURE_VARIABLE_ARG_COUNT) assert(proc->argc == array_len(args));
    // TODO: procedure can only be invoke once.
    return proc->Invoke(proc, args);
}

SIValue* Proc_Step(ProcedureCtx *proc) {
    assert(proc);
    return proc->Step(proc);
}

ProcedureResult ProcedureReset(ProcedureCtx *proc) {
    // return proc->restart(proc);
    return PROCEDURE_OK;
}

void Proc_Free(ProcedureCtx *proc) {
    if(!proc) return;
    proc->Free(proc);
    if(proc->output) array_free(proc->output);
    rm_free(proc);
}
