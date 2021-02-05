/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "json_encoder.h"
// #include "RG.h"
#include "sds/sds.h"
#include "rmalloc.h"
#include "strutil.h"
#include "../value.h"
#include "../query_ctx.h"
#include "../datatypes/map.h"
#include "../datatypes/array.h"
#include "../graph/graphcontext.h"
#include "../graph/entities/node.h"
#include "../graph/entities/edge.h"
#include "../datatypes/path/sipath.h"

// Forward declaration
void _JsonEncoder_SIValue(SIValue v, sds *sds_str);

static void _JsonEncoder_Properties(const GraphEntity *ge, sds *sds_str) {
	sdscat(*sds_str, "\"properties\": {");
	uint prop_count = ENTITY_PROP_COUNT(ge);
	EntityProperty *properties = ENTITY_PROPS(ge);
	GraphContext *gc = QueryCtx_GetGraphCtx();
	for(uint i = 0; i < prop_count; i ++) {
		const char *key = GraphContext_GetAttributeString(gc, properties[i].id);
		sdscatfmt("\"%s\": ", key);
		_JsonEncoder_SIValue(properties[i].value, sds_str);
		if(i < prop_count - 1) sdscat(*sds_str, ", ");
	}
	sdscat(*sds_str, "}");

}

static void _JsonEncoder_Node(const Node *n, sds *sds_str) {
	sdscatfmt(*sds_str, "\"id\": %U", ENTITY_GET_ID(n));
	sdscat(*sds_str, ", \"labels\": [");
	// Label data will only be populated if provided by the query string.
	const char *label = NULL;
	GraphContext *gc = QueryCtx_GetGraphCtx();
	int labelID = Graph_GetNodeLabel(gc->g, ENTITY_GET_ID(n));
	if(labelID != GRAPH_NO_LABEL) {
		label = gc->node_schemas[labelID]->name;
		sdscatfmt(*sds_str, "\"%s\"", label);
	}
	sdscat(*sds_str, "], ");
	_JsonEncoder_Properties((const GraphEntity *)n, sds_str);
}

static void _JsonEncoder_Edge(Edge *e, sds *sds_str) {
	sdscatfmt(*sds_str, "\"id\": %U", ENTITY_GET_ID(e));
	GraphContext *gc = QueryCtx_GetGraphCtx();
	// Retrieve reltype data.
	int id = Graph_GetEdgeRelation(gc->g, e);
	const char *relationship = gc->relation_schemas[id]->name;
	sdscatfmt(*sds_str, ", \"relationship\": \"%s\", ", relationship);

	_JsonEncoder_Properties((const GraphEntity *)e, sds_str);

	sdscat(*sds_str, ", \"start\": {");
	// Retrieve source node data.
	Node src;
	Graph_GetNode(gc->g, e->srcNodeID, &src);
	_JsonEncoder_Node(&src, sds_str);

	sdscat(*sds_str, "}, \"end\": {");
	// Retrieve dest node data.
	Node dest;
	Graph_GetNode(gc->g, e->destNodeID, &dest);
	_JsonEncoder_Node(&dest, sds_str);

	sdscat(*sds_str, "}");
}

static void _JsonEncoder_GraphEntity(GraphEntity *ge, sds *sds_str, GraphEntityType type) {
	switch(type) {
	case GETYPE_NODE:
		sdscat(*sds_str, "{\"type\": \"node\", ");
		_JsonEncoder_Node((const Node *)ge, sds_str);
		break;
	case GETYPE_EDGE:
		sdscat(*sds_str, "{\"type\": \"relationship\", ");
		_JsonEncoder_Edge((Edge *)ge, sds_str);
		break;
	default:
		ASSERT(false);
	}
	sdscat(*sds_str, "}");
}

static void _JsonEncoder_Path(SIValue p, sds *sds_str) {
	// open path with "["
	sdscat(*sds_str, "[");

	size_t nodeCount = SIPath_NodeCount(p);
	for(size_t i = 0; i < nodeCount - 1; i ++) {
		// write the next value
		SIValue node = SIPath_GetNode(p, i);
		_JsonEncoder_GraphEntity((GraphEntity *)&node, sds_str, GETYPE_NODE);
		sdscat(*sds_str, ", ");
		SIValue edge = SIPath_GetRelationship(p, i);
		_JsonEncoder_GraphEntity((GraphEntity *)&edge, sds_str, GETYPE_EDGE);
		sdscat(*sds_str, ", ");
	}
	// Handle last node.
	if(nodeCount > 0) {
		SIValue node = SIPath_GetNode(p, nodeCount - 1);
		_JsonEncoder_GraphEntity((GraphEntity *)&node, sds_str, GETYPE_NODE);
	}

	// close array with "]"
	sdscat(*sds_str, "]");
}

static void _JsonEncoder_Array(SIValue list, sds *sds_str) {
	// open array with "["
	sdscat(*sds_str, "[");
	uint arrayLen = SIArray_Length(list);
	for(uint i = 0; i < arrayLen; i ++) {
		// write the next value
		_JsonEncoder_SIValue(SIArray_Get(list, i), sds_str);
		// if it is not the last element, add ", "
		if(i != arrayLen - 1) sdscat(*sds_str, ", ");
	}

	// close array with "]"
	sdscat(*sds_str, "]");
}

static void _JsonEncoder_Map(SIValue map, sds *sds_str) {
	ASSERT(SI_TYPE(map) & T_MAP);

	// "{" marks the beginning of a map
	sdscat(*sds_str, "{");

	uint key_count = Map_KeyCount(map);
	for(uint i = 0; i < key_count; i ++) {
		Pair p = map.map[i];
		// write the next key/value pair
		sdscat(*sds_str, p.key.stringval);
		sdscat(*sds_str, ": ");
		_JsonEncoder_SIValue(p.val, sds_str);
		// if this is not the last element, add ", "
		if(i != key_count - 1) sdscat(*sds_str, ", ");
	}

	// "}" marks the end of a map
	sdscat(*sds_str, "}");
}

void _JsonEncoder_SIValue(SIValue v, sds *sds_str) {
	switch(v.type) {
	case T_STRING:
		sdscat(*sds_str, v.stringval);
		break;
	case T_INT64:
		sdscatfmt(*sds_str, "%I", v.longval);
		break;
	case T_BOOL:
		if(v.longval) sdscat(*sds_str, "true");
		else sdscat(*sds_str, "false");
		break;
	case T_DOUBLE:
		sdscatprintf(*sds_str, "%f", v.doubleval);
		break;
	case T_NODE:
		_JsonEncoder_GraphEntity(v.ptrval, sds_str, GETYPE_NODE);
		break;
	case T_EDGE:
		_JsonEncoder_GraphEntity(v.ptrval, sds_str, GETYPE_EDGE);
		break;
	case T_ARRAY:
		_JsonEncoder_Array(v, sds_str);
		break;
	case T_MAP:
		_JsonEncoder_Map(v, sds_str);
		break;
	case T_PATH:
		_JsonEncoder_Path(v, sds_str);
		break;
	case T_NULL:
		sdscat(*sds_str, "null");
		break;
	default:
		// unrecognized type
		printf("unrecognized type: %d\n", v.type);
		ASSERT(false);
		break;
	}
}

char *JsonEncoder_SIValue(SIValue v) {
	sds s = sdsempty();
	_JsonEncoder_SIValue(v, &s);
	char *retval = rm_strdup(s);
	sdsfree(s);
	return retval;
}

