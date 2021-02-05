/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <stdlib.h>

#include "node.h"

void Node_ToString(const Node *n, char **buffer, size_t *bufferLen, size_t *bytesWritten,
				   GraphEntityStringFromat format) {
	GraphEntity_ToString((const GraphEntity *)n, buffer, bufferLen, bytesWritten, format,
						 GETYPE_NODE);
}

