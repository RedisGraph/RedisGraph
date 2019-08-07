/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <stdlib.h>

#include "node.h"
#include "edge.h"
#include "assert.h"
#include "graph_entity.h"
#include "../graphcontext.h"

Node *Node_New(const char *label) {
	Node *n = calloc(1, sizeof(Node));
	n->labelID = GRAPH_UNKNOWN_LABEL;
	n->label = label;

	return n;
}

GrB_Matrix Node_GetMatrix(Node *n) {
	/* Node's label must be set,
	 * otherwise it doesn't make sense to refer to a matrix. */
	assert(n && n->label);

	// Retrieve matrix from graph if edge matrix isn't set.
	if(!n->mat) {
		GraphContext *gc = GraphContext_GetFromTLS();
		Graph *g = gc->g;

		/* Get label matrix:
		 * There's no sense in calling Node_GetMatrix
		 * if node isn't labeled. */
		assert(n->labelID != GRAPH_NO_LABEL);
		if(n->labelID == GRAPH_UNKNOWN_LABEL) {
			// Label specified (n:Label), but doesn't exists.
			n->mat = Graph_GetZeroMatrix(g);
		} else {
			n->mat = Graph_GetLabelMatrix(g, n->labelID);
		}
	}

	return n->mat;
}

Node *Node_Clone(const Node *n) {
	Node *clone = Node_New(n->label);
	clone->mat = n->mat;
	// TODO: consider setting labelID in Node_New.
	clone->labelID = n->labelID;
	return clone;
}

int Node_ToString(const Node *n, char *buffer, int bufferLen, GraphEntityStringFromat format) {
	if(bufferLen <= 1) return 0;
	int bytes_written = snprintf(buffer, bufferLen, "(");
	bufferLen -= bytes_written;
	int currentWriteLength = 0;

	// write id
	if(format & ENTITY_ID) {
		currentWriteLength = snprintf(buffer + bytes_written, bufferLen, "%llu", ENTITY_GET_ID(n));
		bytes_written += currentWriteLength;
		bufferLen -= currentWriteLength;
	}

	// write label
	if(bufferLen > 2 && format & ENTITY_LABELS_OR_RELATIONS) {
		if(n->label) {
			currentWriteLength = snprintf(buffer + bytes_written, bufferLen, ":%s", n->label);
			bytes_written += currentWriteLength;
			bufferLen -= currentWriteLength;
		}
	}

	// write properies
	if(bufferLen > 2 && format & ENTITY_PROPERTIES) {
		currentWriteLength = GraphEntity_PropertiesToString((GraphEntity *)n, buffer + bytes_written,
															bufferLen);
		bytes_written += currentWriteLength;
		bufferLen -= currentWriteLength;
	}

	if(bufferLen >= 2) {
		bytes_written += snprintf(buffer + bytes_written, bufferLen, ")");
		return bytes_written;
	}
	// if there is no space left
	snprintf(buffer + strlen(buffer) - 5, 5, "...)");
	return strlen(buffer);
}

void Node_Free(Node *node) {
	if(!node) return;

	free(node);
	node = NULL;
}
