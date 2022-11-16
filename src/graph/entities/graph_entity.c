/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "graph_entity.h"
#include "../../errors.h"
#include "../../query_ctx.h"
#include "../graphcontext.h"
#include "../../util/rmalloc.h"
#include "../../datatypes/map.h"
#include "../../datatypes/array.h"

// add a new property to entity
bool GraphEntity_AddProperty
(
	GraphEntity *e,
	Attribute_ID attr_id,
	SIValue value
) {
	ASSERT(e);

	AttributeSet_Add(e->attributes, attr_id, value);
	
	return true;
}

SIValue *GraphEntity_GetProperty
(
	const GraphEntity *e,
	Attribute_ID attr_id
) {
	ASSERT(e);

	// e->attributes is NULL when dealing with an "intermediate" entity,
	// one which didn't had its attribute-set allocated within the graph datablock.
	if(e->attributes == NULL) {
 		// note that this exception may cause memory to be leaked in the caller
 		ErrorCtx_SetError("Attempted to access undefined attribute");
 		return ATTRIBUTE_NOTFOUND;
 	}

	return AttributeSet_Get(*e->attributes, attr_id);
}

// updates existing property value
bool GraphEntity_SetProperty
(
	const GraphEntity *e,
	Attribute_ID attr_id,
	SIValue value
) {
	ASSERT(e);

	return AttributeSet_Update(e->attributes, attr_id, value);
}

// returns an SIArray of all keys in graph entity properties
SIValue GraphEntity_Keys
(
	const GraphEntity *e
) {
	GraphContext *gc = QueryCtx_GetGraphCtx();
	const AttributeSet set = GraphEntity_GetAttributes(e);
	int prop_count = ATTRIBUTE_SET_COUNT(set);
	SIValue keys = SIArray_New(prop_count);
	for(int i = 0; i < prop_count; i++) {
		Attribute_ID attr_id;
		AttributeSet_GetIdx(set, i, &attr_id);
		const char *key = GraphContext_GetAttributeString(gc, attr_id);
		SIArray_Append(&keys, SI_ConstStringVal(key));
	}
	return keys;
}

// returns a map containing all the properties in the given node, or edge. 
SIValue GraphEntity_Properties
(
	const GraphEntity *e
) {
	GraphContext *gc = QueryCtx_GetGraphCtx();
	const AttributeSet set = GraphEntity_GetAttributes(e);
	int propCount = ATTRIBUTE_SET_COUNT(set);
	SIValue map = SI_Map(propCount);
	for(int i = 0; i < propCount; i++) {
		Attribute_ID attr_id;
		SIValue value = AttributeSet_GetIdx(set, i, &attr_id);
		const char *key = GraphContext_GetAttributeString(gc, attr_id);
		Map_Add(&map, SI_ConstStringVal(key), value);
	}
	return map;
}

// prints the attribute set into a buffer, returns what is the string length
// buffer can be re-allocated if needed
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
	const AttributeSet set = GraphEntity_GetAttributes(e);
	int propCount = ATTRIBUTE_SET_COUNT(set);
	for(int i = 0; i < propCount; i++) {
		Attribute_ID attr_id;
		SIValue value = AttributeSet_GetIdx(set, i, &attr_id);
		// print key
		const char *key = GraphContext_GetAttributeString(gc, attr_id);
		// check for enough space
		size_t keyLen = strlen(key);
		if(*bufferLen - *bytesWritten < keyLen) {
			*bufferLen += keyLen;
			*buffer = rm_realloc(*buffer, *bufferLen);
		}
		*bytesWritten += snprintf(*buffer + *bytesWritten, *bufferLen, "%s:", key);

		// print value
		SIValue_ToString(value, buffer, bufferLen, bytesWritten);

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
	return Graph_EntityIsDeleted(e);
}

inline const AttributeSet GraphEntity_GetAttributes
(
	const GraphEntity *e
) {
	ASSERT(e != NULL);

	return *e->attributes;
}

inline int GraphEntity_ClearAttributes
(
	GraphEntity *e
) {
	ASSERT(e != NULL);

	int count = ATTRIBUTE_SET_COUNT(*e->attributes);
	
	AttributeSet_Free(e->attributes);
	
	return count;
}
