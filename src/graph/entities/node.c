/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "node.h"
#include "../../util/rmalloc.h"

void Node_Clone
(
	const Node *n,
	Node *clone
) {
	ASSERT(n != NULL);
	ASSERT(clone != NULL);

	clone->id                     = n->id;
	clone->attributes             = rm_malloc(sizeof(AttributeSet));
	clone->attributes->prop_count = ENTITY_PROP_COUNT(n);
	clone->attributes->properties = rm_malloc(sizeof(EntityProperty) * ENTITY_PROP_COUNT(n));
	for (uint i = 0; i < ENTITY_PROP_COUNT(n); i++) {
		EntityProperty *prop       = ENTITY_PROPS(n) + i;
		EntityProperty *clone_prop = ENTITY_PROPS(clone) + i;
		clone_prop->id             = prop->id;
		clone_prop->value          = SI_CloneValue(prop->value);
	}
}

void Node_ToString
(
	const Node *n,
	char **buffer,
	size_t *bufferLen,
	size_t *bytesWritten,
	GraphEntityStringFromat format
) {
	GraphEntity_ToString((const GraphEntity *)n, buffer, bufferLen,
			bytesWritten, format, GETYPE_NODE);
}

