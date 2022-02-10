/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graph_entity.h"
#include "../../query_ctx.h"
#include "../graphcontext.h"
#include "../../util/rmalloc.h"
#include "../../datatypes/array.h"

SIValue GraphEntity_Keys
(
	const GraphEntity *e
) {
	GraphContext *gc = QueryCtx_GetGraphCtx();
	SIValue keys = SIArray_New(ENTITY_PROP_COUNT(e));
	for(int i = 0; i < e->attributes->attr_count; i++) {
		const char *key = GraphContext_GetAttributeString(gc, ENTITY_PROPS(e)[i].id);
		SIArray_Append(&keys, SI_ConstStringVal(key));
	}
	return keys;
}

size_t GraphEntity_PropertiesToString
(
	const GraphEntity *e,
	char **buffer,
	size_t *bufferLen,
	size_t *bytesWritten
) {
	// make sure there is enough space for "{...}\0"
	if(*bufferLen - *bytesWritten < 64) {
		*bufferLen += 64;
		*buffer = rm_realloc(*buffer, *bufferLen);
	}
	*bytesWritten += snprintf(*buffer, *bufferLen, "{");
	GraphContext *gc = QueryCtx_GetGraphCtx();
	int propCount = ENTITY_PROP_COUNT(e);
	Attribute *attributes = ENTITY_PROPS(e);
	for(int i = 0; i < propCount; i++) {
		// print key
		const char *key = GraphContext_GetAttributeString(gc, attributes[i].id);
		// check for enough space
		size_t keyLen = strlen(key);
		if(*bufferLen - *bytesWritten < keyLen) {
			*bufferLen += keyLen;
			*buffer = rm_realloc(*buffer, *bufferLen);
		}
		*bytesWritten += snprintf(*buffer + *bytesWritten, *bufferLen, "%s:", key);

		// print value
		SIValue_ToString(attributes[i].value, buffer, bufferLen, bytesWritten);

		// if not the last element print ", "
		if(i != propCount - 1) *bytesWritten = snprintf(*buffer + *bytesWritten, *bufferLen, ", ");

	}
	// check for enough space for close with "}\0"
	if(*bufferLen - *bytesWritten < 2) {
		*bufferLen += 2;
		*buffer = rm_realloc(*buffer, *bufferLen);
	}
	*bytesWritten += snprintf(*buffer + *bytesWritten, *bufferLen, "}");
	return *bytesWritten;
}

void GraphEntity_ToString
(
	const GraphEntity *e,
	char **buffer,
	size_t *bufferLen,
	size_t *bytesWritten,
	GraphEntityStringFromat format,
	GraphEntityType entityType
) {
	// space allocation
	if(*bufferLen - *bytesWritten < 64)  {
		*bufferLen += 64;
		*buffer = rm_realloc(*buffer, sizeof(char) * *bufferLen);
	}

	// get open an close symbols
	char *openSymbole;
	char *closeSymbole;
	if(entityType == GETYPE_NODE) {
		openSymbole = "(";
		closeSymbole = ")";
	} else {
		openSymbole = "[";
		closeSymbole = "]";
	}
	*bytesWritten += snprintf(*buffer + *bytesWritten, *bufferLen, "%s", openSymbole);

	// write id
	if(format & ENTITY_ID) {
		*bytesWritten += snprintf(*buffer + *bytesWritten, *bufferLen, "%" PRIu64, ENTITY_GET_ID(e));
	}

	// write label
	if(format & ENTITY_LABELS_OR_RELATIONS) {
		switch(entityType) {
			case GETYPE_NODE: {
				Node *n = (Node *)e;
				GraphContext *gc = QueryCtx_GetGraphCtx();

				// retrieve node labels
				uint label_count;
				NODE_GET_LABELS(gc->g, n, label_count);
				for(uint i = 0; i < label_count; i ++) {
					Schema *s = GraphContext_GetSchemaByID(gc, i, SCHEMA_NODE);
					const char *name = Schema_GetName(s);

					// allocate space if needed
					size_t labelLen = strlen(name);
					if(*bufferLen - *bytesWritten < labelLen) {
						*bufferLen += labelLen;
						*buffer = rm_realloc(*buffer, sizeof(char) * *bufferLen);
					}
					*bytesWritten += snprintf(*buffer + *bytesWritten, *bufferLen, ":%s", name);
				}
				break;
			}

			case GETYPE_EDGE: {
				Edge *edge = (Edge *)e;
				if(edge->relationship) {
					size_t relationshipLen = strlen(edge->relationship);
					if(*bufferLen - *bytesWritten < relationshipLen) {
						*bufferLen += relationshipLen;
						*buffer = rm_realloc(*buffer, sizeof(char) * *bufferLen);
					}
					*bytesWritten += snprintf(*buffer + *bytesWritten, *bufferLen, ":%s", edge->relationship);
				}
				break;
			}

			default:
				ASSERT(false);
		}
	}

	// write properies
	if(format & ENTITY_PROPERTIES) {
		GraphEntity_PropertiesToString(e, buffer, bufferLen, bytesWritten);
	}

	// check for enough space for close with closing symbol
	if(*bufferLen - *bytesWritten < 2) {
		*bufferLen += 2;
		*buffer = rm_realloc(*buffer, sizeof(char) * *bufferLen);
	}
	*bytesWritten += snprintf(*buffer + *bytesWritten, *bufferLen, "%s", closeSymbole);
}

inline bool GraphEntity_IsDeleted
(
	const GraphEntity *e
) {
	return Graph_EntityIsDeleted(e->attributes);
}
