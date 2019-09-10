/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include"ast/ast.h"
#include "redismodule.h"
#include "util/rmalloc.h"
#include "graph/graphcontext.h"
#include <setjmp.h>
#include <pthread.h>

typedef struct {
	AST *ast;             // The scoped AST associated with this query.
	GraphContext *gc;     // The GraphContext associated with this query's graph.
	double timer[2];      // Query execution time tracking.
	char *error;          // The error message produced by this query, if any.
	jmp_buf *breakpoint;  // The breakpoint to return to if the query causes an exception.
	bool free_cause;      // Whether the code that causes an exception must free memory.
} QueryCtx;

extern pthread_key_t _tlsQueryCtxKey;  // Thread local storage query context key.

/* On invocation, set an exception handler and a freeing policy, returning 0 from this macro.
 * Upon encountering an exception, execution will resume at this point and return nonzero. */
#define SET_EXCEPTION_HANDLER(free_exception_cause)              \
   ({                                                            \
	QueryCtx *ctx = pthread_getspecific(_tlsQueryCtxKey);        \
	ctx->free_cause = free_exception_cause;                      \
	setjmp(*ctx->breakpoint);                                    \
})

/* Instantiate the thread-local QueryCtx on module load. */
bool QueryCtx_Init(void);
/* Free the thread-local QueryCtx variable (unused). */
void QueryCtx_Finalize(void);

/* Instantiate thread-local variables and start timing query execution. */
void QueryCtx_Begin(void);

/* Set the provided AST for access through the QueryCtx. */
void QueryCtx_SetAST(AST *ast);
/* Set the provided GraphCtx for access through the QueryCtx. */
void QueryCtx_SetGraphCtx(GraphContext *gc);
/* Set the error message for this query. */
void QueryCtx_SetError(char *error);

/* Retrieve the AST. */
AST *QueryCtx_GetAST(void);
/* Retrieve the GraphCtx. */
GraphContext *QueryCtx_GetGraphCtx(void);
/* Retrieve the Graph object. */
Graph *QueryCtx_GetGraph(void);
/* Retrieve the exception handler. */
jmp_buf *QueryCtx_GetExceptionHandler(void);
/* Retrieve the error message if the query generated one. */
char *QueryCtx_GetError(void);

/* Returns true if the code which caused an exception must perform cleanup (compile-time errors). */
bool QueryCtx_ShouldFreeExceptionCause(void);
/* Returns true if this query has caused an error. */
bool QueryCtx_EncounteredError(void);

/* Compute and return elapsed query execution time. */
double QueryCtx_GetExecutionTime(void);

/* Free the allocations within the QueryCtx and reset it for the next query. */
void QueryCtx_Free(void);

