/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>
#include "prev_decode_graph.h"
#include "../../../graph.h"

SIValue _PrevRdbLoadSIValue(RedisModuleIO *rdb) {
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
	case T_NULL:
	default: // currently impossible
		return SI_NullVal();
	}
}

void _PrevRdbLoadEntity(RedisModuleIO *rdb, GraphContext *gc, GraphEntity *e) {
	/* Format:
	 * #properties N
	 * (name, value type, value) X N
	*/
	uint64_t propCount = RedisModule_LoadUnsigned(rdb);
	if(!propCount) return;

	for(int i = 0; i < propCount; i++) {
		char *attr_name = RedisModule_LoadStringBuffer(rdb, NULL);
		SIValue attr_value = _PrevRdbLoadSIValue(rdb);
		Attribute_ID attr_id = GraphContext_GetAttributeID(gc, attr_name);
		assert(attr_id != ATTRIBUTE_NOTFOUND);
		GraphEntity_AddProperty(e, attr_id, attr_value);
		RedisModule_Free(attr_name);
	}
}

void _PrevRdbLoadNodes(RedisModuleIO *rdb, GraphContext *gc) {
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

	Graph_AllocateNodes(gc->g, nodeCount);
	for(uint64_t i = 0; i < nodeCount; i++) {
		Node n;
		// * ID
		NodeID id = RedisModule_LoadUnsigned(rdb);

		// Extend this logic when multi-label support is added.
		// * #labels M
		uint64_t nodeLabelCount = RedisModule_LoadUnsigned(rdb);

		// * (labels) x M
		// M will currently always be 0 or 1
		uint64_t l = (nodeLabelCount) ? RedisModule_LoadUnsigned(rdb) : GRAPH_NO_LABEL;
		Graph_CreateNode(gc->g, l, &n);

		_PrevRdbLoadEntity(rdb, gc, (GraphEntity *)&n);
	}
}

void _PrevRdbLoadEdges(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * #edges (N)
	 * {
	 *  edge ID, currently not in use.
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
		EdgeID edgeId = RedisModule_LoadUnsigned(rdb);
		NodeID srcId = RedisModule_LoadUnsigned(rdb);
		NodeID destId = RedisModule_LoadUnsigned(rdb);
		uint64_t relation = RedisModule_LoadUnsigned(rdb);
		assert(Graph_ConnectNodes(gc->g, srcId, destId, relation, &e));
		_PrevRdbLoadEntity(rdb, gc, (GraphEntity *)&e);
	}
}

void PrevRdbLoadGraph(RedisModuleIO *rdb, GraphContext *gc) {
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
	_PrevRdbLoadNodes(rdb, gc);

	// Load edges.
	_PrevRdbLoadEdges(rdb, gc);

	// Revert to default synchronization behavior
	Graph_SetMatrixPolicy(gc->g, SYNC_AND_MINIMIZE_SPACE);

	// Resize and flush all pending changes to matrices.
	Graph_ApplyAllPending(gc->g);
}
