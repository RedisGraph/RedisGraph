#include "RG.h"
#include "errors.h"
#include "util/arr.h"
#include "query_ctx.h"
#include "util/rax_extensions.h"
#include "../deps/libcypher-parser/lib/src/operators.h"

pthread_key_t _tlsErrorCtx; // Error-handling context held in thread-local storage.

//------------------------------------------------------------------------------
// Error context initialization
//------------------------------------------------------------------------------

bool ErrorCtx_Init(void) {
	int res = pthread_key_create(&_tlsErrorCtx, NULL);
	ASSERT(res == 0);

	return (res == 0);
}

ErrorCtx *ErrorCtx_Get(void) {
	ErrorCtx *ctx = pthread_getspecific(_tlsErrorCtx);

	if(ctx == NULL) {
		ctx = rm_calloc(1, sizeof(ErrorCtx));
		int res = pthread_setspecific(_tlsErrorCtx, ctx);
		ASSERT(res == 0);
	}

	return ctx;
}

void ErrorCtx_Clear(void) {
	ErrorCtx *ctx = ErrorCtx_Get();
	ASSERT(ctx != NULL);

	if(ctx->error != NULL) {
		free(ctx->error);
		ctx->error = NULL;
	}

	if(ctx->breakpoint != NULL) {
		rm_free(ctx->breakpoint);
		ctx->breakpoint = NULL;
	}
}

//------------------------------------------------------------------------------
// Error setting and emitting
//------------------------------------------------------------------------------

static void _ErrorCtx_SetError(const char *err_fmt, va_list args) {
	ErrorCtx *ctx = ErrorCtx_Get();
	ASSERT(ctx != NULL);

	// An error is already set - free it
	if(ctx->error != NULL) free(ctx->error);

	vasprintf(&ctx->error, err_fmt, args);
}

void ErrorCtx_SetError(const char *err_fmt, ...) {
	// Set the new error
	va_list valist;
	va_start(valist, err_fmt);
	_ErrorCtx_SetError(err_fmt, valist);
	va_end(valist);
}

/* An error was encountered during evaluation, and has already been set in the ErrorCtx.
 * If an exception handler has been set, exit this routine and return to
 * the point on the stack where the handler was instantiated. */
void ErrorCtx_RaiseRuntimeException(const char *err_fmt, ...) {
	ErrorCtx *ctx = ErrorCtx_Get();
	ASSERT(ctx != NULL);

	// set error if specified
	if(err_fmt != NULL) {
		va_list valist;
		va_start(valist, err_fmt);
		_ErrorCtx_SetError(err_fmt, valist);
		va_end(valist);
	}

	jmp_buf *env = ctx->breakpoint;
	// If the exception handler hasn't been set, this function returns to the caller,
	// which will manage its own freeing and error reporting.
	if(env != NULL) longjmp(*env, 1);
}

// Reply to caller with error
void ErrorCtx_EmitException(void) {
	ErrorCtx *ctx = ErrorCtx_Get();
	ASSERT(ctx != NULL);

	if(ctx->error != NULL) {
		RedisModuleCtx *rm_ctx = QueryCtx_GetRedisModuleCtx();
		RedisModule_ReplyWithError(rm_ctx, ctx->error);
	}

	// clear error context once error emitted
	ErrorCtx_Clear();
}

// Returns true if error is set
inline bool ErrorCtx_EncounteredError(void) {
	ErrorCtx *ctx = ErrorCtx_Get();
	ASSERT(ctx != NULL);

	return ctx->error != NULL;
}

//------------------------------------------------------------------------------
// Specific error scenarios
//------------------------------------------------------------------------------

void Error_InvalidFilterPlacement(rax *entitiesRax) {
	ASSERT(entitiesRax != NULL);

	// Something is wrong - could not find a matching op where all references are solved.
	raxIterator it;
	raxStart(&it, entitiesRax);
	// Retrieve the first key in the rax.
	raxSeek(&it, "^", NULL, 0);
	raxNext(&it);
	// Build invalid entity string on the stack to add null terminator.
	char invalid_entity[it.key_len + 1];
	memcpy(invalid_entity, it.key, it.key_len);
	invalid_entity[it.key_len] = 0;
	// Emit compile-time error.
	ErrorCtx_SetError("Unable to resolve filtered alias '%s'", invalid_entity);
	raxFree(entitiesRax);
}

void Error_SITypeMismatch(SIValue received, SIType expected) {
	ErrorCtx_SetError("Type mismatch: expected %s but was %s", SIType_ToString(expected),
					  SIType_ToString(SI_TYPE(received)));
}

void Error_UnsupportedASTNodeType(const cypher_astnode_t *node) {
	ASSERT(node != NULL);

	cypher_astnode_type_t type = cypher_astnode_type(node);
	const char *type_str = cypher_astnode_typestr(type);
	ErrorCtx_SetError("RedisGraph does not currently support %s", type_str);
}

void Error_UnsupportedASTOperator(const cypher_operator_t *op) {
	ASSERT(op != NULL);

	ErrorCtx_SetError("RedisGraph does not currently support %s", op->str);
}

inline void Error_InvalidPropertyValue(void) {
	ErrorCtx_SetError("Property values can only be of primitive types or arrays thereof");
}

