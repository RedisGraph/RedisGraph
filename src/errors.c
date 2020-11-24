#include "RG.h"
#include "errors.h"
#include "util/arr.h"
#include "query_ctx.h"
#include "util/rax_extensions.h"
#include "../deps/libcypher-parser/lib/src/operators.h"

pthread_key_t _tlsErrorCtx; // Error-handling context held in thread-local storage.

//------------------------------------------------------------------------------
// Error setting and emitting
//------------------------------------------------------------------------------

bool ErrorCtx_Init(void) {
	return (pthread_key_create(&_tlsErrorCtx, NULL) == 0);
}

void ErrorCtx_New(void) {
	ErrorCtx *ctx = pthread_getspecific(_tlsErrorCtx);
	ctx = rm_calloc(1, sizeof(ErrorCtx));
	pthread_setspecific(_tlsErrorCtx, ctx);
}

void ErrorCtx_SetError(char *err_fmt, ...) {
	ErrorCtx *ctx = pthread_getspecific(_tlsErrorCtx);
	// An error is already set - free it
	if(ctx->error) free(ctx->error);
	// Set the new error
	va_list valist;
	va_start(valist, err_fmt);
	vasprintf(&ctx->error, err_fmt, valist);
	va_end(valist);
}

/* An error was encountered during evaluation, and has already been set in the ErrorCtx.
 * If an exception handler has been set, exit this routine and return to
 * the point on the stack where the handler was instantiated. */
void ErrorCtx_RaiseRuntimeException(void) {
	ErrorCtx *ctx = pthread_getspecific(_tlsErrorCtx);
	jmp_buf *env = ctx->breakpoint;
	// If the exception handler hasn't been set, this function returns to the caller,
	// which will manage its own freeing and error reporting.
	if(env) longjmp(*env, 1);
}

void ErrorCtx_EmitException(void) {
	ErrorCtx *ctx = pthread_getspecific(_tlsErrorCtx);
	if(ctx->error) {
		RedisModuleCtx *rm_ctx = QueryCtx_GetRedisModuleCtx();
		RedisModule_ReplyWithError(rm_ctx, ctx->error);
	}
}

inline bool ErrorCtx_EncounteredError(void) {
	ErrorCtx *ctx = pthread_getspecific(_tlsErrorCtx);
	return ctx->error != NULL;
}

void ErrorCtx_Free(void) {
	ErrorCtx *ctx = pthread_getspecific(_tlsErrorCtx);
	if(ctx->error) free(ctx->error);
	if(ctx->breakpoint) rm_free(ctx->breakpoint);
	rm_free(ctx);
	// NULL-set the context for reuse the next time this thread receives a query
	pthread_setspecific(_tlsErrorCtx, NULL);
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

