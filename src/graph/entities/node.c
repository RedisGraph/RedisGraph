/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <stdlib.h>

#include "node.h"
#include "edge.h"
#include "assert.h"
#include "graph_entity.h"
#include "../graphcontext.h"
#include "../../query_ctx.h"

inline Node Node_New() {
	Node n = {
		.entity = NULL,
		.label = NULL,
		.labelID = GRAPH_NO_LABEL,
	};
	return n;
}

inline void Node_SetLabel(Node *n, const char *label, int label_id) {
	n->label = label;
	n->labelID = label_id;
}

void Node_ToString(const Node *n, char **buffer, size_t *bufferLen, size_t *bytesWritten,
				   GraphEntityStringFromat format) {
	GraphEntity_ToString((const GraphEntity *)n, buffer, bufferLen, bytesWritten, format,
						 GETYPE_NODE);
}

