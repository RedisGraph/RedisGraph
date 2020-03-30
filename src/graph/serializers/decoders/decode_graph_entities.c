/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_graph_entities.h"
#include "../../graph.h"
#include "../../../datatypes/array.h"

// Forward declerations.
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
	if(!propCount) return;

	for(int i = 0; i < propCount; i++) {
		Attribute_ID attr_id  = RedisModule_LoadUnsigned(rdb);
		SIValue attr_value = _RdbLoadSIValue(rdb);
		GraphEntity_AddProperty(e, attr_id, attr_value);
		SIValue_Free(attr_value);
	}
}


void RdbLoadNodes(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * #nodes
	 *      ID
	 *      #labels M
	 *      (labels) X M
	 *      #properties N
	 *      (name, value type, value) X N
	*/

	uint64_t nodeCount = RedisModule_LoadUnsigned(rdb);
	if(nodeCount == 0) return;

	for(uint64_t i = 0; i < nodeCount; i++) {
		Node n;
		NodeID id = RedisModule_LoadUnsigned(rdb);

		// Extend this logic when multi-label support is added.
		// #labels M
		uint64_t nodeLabelCount = RedisModule_LoadUnsigned(rdb);

		// * (labels) x M
		// M will currently always be 0 or 1
		uint64_t l = (nodeLabelCount) ? RedisModule_LoadUnsigned(rdb) : GRAPH_NO_LABEL;
		Graph_SetNode(gc->g, id, l, &n);

		_RdbLoadEntity(rdb, gc, (GraphEntity *)&n);
	}
}
void RdbLoadDeletedNodes(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	* #deleted nodes N
	* node id X N */
	uint64_t deleted_node_count = RedisModule_LoadUnsigned(rdb);
	for(uint64_t i = 0; i < deleted_node_count; i++) {
		NodeID id = RedisModule_LoadUnsigned(rdb);
		Graph_MarkNodeDeleted(gc->g, id);
	}
}
void RdbLoadEdges(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * #edges (N)
	 * {
	 *  edge ID
	 *  source node ID
	 *  destination node ID
	 *  relation type
	 * } X N
	 * edge properties X N */

	uint64_t edgeCount = RedisModule_LoadUnsigned(rdb);

	// Construct connections.
	for(int i = 0; i < edgeCount; i++) {
		Edge e;
		EdgeID edgeId = RedisModule_LoadUnsigned(rdb);
		NodeID srcId = RedisModule_LoadUnsigned(rdb);
		NodeID destId = RedisModule_LoadUnsigned(rdb);
		uint64_t relation = RedisModule_LoadUnsigned(rdb);
		Graph_SetEdge(gc->g, edgeId, srcId, destId, relation, &e);
		_RdbLoadEntity(rdb, gc, (GraphEntity *)&e);
	}

}
void RdbLoadDeletedEdges(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * #deleted edges N
	 * edge id X N */
	uint64_t deleted_edge_count = RedisModule_LoadUnsigned(rdb);
	for(uint64_t i = 0; i < deleted_edge_count; i++) {
		EdgeID id = RedisModule_LoadUnsigned(rdb);
		Graph_MarkEdgeDeleted(gc->g, id);
	}
}