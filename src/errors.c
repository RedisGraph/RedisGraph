#include "RG.h"
#include "errors.h"
#include "util/arr.h"
#include "query_ctx.h"
#include "util/rax_extensions.h"
#include "../deps/libcypher-parser/lib/src/operators.h"

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
	QueryCtx_SetError("Unable to resolve filtered alias '%s'", invalid_entity);
	raxFree(entitiesRax);
}

void Error_SITypeMismatch(SIValue received, SIType expected) {
	QueryCtx_SetError("Type mismatch: expected %s but was %s", SIType_ToString(expected),
					  SIType_ToString(SI_TYPE(received)));
}

void Error_UnsupportedASTNodeType(const cypher_astnode_t *node) {
	ASSERT(node != NULL);

	cypher_astnode_type_t type = cypher_astnode_type(node);
	const char *type_str = cypher_astnode_typestr(type);
	QueryCtx_SetError("RedisGraph does not currently support %s", type_str);
}

void Error_UnsupportedASTOperator(const cypher_operator_t *op) {
	ASSERT(op != NULL);

	QueryCtx_SetError("RedisGraph does not currently support %s", op->str);
}

inline void Error_InvalidPropertyValue(void) {
	QueryCtx_SetError("Property values can only be of primitive types or arrays thereof");
}

