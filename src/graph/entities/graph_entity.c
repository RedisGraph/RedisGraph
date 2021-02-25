/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "graph_entity.h"
#include "node.h"
#include "edge.h"
#include "../../errors.h"
#include "../../query_ctx.h"
#include "../graphcontext.h"
#include "../../util/rmalloc.h"

inline bool GraphEntity_IsDeleted
(
	const GraphEntity *e
) {
	return Graph_EntityIsDeleted(e->entity);
}


static AttributeSet *_GraphEntity_GetAttrSet
(
	const GraphEntity *ge
) {
	if(ge->entity->sets == NULL) return NULL;
	return ge->entity->sets;
}

// removes entity's property
static bool _GraphEntity_RemoveProperty
(
	const GraphEntity *e,
	Attribute_ID attr_id
) {
	bool removed = false;

	if(attr_id != ATTRIBUTE_UNKNOWN) {
		AttributeSet *s = _GraphEntity_GetAttrSet(e);
		ASSERT(s != NULL);
		AttributeSet_RemoveAttr(s, attr_id, &removed);
	}

	return removed;
}

int GraphEntity_ClearProperties(GraphEntity *e) {
	ASSERT(e);

	int prop_count = e->entity->prop_count;
	for(int i = 0; i < prop_count; i++) {
		// free all allocated properties
		SIValue_Free(e->entity->properties[i].value);
	}
	e->entity->prop_count = 0;

	// free and NULL-set the properties bag.
	rm_free(e->entity->properties);
	e->entity->properties = NULL;

	return prop_count;
}

const AttributeSet GraphEntity_GetAttributeSet
(
	const GraphEntity *e
) {
	const AttributeSet *s = NULL;
	AttributeSet *set = _GraphEntity_GetAttrSet(e);
	if(set != NULL) s = (const AttributeSet) * set;

	return s;
}

uint GraphEntity_PropCount
(
	const GraphEntity *e  // entity to get attribute count from
) {
	ASSERT(e != NULL);

	uint attr_count = 0;
	AttributeSet *s = _GraphEntity_GetAttrSet(e);
	if(s != NULL) attr_count = AttributeSet_AttributeCount(*s);
	return attr_count;
}

// add a new property to entity
void GraphEntity_AddProperty
(
	GraphEntity *e,
	Attribute_ID attr_id,
	SIValue value
) {
	ASSERT(e != NULL);
	ASSERT(SIValue_IsNull(value) == false);

	AttributeSet *s = _GraphEntity_GetAttrSet(e);
	ASSERT(s != NULL);

	AttributeSet_SetAttr(s, attr_id, value);
}

SIValue *GraphEntity_GetProperty
(
	const GraphEntity *e,
	Attribute_ID attr_id
) {
	if(attr_id == ATTRIBUTE_UNKNOWN) return ATTRIBUTE_NOTFOUND;

	if(ENTITY_GET_ID(e) == INVALID_ENTITY_ID) {
		/* The internal entity pointer should only be NULL if the entity
		 * is in an intermediate state, such as a node scheduled for creation.
		 * Note that this exception may cause memory to be leaked in the caller. */
		ErrorCtx_SetError("Attempted to access undefined property");
		return ATTRIBUTE_NOTFOUND;
	}

	AttributeSet *s = _GraphEntity_GetAttrSet(e);
	if(s == NULL) return ATTRIBUTE_NOTFOUND;
	return AttributeSet_GetAttr(*s, attr_id);
}

// updates existing property value
bool GraphEntity_SetProperty
(
	const GraphEntity *e,
	Attribute_ID attr_id,
	SIValue value
) {
	ASSERT(e != NULL);

	// setting an attribute value to NULL removes that attribute
	if(SIValue_IsNull(value)) return _GraphEntity_RemoveProperty(e, attr_id);

	AttributeSet *s = _GraphEntity_GetAttrSet(e);
	ASSERT(s != NULL);

	SIValue *current = AttributeSet_GetAttr(*s, attr_id);
	ASSERT(current != ATTRIBUTE_NOTFOUND);

	// compare current value to new value, only update if current != new
	if(SIValue_Compare(*current, value, NULL) == 0) return false;

	// value != current, update entity
	AttributeSet_SetAttr(s, attr_id, value);
	return true;
}

size_t GraphEntity_PropertiesToString
(
	const GraphEntity *e,
	char **buffer,
	size_t *bufferLen,
	size_t *bytesWritten
) {
	ASSERT(e != NULL);
	ASSERT(bytesWritten != NULL);

	*bytesWritten = 0;

	AttributeSet *s = _GraphEntity_GetAttrSet(e);
	if(s == NULL) return *bytesWritten;

	// make sure there is enough space for "{...}\0"
	if(*bufferLen - *bytesWritten < 64) {
		*bufferLen += 64;
		*buffer = rm_realloc(*buffer, *bufferLen);
	}

	*bytesWritten += snprintf(*buffer, *bufferLen, "{");
	GraphContext *gc = QueryCtx_GetGraphCtx();

	int attr_count = AttributeSet_AttributeCount(*s);

	for(int i = 0; i < attr_count; i++) {
		SIValue val;
		Attribute_ID id;

		AttributeSet_GetAttrIdx(*s, i, &val, &id);

		ASSERT(!SIValue_IsNull(val));
		ASSERT(id != ATTRIBUTE_NOTFOUND);

		// print key
		const char *key = GraphContext_GetAttributeString(gc, id);

		// check for enough space
		size_t keyLen = strlen(key);
		if(*bufferLen - *bytesWritten < keyLen) {
			*bufferLen += keyLen;
			*buffer = rm_realloc(*buffer, *bufferLen);
		}

		*bytesWritten +=
			snprintf(*buffer + *bytesWritten, *bufferLen, "%s:", key);

		// print value
		SIValue_ToString(val, buffer, bufferLen, bytesWritten);

		// if not the last element print ", "
		if(i != attr_count - 1) {
			*bytesWritten = snprintf(*buffer + *bytesWritten, *bufferLen, ", ");
		}
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
				if(n->label) {
					// allocate space if needed
					size_t labelLen = strlen(n->label);
					if(*bufferLen - *bytesWritten < labelLen) {
						*bufferLen += labelLen;
						*buffer = rm_realloc(*buffer, sizeof(char) * *bufferLen);
					}
					*bytesWritten += snprintf(*buffer + *bytesWritten, *bufferLen, ":%s", n->label);
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

void Entity_Free(Entity *e) {
	ASSERT(e != NULL);

	// free attribute sets
	AttributeSet *sets = e->sets;
	uint attr_set_count = array_len(sets);
	for(uint i = 0; i < attr_set_count; i++) {
		AttributeSet s = array_pop(sets);
		AttributeSet_Free(s);
	}
	array_free(sets);
}

