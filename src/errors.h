/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include <setjmp.h>
#include <stddef.h>
#include "rax.h"
#include "value.h"
#include "cypher-parser.h"

extern pthread_key_t _tlsErrorCtx; // Error-handling context held in thread-local storage.

typedef struct {
	char *error;                // The error message produced, if any.
	jmp_buf *breakpoint;        // The breakpoint to jump to in the case of an exception.
} ErrorCtx;

/* On invocation, set an exception handler, returning 0 from this macro.
 * Upon encountering an exception, execution will resume at this point and return nonzero. */
#define SET_EXCEPTION_HANDLER()                                         \
   ({                                                                   \
    ErrorCtx *ctx = ErrorCtx_Get();                                     \
    if(!ctx->breakpoint) ctx->breakpoint = rm_malloc(sizeof(jmp_buf));  \
    setjmp(*ctx->breakpoint);                                           \
})

// Instantiate the thread-local ErrorCtx on module load.
bool ErrorCtx_Init(void);

// Retrieve the ErrorCtx, building one if it does not exist.
ErrorCtx *ErrorCtx_Get(void);

// Zero-set the members of the ErrorCtx.
void ErrorCtx_Clear(void);

// Set the error message for this query.
void ErrorCtx_SetError(const char *err_fmt, ...);

// Jump to a runtime exception breakpoint if one has been set
void ErrorCtx_RaiseRuntimeException(const char *err_fmt, ...);

// Reply back to the user with error
void ErrorCtx_EmitException(void);

bool ErrorCtx_EncounteredError(void);

// Report an error in filter placement with the first unresolved entity.
void Error_InvalidFilterPlacement(rax *entitiesRax);

// Report an error when an SIValue resolves to an unhandled type.
void Error_SITypeMismatch(SIValue received, SIType expected);

// Report an error on receiving an unhandled AST node type.
void Error_UnsupportedASTNodeType(const cypher_astnode_t *node);

// Report an error on receiving an unhandled AST operator.
void Error_UnsupportedASTOperator(const cypher_operator_t *op);

// Report an error on trying to assign a complex type to a property.
void Error_InvalidPropertyValue(void);
