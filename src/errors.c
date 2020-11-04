#include "errors.h"
#include "util/arr.h"
#include "query_ctx.h"
#include "util/rax_extensions.h"

void Error_InvalidFilterPlacement(rax *entitiesRax) {
	// Something is wrong - could not find a matching op where all references are solved.
	raxIterator it;
	raxStart(&it, entitiesRax);
	// Retrieve the first key in the rax.
	raxSeek(&it, "^", NULL, 0);
	raxNext(&it);
	// Build invalid entity string on the stack to add null terminator.
	char *invalid_entity[it.key_len + 1];
	memcpy(invalid_entity, it.key, it.key_len);
	invalid_entity[it.key_len] = 0;
	// Emit compile-time error.
	QueryCtx_SetError("Unable to resolve filtered alias '%s'", invalid_entity);
	raxFree(entitiesRax);
}

inline void Error_SITypeMismatch(const char *received, const char *expected) {
	QueryCtx_SetError("Type mismatch: expected %s but was %s", expected, received);
}

inline void Error_UnsupportedType(const char *type) {
	QueryCtx_SetError("RedisGraph does not currently support %s", type);
}

