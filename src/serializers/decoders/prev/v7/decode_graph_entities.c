/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_v7.h"

// Forward declarations.
static SIValue _RdbLoadSIArray(RedisModuleIO *rdb);

static SIValue _RdbLoadSIValue(RedisModuleIO *rdb) {
	/* Format:
	 * SIType
	 * Value */
	SIType t = RedisModule_LoadUnsigned(rdb);
	switch(t) {
	case T_INT64:
		return SI_LongVal(RedisModule_LoadSigned(rdb));
	case T_DOUBLE:
		return SI_DoubleVal(RedisModule_LoadDouble(rdb));
	case T_STRING:
		// Transfer ownership of the heap-allocated string to the
		// newly-created SIValue
		return SI_TransferStringVal(RedisModule_LoadStringBuffer(rdb, NULL));
	case T_BOOL:
		return SI_BoolVal(RedisModule_LoadSigned(rdb));
	case T_ARRAY:
		return _RdbLoadSIArray(rdb);
	case T_NULL:
	default: // currently impossible
		return SI_NullVal();
	}
}

static SIValue _RdbLoadSIArray(RedisModuleIO *rdb) {
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
		SIArray_Append(&list, _RdbLoadSIValue(rdb));
	}
	return list;
}

static void _RdbLoadEntity(RedisModuleIO *rdb, GraphContext *gc, GraphEntity *e) {
	/* Format:
	 * #properties N
	 * (name, value type, value) X N
	*/
	uint64_t propCount = RedisModule_LoadUnsigned(rdb);

	for(int i = 0; i < propCount; i++) {
		Attribute_ID attr_id = RedisModule_LoadUnsigned(rdb);
		SIValue attr_value = _RdbLoadSIValue(rdb);
		GraphEntity_AddProperty(e, attr_id, attr_value);
		SIValue_Free(attr_value);
	}
}


void RdbLoadNodes_v7(RedisModuleIO *rdb, GraphContext *gc, uint64_t node_count) {
	/* Node Format:
	 *      ID
	 *      #labels M
	 *      (labels) X M
	 *      #properties N
	 *      (name, value type, value) X N
	 */

	for(uint64_t i = 0; i < node_count; i++) {
		Node n;
		NodeID id = RedisModule_LoadUnsigned(rdb);

		// Extend this logic when multi-label support is added.
		// #labels M
		uint64_t nodeLabelCount = RedisModule_LoadUnsigned(rdb);

		// * (labels) x M
		// M will currently always be 0 or 1
		uint64_t l = (nodeLabelCount) ? RedisModule_LoadUnsigned(rdb) : GRAPH_NO_LABEL;
		Serializer_Graph_SetNode(gc->g, id, l, &n);

		_RdbLoadEntity(rdb, gc, (GraphEntity *)&n);
	}
}

void RdbLoadDeletedNodes_v7(RedisModuleIO *rdb, GraphContext *gc, uint64_t deleted_node_count) {
	/* Format:
	* node id X N */
	for(uint64_t i = 0; i < deleted_node_count; i++) {
		NodeID id = RedisModule_LoadUnsigned(rdb);
		Serializer_Graph_MarkNodeDeleted(gc->g, id);
	}
}

void RdbLoadEdges_v7(RedisModuleIO *rdb, GraphContext *gc, uint64_t edge_count) {
	/* Format:
	 * {
	 *  edge ID
	 *  source node ID
	 *  destination node ID
	 *  relation type
	 * } X N
	 * edge properties X N */

	// Construct connections.
	for(uint64_t i = 0; i < edge_count; i++) {
		Edge e;
		EdgeID edgeId = RedisModule_LoadUnsigned(rdb);
		NodeID srcId = RedisModule_LoadUnsigned(rdb);
		NodeID destId = RedisModule_LoadUnsigned(rdb);
		uint64_t relation = RedisModule_LoadUnsigned(rdb);
		Serializer_Graph_SetEdge(gc->g, edgeId, srcId, destId, relation, &e);
		_RdbLoadEntity(rdb, gc, (GraphEntity *)&e);
	}
}

void RdbLoadDeletedEdges_v7(RedisModuleIO *rdb, GraphContext *gc, uint64_t deleted_edge_count) {
	/* Format:
	 * edge id X N */
	for(uint64_t i = 0; i < deleted_edge_count; i++) {
		EdgeID id = RedisModule_LoadUnsigned(rdb);
		Serializer_Graph_MarkEdgeDeleted(gc->g, id);
	}
}
