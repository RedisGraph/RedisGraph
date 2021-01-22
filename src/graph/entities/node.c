/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <stdlib.h>

#include "node.h"
#include "../graph.h"
#include "../../RG.h"
#include "../../query_ctx.h"

void Node_ToString(const Node *n, char **buffer, size_t *bufferLen, size_t *bytesWritten,
				   GraphEntityStringFromat format) {
	GraphEntity_ToString((const GraphEntity *)n, buffer, bufferLen, bytesWritten, format,
						 GETYPE_NODE);
}

uint Node_GetLabels(const Node *n, Label *l, uint ln) {
	// validate inputs
	ASSERT(n != NULL);
	ASSERT(l != NULL);

	GrB_Info res;
	GrB_Matrix M;
	uint count = 0;
	EntityID id = ENTITY_GET_ID(n);
	Graph *g = QueryCtx_GetGraph();
	GraphContext *gc = QueryCtx_GetGraphCtx();
	int labels_count = Graph_LabelTypeCount(g);

	for(int i = 0; i < labels_count && ln > count; i++) {
		bool x = false;
		M = Graph_GetLabelMatrix(g, i);
		res = GrB_Matrix_extractElement_BOOL(&x, M, id, id);

		if(res == GrB_SUCCESS) {
			// make sure buffer is big enough
			const Schema *s = GraphContext_GetSchemaByID(gc, i, SCHEMA_NODE);
			l[count].id = i;
			l[count].name = Schema_GetName(s);
			count++;
		}
	}

	return count;
}

