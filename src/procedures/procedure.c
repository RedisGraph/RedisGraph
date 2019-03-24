/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./procedure.h"
#include "procedures.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../util/triemap/triemap.h"

static TrieMap *__procedures = NULL;

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

ProcedureCtx* ProcCtxNew(const char *name,
                         unsigned int argc,
                         ProcedureOutput **output,
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
    for(uint i = 0; i < array_len(proc->output); i++) {
        rm_free(proc->output[i]);
    }
    if(proc->output) array_free(proc->output);
    rm_free(proc);
}
