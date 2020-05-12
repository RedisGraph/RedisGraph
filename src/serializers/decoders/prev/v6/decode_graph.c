/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_v6.h"
#include <assert.h>

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

SIValue _RdbLoadSIArray(RedisModuleIO *rdb) {
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
		char *attr_name = RedisModule_LoadStringBuffer(rdb, NULL);
		SIValue attr_value = _RdbLoadSIValue(rdb);
		Attribute_ID attr_id = GraphContext_GetAttributeID(gc, attr_name);
		assert(attr_id != ATTRIBUTE_NOTFOUND);
		GraphEntity_AddProperty(e, attr_id, attr_value);
		SIValue_Free(attr_value);
		RedisModule_Free(attr_name);
	}
}

static void _RdbLoadNodes(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * #nodes
	 *      #labels M
	 *      (labels) X M
	 *      #properties N
	 *      (name, value type, value) X N
	*/

	uint64_t nodeCount = RedisModule_LoadUnsigned(rdb);
	if(nodeCount == 0) return;

	Graph_AllocateNodes(gc->g, nodeCount);
	for(uint64_t i = 0; i < nodeCount; i++) {
		Node n;

		// Extend this logic when multi-label support is added.
		// #labels M
		uint64_t nodeLabelCount = RedisModule_LoadUnsigned(rdb);

		// * (labels) x M
		// M will currently always be 0 or 1
		uint64_t l = (nodeLabelCount) ? RedisModule_LoadUnsigned(rdb) : GRAPH_NO_LABEL;
		Graph_CreateNode(gc->g, l, &n);

		_RdbLoadEntity(rdb, gc, (GraphEntity *)&n);
	}
}

static void _RdbLoadEdges(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * #edges (N)
	 * {
	 *  source node ID
	 *  destination node ID
	 *  relation type
	 * } X N
	 * edge properties X N */

	uint64_t edgeCount = RedisModule_LoadUnsigned(rdb);
	if(edgeCount == 0) return;

	Graph_AllocateEdges(gc->g, edgeCount);
	// Construct connections.
	for(int i = 0; i < edgeCount; i++) {
		Edge e;
		NodeID srcId = RedisModule_LoadUnsigned(rdb);
		NodeID destId = RedisModule_LoadUnsigned(rdb);
		uint64_t relation = RedisModule_LoadUnsigned(rdb);
		assert(Graph_ConnectNodes(gc->g, srcId, destId, relation, &e));
		_RdbLoadEntity(rdb, gc, (GraphEntity *)&e);
	}
}

void RdbLoadGraph_v6(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * #nodes
	 *      #labels M
	 *      (labels) X M
	 *      #properties N
	 *      (name, value type, value) X N
	 *
	 * #edges
	 *      relation type
	 *      source node ID
	 *      destination node ID
	 *      #properties N
	 *      (name, value type, value) X N
	 */

	// While loading the graph, minimize matrix realloc and synchronization calls.
	Graph_SetMatrixPolicy(gc->g, RESIZE_TO_CAPACITY);

	// Load nodes.
	_RdbLoadNodes(rdb, gc);

	// Load edges.
	_RdbLoadEdges(rdb, gc);

	// Revert to default synchronization behavior
	Graph_SetMatrixPolicy(gc->g, SYNC_AND_MINIMIZE_SPACE);

	// Resize and flush all pending changes to matrices.
	Graph_ApplyAllPending(gc->g);
}

