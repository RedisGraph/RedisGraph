/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "node.h"
#include "../../util/rmalloc.h"

void Node_Clone
(
	const Node *n,
	Node *clone
) {
	clone->id = n->id;
	clone->entity = rm_malloc(sizeof(Entity));
	clone->entity->prop_count = n->entity->prop_count;
	clone->entity->properties = rm_malloc(sizeof(EntityProperty) * n->entity->prop_count);
	for (uint i = 0; i < clone->entity->prop_count; i++) {
		clone->entity->properties[i] = n->entity->properties[i];
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

