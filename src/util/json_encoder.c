/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "json_encoder.h"
// #include "RG.h"
#include "rmalloc.h"
#include "strutil.h"
#include "sds/sds.h"
#include "../value.h"
#include "../query_ctx.h"
#include "../datatypes/map.h"
#include "../datatypes/array.h"
#include "../graph/graphcontext.h"
#include "../graph/entities/node.h"
#include "../graph/entities/edge.h"
#include "../datatypes/path/sipath.h"

static inline void _JsonEncoder_String(SIValue str, char **buf, size_t *bufferLen,
									   size_t *bytesWritten) {
	size_t strLen = strlen(str.stringval) + 2;
	if(*bufferLen - *bytesWritten < strLen) {
		*bufferLen += strLen;
		*buf = rm_realloc(*buf, *bufferLen);
	}
	// Print string to buffer, interpolating with quotes if specified.
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "\"%s\"", str.stringval);
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
		JsonEncoder_SIValue(properties[i].value, buf, bufferLen, bytesWritten);
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
	if(label) {
		if(*bufferLen - *bytesWritten < 64 + strlen(label)) {
			str_ExtendBuffer(buf, bufferLen, 64 + strlen(label));
		}
		*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "\"%s\"", label);
	}
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

static void _JsonEncoder_GraphEntity(GraphEntity *ge, char **buf, size_t *bufferLen,
									 size_t *bytesWritten, GraphEntityType type) {
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

static void _JsonEncoder_Path(SIValue p, char **buf, size_t *bufferLen, size_t *bytesWritten) {
	// 64 is defiend arbitrarily.
	if(*bufferLen - *bytesWritten < 64) {
		*bufferLen += 64;
		*buf = rm_realloc(*buf, sizeof(char) * *bufferLen);
	}
	// open path with "["
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "[");

	size_t nodeCount = SIPath_NodeCount(p);
	for(size_t i = 0; i < nodeCount - 1; i ++) {
		// write the next value
		SIValue node = SIPath_GetNode(p, i);
		_JsonEncoder_GraphEntity((GraphEntity *)&node, buf, bufferLen, bytesWritten, GETYPE_NODE);
		* bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, ", ");
		SIValue edge = SIPath_GetRelationship(p, i);
		_JsonEncoder_GraphEntity((GraphEntity *)&edge, buf, bufferLen, bytesWritten, GETYPE_EDGE);
		* bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, ", ");
	}
	// Handle last node.
	if(nodeCount > 0) {
		SIValue node = SIPath_GetNode(p, nodeCount - 1);
		_JsonEncoder_GraphEntity((GraphEntity *)&node, buf, bufferLen, bytesWritten, GETYPE_NODE);
	}

	if(*bufferLen - *bytesWritten < 2) {
		*bufferLen += 2;
		*buf = rm_realloc(*buf, sizeof(char) * *bufferLen);
	}
	// close array with "]"
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "]");
}

static void _JsonEncoder_Array(SIValue list, char **buf, size_t *bufferLen, size_t *bytesWritten) {
	if(*bufferLen - *bytesWritten < 64) {
		*bufferLen += 64;
		*buf = rm_realloc(*buf, sizeof(char) * *bufferLen);
	}
	// open array with "["
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "[");
	uint arrayLen = SIArray_Length(list);
	for(uint i = 0; i < arrayLen; i ++) {
		// write the next value
		JsonEncoder_SIValue(SIArray_Get(list, i), buf, bufferLen, bytesWritten);
		// if it is the last element, add ", "
		if(i != arrayLen - 1) *bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, ", ");
	}
	if(*bufferLen - *bytesWritten < 2) {
		*bufferLen += 2;
		*buf = rm_realloc(*buf, sizeof(char) * *bufferLen);
	}
	// close array with "]"
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "]");
}

static void _JsonEncoder_Map(SIValue map, char **buf, size_t *bufferLen, size_t *bytesWritten) {
	ASSERT(SI_TYPE(map) & T_MAP);
	ASSERT(buf != NULL);
	ASSERT(bufferLen != NULL);
	ASSERT(bytesWritten != NULL);

	// resize buffer if buffer length is less than 64
	if(*bufferLen - *bytesWritten < 64) str_ExtendBuffer(buf, bufferLen, 64);

	uint key_count = Map_KeyCount(map);

	// "{" marks the beginning of a map
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "{");

	for(uint i = 0; i < key_count; i ++) {
		Pair p = map.map[i];
		// write the next key/value pair
		_JsonEncoder_String(p.key, buf, bufferLen, bytesWritten);
		if(*bufferLen - *bytesWritten < 64) str_ExtendBuffer(buf, bufferLen, 64);
		*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, ": ");
		JsonEncoder_SIValue(p.val, buf, bufferLen, bytesWritten);
		// if this is not the last element, add ", "
		if(i != key_count - 1) {
			if(*bufferLen - *bytesWritten < 64) str_ExtendBuffer(buf, bufferLen, 64);
			*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, ", ");
		}
	}

	// make sure there's enough space for "}"
	if(*bufferLen - *bytesWritten < 2) str_ExtendBuffer(buf, bufferLen, 2);

	// "}" marks the end of a map
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "}");
}

void JsonEncoder_SIValue(SIValue v, char **buf, size_t *bufferLen, size_t *bytesWritten) {
	// uint64 max and int64 min string representation requires 21 bytes
	// float defaults to print 6 digit after the decimal-point
	// checkt for enough space
	if(*bufferLen - *bytesWritten < 64) {
		*bufferLen += 64;
		*buf = rm_realloc(*buf, sizeof(char) * *bufferLen);
	}

	switch(v.type) {
	case T_STRING:
		_JsonEncoder_String(v, buf, bufferLen, bytesWritten);
		break;
	case T_INT64:
		*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "%lld", (long long)v.longval);
		break;
	case T_BOOL:
		*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "%s", v.longval ? "true" : "false");
		break;
	case T_DOUBLE:
		*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "%f", v.doubleval);
		break;
	case T_NODE:
		_JsonEncoder_GraphEntity(v.ptrval, buf, bufferLen, bytesWritten, GETYPE_NODE);
		break;
	case T_EDGE:
		_JsonEncoder_GraphEntity(v.ptrval, buf, bufferLen, bytesWritten, GETYPE_EDGE);
		break;
	case T_ARRAY:
		_JsonEncoder_Array(v, buf, bufferLen, bytesWritten);
		break;
	case T_MAP:
		_JsonEncoder_Map(v, buf, bufferLen, bytesWritten);
		break;
	case T_PATH:
		_JsonEncoder_Path(v, buf, bufferLen, bytesWritten);
		break;
	case T_NULL:
		*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "null");
		break;
	default:
		// unrecognized type
		printf("unrecognized type: %d\n", v.type);
		ASSERT(false);
		break;
	}
}

