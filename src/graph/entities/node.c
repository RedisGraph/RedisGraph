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
	clone->attributes             = AttributeSet_Clone(n->attributes);
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

