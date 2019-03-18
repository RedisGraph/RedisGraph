/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "fulltext_create_index.h"
#include "../value.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"

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
    ProcedureCtx *ctx = ProcCtxNew("db.idx.fulltext.createNodeIndex",
                                    PROCEDURE_VARIABLE_ARG_COUNT,
                                    output,
                                    fulltextCreateNodeIdxStep,
                                    fulltextCreateNodeIdxInvoke,
                                    fulltextCreateNodeIdxFree,
                                    privateData);

    return ctx;
}
