/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graph_entity.h"
#include "node.h"
#include "edge.h"
#include "../../RG.h"
#include "../../errors.h"
#include "../../query_ctx.h"
#include "../graphcontext.h"
#include "../../util/rmalloc.h"
#include "../../util/strutil.h"

SIValue *PROPERTY_NOTFOUND = &(SIValue) {
	.longval = 0, .type = T_NULL
};

/* Removes entity's property. */
static bool _GraphEntity_RemoveProperty(const GraphEntity *e, Attribute_ID attr_id) {
	// Quick return if attribute is missing.
	if(attr_id == ATTRIBUTE_NOTFOUND) return false;

	// Locate attribute position.
	int prop_count = e->entity->prop_count;
	for(int i = 0; i < prop_count; i++) {
		if(attr_id == e->entity->properties[i].id) {
			SIValue_Free(e->entity->properties[i].value);
			e->entity->prop_count--;

			if(e->entity->prop_count == 0) {
				/* Only attribute removed, free properties bag. */
				rm_free(e->entity->properties);
				e->entity->properties = NULL;
			} else {
				/* Overwrite deleted attribute with the last
				 * attribute and shrink properties bag. */
				e->entity->properties[i] = e->entity->properties[prop_count - 1];
				e->entity->properties = rm_realloc(e->entity->properties,
												   sizeof(EntityProperty) * e->entity->prop_count);
			}

			return true;
		}
	}

	return false;
}

/* Add a new property to entity */
bool GraphEntity_AddProperty(GraphEntity *e, Attribute_ID attr_id, SIValue value) {
	ASSERT(e);
	if(SIValue_IsNull(value)) return false;

	if(e->entity->properties == NULL) {
		e->entity->properties = rm_malloc(sizeof(EntityProperty));
	} else {
		e->entity->properties = rm_realloc(e->entity->properties,
										   sizeof(EntityProperty) * (e->entity->prop_count + 1));
	}

	int prop_idx = e->entity->prop_count;
	e->entity->properties[prop_idx].id = attr_id;
	e->entity->properties[prop_idx].value = SI_CloneValue(value);
	e->entity->prop_count++;

	return true;
}

SIValue *GraphEntity_GetProperty(const GraphEntity *e, Attribute_ID attr_id) {
	if(attr_id == ATTRIBUTE_NOTFOUND) return PROPERTY_NOTFOUND;
	if(e->entity == NULL) {
		/* The internal entity pointer should only be NULL if the entity
		 * is in an intermediate state, such as a node scheduled for creation.
		 * Note that this exception may cause memory to be leaked in the caller. */
		ASSERT(e->id == INVALID_ENTITY_ID);
		ErrorCtx_SetError("Attempted to access undefined property");
		return PROPERTY_NOTFOUND;
	}

	for(int i = 0; i < e->entity->prop_count; i++) {
		if(attr_id == e->entity->properties[i].id) {
			// Note, unsafe as entity properties can get reallocated.
			return &(e->entity->properties[i].value);
		}
	}

	return PROPERTY_NOTFOUND;
}

// Updates existing property value.
bool GraphEntity_SetProperty(const GraphEntity *e, Attribute_ID attr_id, SIValue value) {
	ASSERT(e);

	// Setting an attribute value to NULL removes that attribute.
	if(SIValue_IsNull(value)) return _GraphEntity_RemoveProperty(e, attr_id);

	SIValue *current = GraphEntity_GetProperty(e, attr_id);
	ASSERT(current != PROPERTY_NOTFOUND);

	// compare current value to new value, only update if current != new
	if(SIValue_Compare(*current, value, NULL) == 0) return false;

	// value != current, update entity
	SIValue_Free(*current);
	*current = SI_CloneValue(value);
	return true;
}

