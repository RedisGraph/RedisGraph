/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_v11.h"

// forward declarations
static SIValue _RdbLoadPoint(IODecoder *io);
static SIValue _RdbLoadSIArray(IODecoder *io);

static SIValue _RdbLoadSIValue
(
	IODecoder *io
) {
	// Format:
	// SIType
	// Value
	SIType t = IODecoder_LoadUnsigned(io);
	switch(t) {
	case T_INT64:
		return SI_LongVal(IODecoder_LoadSigned(io));
	case T_DOUBLE:
		return SI_DoubleVal(IODecoder_LoadDouble(io));
	case T_STRING:
		// transfer ownership of the heap-allocated string to the
		// newly-created SIValue
		return SI_TransferStringVal(IODecoder_LoadStringBuffer(io, NULL));
	case T_BOOL:
		return SI_BoolVal(IODecoder_LoadSigned(io));
	case T_ARRAY:
		return _RdbLoadSIArray(io);
	case T_POINT:
		return _RdbLoadPoint(io);
	case T_NULL:
	default: // currently impossible
		return SI_NullVal();
	}
}

static SIValue _RdbLoadPoint
(
	IODecoder *io
) {
	double lat = IODecoder_LoadDouble(io);
	double lon = IODecoder_LoadDouble(io);
	return SI_Point(lat, lon);
}

static SIValue _RdbLoadSIArray
(
	IODecoder *io
) {
	/* loads array as
	   unsinged : array legnth
	   array[0]
	   .
	   .
	   .
	   array[array length -1]
	 */
	uint arrayLen = IODecoder_LoadUnsigned(io);
	SIValue list = SI_Array(arrayLen);
	for(uint i = 0; i < arrayLen; i++) {
		SIValue elem = _RdbLoadSIValue(io);
		SIArray_Append(&list, elem);
		SIValue_Free(elem);
	}
	return list;
}

static void _RdbLoadEntity
(
	IODecoder *io,
	GraphContext *gc,
	GraphEntity *e
) {
	// Format:
	// #properties N
	// (name, value type, value) X N

	uint64_t propCount = IODecoder_LoadUnsigned(io);

	for(int i = 0; i < propCount; i++) {
		Attribute_ID attr_id = IODecoder_LoadUnsigned(io);
		SIValue attr_value = _RdbLoadSIValue(io);
		GraphEntity_AddProperty(e, attr_id, attr_value);
		SIValue_Free(attr_value);
	}
}

void RdbLoadNodes_v11
(
	IODecoder *io,
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
		NodeID id = IODecoder_LoadUnsigned(io);

		// #labels M
		uint64_t nodeLabelCount = IODecoder_LoadUnsigned(io);

		// * (labels) x M
		uint64_t labels[nodeLabelCount];
		for(uint64_t i = 0; i < nodeLabelCount; i ++){
			labels[i] = IODecoder_LoadUnsigned(io);
		}

		Serializer_Graph_SetNode(gc->g, id, labels, nodeLabelCount, &n);

		_RdbLoadEntity(io, gc, (GraphEntity *)&n);

		// introduce n to each relevant index
		for (int i = 0; i < nodeLabelCount; i++) {
			Schema *s = GraphContext_GetSchemaByID(gc, labels[i], SCHEMA_NODE);
			ASSERT(s != NULL);
			if(s->index) Index_IndexNode(s->index, &n);
			if(s->fulltextIdx) Index_IndexNode(s->fulltextIdx, &n);
		}
	}
}

void RdbLoadDeletedNodes_v11
(
	IODecoder *io,
	GraphContext *gc,
	uint64_t deleted_node_count
) {
	// Format:
	// node id X N
	for(uint64_t i = 0; i < deleted_node_count; i++) {
		NodeID id = IODecoder_LoadUnsigned(io);
		Serializer_Graph_MarkNodeDeleted(gc->g, id);
	}
}

void RdbLoadEdges_v11
(
	IODecoder *io,
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
		EdgeID    edgeId    =  IODecoder_LoadUnsigned(io);
		NodeID    srcId     =  IODecoder_LoadUnsigned(io);
		NodeID    destId    =  IODecoder_LoadUnsigned(io);
		uint64_t  relation  =  IODecoder_LoadUnsigned(io);
		Serializer_Graph_SetEdge(gc->g,
				gc->decoding_context->multi_edge[relation], edgeId, srcId,
				destId, relation, &e);
		_RdbLoadEntity(io, gc, (GraphEntity *)&e);

		// index edge
		Schema *s = GraphContext_GetSchemaByID(gc, relation, SCHEMA_EDGE);
		ASSERT(s != NULL);
		if(s->index) Index_IndexEdge(s->index, &e);
		if(s->fulltextIdx) Index_IndexEdge(s->fulltextIdx, &e);
	}
}

void RdbLoadDeletedEdges_v11
(
	IODecoder *io,
	GraphContext *gc,
	uint64_t deleted_edge_count
) {
	// Format:
	// edge id X N
	for(uint64_t i = 0; i < deleted_edge_count; i++) {
		EdgeID id = IODecoder_LoadUnsigned(io);
		Serializer_Graph_MarkEdgeDeleted(gc->g, id);
	}
}
