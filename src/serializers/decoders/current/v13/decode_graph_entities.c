/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "decode_v13.h"

// forward declarations
static SIValue _RdbLoadPoint(RedisModuleIO *rdb);
static SIValue _RdbLoadSIArray(RedisModuleIO *rdb);

static SIValue _RdbLoadSIValue
(
	RedisModuleIO *rdb
) {
	// Format:
	// SIType
	// Value
	SIType t = RedisModule_LoadUnsigned(rdb);
	switch(t) {
	case T_INT64:
		return SI_LongVal(RedisModule_LoadSigned(rdb));
	case T_DOUBLE:
		return SI_DoubleVal(RedisModule_LoadDouble(rdb));
	case T_STRING:
		// transfer ownership of the heap-allocated string to the
		// newly-created SIValue
		return SI_TransferStringVal(RedisModule_LoadStringBuffer(rdb, NULL));
	case T_BOOL:
		return SI_BoolVal(RedisModule_LoadSigned(rdb));
	case T_ARRAY:
		return _RdbLoadSIArray(rdb);
	case T_POINT:
		return _RdbLoadPoint(rdb);
	case T_NULL:
	default: // currently impossible
		return SI_NullVal();
	}
}

static SIValue _RdbLoadPoint
(
	RedisModuleIO *rdb
) {
	double lat = RedisModule_LoadDouble(rdb);
	double lon = RedisModule_LoadDouble(rdb);
	return SI_Point(lat, lon);
}

static SIValue _RdbLoadSIArray
(
	RedisModuleIO *rdb
) {
	/* loads array as
	   unsinged : array legnth
	   array[0]
	   .
	   .
	   .
	   array[array length -1]
	 */
	uint arrayLen = RedisModule_LoadUnsigned(rdb);
	SIValue list = SI_Array(arrayLen);
	for(uint i = 0; i < arrayLen; i++) {
		SIValue elem = _RdbLoadSIValue(rdb);
		SIArray_Append(&list, elem);
		SIValue_Free(elem);
	}
	return list;
}

static void _RdbLoadEntity
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	GraphEntity *e
) {
	// Format:
	// #properties N
	// (name, value type, value) X N

	uint64_t propCount = RedisModule_LoadUnsigned(rdb);

	for(int i = 0; i < propCount; i++) {
		Attribute_ID attr_id = RedisModule_LoadUnsigned(rdb);
		SIValue attr_value = _RdbLoadSIValue(rdb);
		GraphEntity_AddProperty(e, attr_id, attr_value);
		SIValue_Free(attr_value);
	}
}

void RdbLoadNodes_v13
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t node_count
) {
	// Node Format:
	//      ID
	//      #labels M
	//      (labels) X M
	//      #properties N
	//      (name, value type, value) X N

	for(uint64_t i = 0; i < node_count; i++) {
		Node n;
		NodeID id = RedisModule_LoadUnsigned(rdb);

		// #labels M
		uint64_t nodeLabelCount = RedisModule_LoadUnsigned(rdb);

		// * (labels) x M
		LabelID labels[nodeLabelCount];
		for(uint64_t i = 0; i < nodeLabelCount; i ++){
			labels[i] = RedisModule_LoadUnsigned(rdb);
		}

		Serializer_Graph_SetNode(gc->g, id, labels, nodeLabelCount, &n);

		_RdbLoadEntity(rdb, gc, (GraphEntity *)&n);

		// introduce n to each relevant index
		for (int i = 0; i < nodeLabelCount; i++) {
			Schema *s = GraphContext_GetSchemaByID(gc, labels[i], SCHEMA_NODE);
			ASSERT(s != NULL);
			if(s->index) Index_IndexNode(s->index, &n);
			if(s->fulltextIdx) Index_IndexNode(s->fulltextIdx, &n);

			// Entities must satisfy their constraints.
			ASSERT(Constraints_enforce_entity(s->constraints, GraphEntity_GetAttributes((GraphEntity *)&n), Index_RSIndex(s->index)));
		}
	}
}

void RdbLoadDeletedNodes_v13
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t deleted_node_count
) {
	// Format:
	// node id X N
	for(uint64_t i = 0; i < deleted_node_count; i++) {
		NodeID id = RedisModule_LoadUnsigned(rdb);
		Serializer_Graph_MarkNodeDeleted(gc->g, id);
	}
}

void RdbLoadEdges_v13
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t edge_count
) {
	// Format:
	// {
	//  edge ID
	//  source node ID
	//  destination node ID
	//  relation type
	// } X N
	// edge properties X N

	// construct connections
	for(uint64_t i = 0; i < edge_count; i++) {
		Edge e;
		EdgeID    edgeId    =  RedisModule_LoadUnsigned(rdb);
		NodeID    srcId     =  RedisModule_LoadUnsigned(rdb);
		NodeID    destId    =  RedisModule_LoadUnsigned(rdb);
		uint64_t  relation  =  RedisModule_LoadUnsigned(rdb);
		Serializer_Graph_SetEdge(gc->g,
				gc->decoding_context->multi_edge[relation], edgeId, srcId,
				destId, relation, &e);
		_RdbLoadEntity(rdb, gc, (GraphEntity *)&e);

		// index edge
		Schema *s = GraphContext_GetSchemaByID(gc, relation, SCHEMA_EDGE);
		ASSERT(s != NULL);
		if(s->index) Index_IndexEdge(s->index, &e);
		if(s->fulltextIdx) Index_IndexEdge(s->fulltextIdx, &e);

		// Entities must satisfy their constraints.
		ASSERT(Constraints_enforce_entity(s->constraints, GraphEntity_GetAttributes((GraphEntity *)&e), Index_RSIndex(s->index)));
	}
}

void RdbLoadDeletedEdges_v13
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t deleted_edge_count
) {
	// Format:
	// edge id X N
	for(uint64_t i = 0; i < deleted_edge_count; i++) {
		EdgeID id = RedisModule_LoadUnsigned(rdb);
		Serializer_Graph_MarkEdgeDeleted(gc->g, id);
	}
}