size_t GraphEntity_PropertiesToString(const GraphEntity *e, char **buffer, size_t *bufferLen,
									  size_t *bytesWritten) {
	// make sure there is enough space for "{...}\0"
	if(*bufferLen - *bytesWritten < 64) {
		*bufferLen += 64;
		*buffer = rm_realloc(*buffer, *bufferLen);
	}
	*bytesWritten += snprintf(*buffer, *bufferLen, "{");
	GraphContext *gc = QueryCtx_GetGraphCtx();
	int propCount = ENTITY_PROP_COUNT(e);
	EntityProperty *properties = ENTITY_PROPS(e);
	for(int i = 0; i < propCount; i++) {
		// print key
		const char *key = GraphContext_GetAttributeString(gc, properties[i].id);
		// check for enough space
		size_t keyLen = strlen(key);
		if(*bufferLen - *bytesWritten < keyLen) {
			*bufferLen += keyLen;
			*buffer = rm_realloc(*buffer, *bufferLen);
		}
		*bytesWritten += snprintf(*buffer + *bytesWritten, *bufferLen, "%s:", key);

		// print value
		SIValue_ToString(properties[i].value, buffer, bufferLen, bytesWritten, false);

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

void GraphEntity_ToString(const GraphEntity *e, char **buffer, size_t *bufferLen,
						  size_t *bytesWritten,
						  GraphEntityStringFromat format, GraphEntityType entityType) {
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
		*bytesWritten += snprintf(*buffer + *bytesWritten, *bufferLen, "%llu", ENTITY_GET_ID(e));
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

static void _PropertiesToJSON(const GraphEntity *ge, char **buf, size_t *bufferLen,
							  size_t *bytesWritten) {

	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "\"properties\": {");
	uint prop_count = ENTITY_PROP_COUNT(ge);
	EntityProperty *properties = ENTITY_PROPS(ge);
	GraphContext *gc = QueryCtx_GetGraphCtx();
	for(uint i = 0; i < prop_count; i ++) {
		const char *key = GraphContext_GetAttributeString(gc, properties[i].id);
		if(*bufferLen - *bytesWritten < 64 + strlen(key)) {
			str_ExtendBuffer(buf, bufferLen, 64 + strlen(key));
		}
		*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "\"%s\": ", key);
		SIValue_ToString(properties[i].value, buf, bufferLen, bytesWritten, true);
		if(i < prop_count - 1) *bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, ", ");
	}
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "}");

}

static void _Node_ToJSON(const Node *n, char **buf, size_t *bufferLen, size_t *bytesWritten) {
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "\"id\": %lu, ", ENTITY_GET_ID(n));
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "\"labels\": [");
	// Label data will only be populated if provided by the query string.
	const char *label = NULL;
	GraphContext *gc = QueryCtx_GetGraphCtx();
	int labelID = Graph_GetNodeLabel(gc->g, ENTITY_GET_ID(n));
	if(labelID != GRAPH_NO_LABEL) label = gc->node_schemas[labelID]->name;
	if(*bufferLen - *bytesWritten < 64 + strlen(label)) {
		str_ExtendBuffer(buf, bufferLen, 64 + strlen(label));
	}
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "\"%s\"", label);
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "], ");
	_PropertiesToJSON((const GraphEntity *)n, buf, bufferLen, bytesWritten);
}

static void _Edge_ToJSON(Edge *e, char **buf, size_t *bufferLen, size_t *bytesWritten) {
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "\"id\": %lu, ", ENTITY_GET_ID(e));
	GraphContext *gc = QueryCtx_GetGraphCtx();
	// Retrieve reltype data.
	int id = Graph_GetEdgeRelation(gc->g, e);
	const char *relationship = gc->relation_schemas[id]->name;
	if(*bufferLen - *bytesWritten < 64 + strlen(relationship)) {
		str_ExtendBuffer(buf, bufferLen, 64 + strlen(relationship));
	}
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "\"relationship\": \"%s\", ",
							  relationship);
	_PropertiesToJSON((const GraphEntity *)e, buf, bufferLen, bytesWritten);

	if(*bufferLen - *bytesWritten < 64) str_ExtendBuffer(buf, bufferLen, 64);
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, ", \"start\": {");
	// Retrieve source node data.
	Node src;
	Graph_GetNode(gc->g, e->srcNodeID, &src);
	_Node_ToJSON(&src, buf, bufferLen, bytesWritten);

	if(*bufferLen - *bytesWritten < 64) str_ExtendBuffer(buf, bufferLen, 64);
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "}, \"end\": {");
	// Retrieve dest node data.
	Node dest;
	Graph_GetNode(gc->g, e->destNodeID, &dest);
	_Node_ToJSON(&dest, buf, bufferLen, bytesWritten);

	if(*bufferLen - *bytesWritten < 2) str_ExtendBuffer(buf, bufferLen, 2);
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "}");
}

void GraphEntity_ToJSON(GraphEntity *ge, char **buf, size_t *bufferLen, size_t *bytesWritten,
						GraphEntityType type) {
	// resize buffer if buffer length is less than 64
	if(*bufferLen - *bytesWritten < 64) str_ExtendBuffer(buf, bufferLen, 64);
	switch(type) {
	case GETYPE_NODE:
		*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "{\"type\": \"node\", ");
		_Node_ToJSON((const Node *)ge, buf, bufferLen, bytesWritten);
		break;
	case GETYPE_EDGE:
		*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "{\"type\": \"relationship\", ");
		_Edge_ToJSON((Edge *)ge, buf, bufferLen, bytesWritten);
		break;
	default:
		ASSERT(false);
	}
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "}");
}

inline bool GraphEntity_IsDeleted(const GraphEntity *e) {
	return Graph_EntityIsDeleted(e->entity);
}

void FreeEntity(Entity *e) {
	ASSERT(e);
	if(e->properties != NULL) {
		for(int i = 0; i < e->prop_count; i++) SIValue_Free(e->properties[i].value);
		rm_free(e->properties);
		e->properties = NULL;
		e->prop_count = 0;
	}
}

