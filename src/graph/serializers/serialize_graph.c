/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>
#include "../graph.h"
#include "serialize_graph.h"
#include "../../util/arr.h"
#include "../../util/qsort.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

// Use a modified binary search to find the number of elements in
// the array less than the input ID.
// This value is the number the input ID should be decremented by.
static uint64_t shiftCount(uint64_t *array, NodeID id) {
	uint32_t deletedIndicesCount = array_len(array);
	uint32_t left = 0;
	uint32_t right = deletedIndicesCount;
	uint32_t pos;
	while(left < right) {
		pos = (right + left) / 2;
		if(array[pos] < id) {
			left = pos + 1;
		} else {
			right = pos;
		}
	}
	return left;
}

static NodeID _updatedID(uint64_t *array, NodeID id) {
	uint32_t itemCount = array_len(array);
	if(itemCount == 0) {
		// No deleted elements; don't modify ID
		return id;
	} else if(id > array[itemCount - 1]) {
		// ID is greater than all deleted elements, reduce by deleted count
		return id - itemCount;
	} else if(id < array[0]) {
		// ID is lower than all deleted elements, don't modify
		return id;
	} else {
		// Shift ID left by number of deleted IDs lower than it
		return id - shiftCount(array, id);
	}
}

SIValue _RdbLoadSIValue(RedisModuleIO *rdb) {
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
	case T_CONSTSTRING:
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

void _RdbLoadEntity(RedisModuleIO *rdb, GraphContext *gc, GraphEntity *e) {
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
		RedisModule_Free(attr_name);
	}
}

void _RdbLoadNodes(RedisModuleIO *rdb, GraphContext *gc) {
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

		_RdbLoadEntity(rdb, gc, (GraphEntity *)&n);
	}
}

void _RdbLoadEdges(RedisModuleIO *rdb, GraphContext *gc) {
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
		_RdbLoadEntity(rdb, gc, (GraphEntity *)&e);
	}
}

void _RdbSaveSIValue(RedisModuleIO *rdb, const SIValue *v) {
	/* Format:
	 * SIType
	 * Value */
	RedisModule_SaveUnsigned(rdb, v->type);
	switch(v->type) {
	case T_BOOL:
	case T_INT64:
		RedisModule_SaveSigned(rdb, v->longval);
		return;
	case T_DOUBLE:
		RedisModule_SaveDouble(rdb, v->doubleval);
		return;
	case T_STRING:
		RedisModule_SaveStringBuffer(rdb, v->stringval, strlen(v->stringval) + 1);
		return;
	case T_NULL:
		return; // No data beyond the type needs to be encoded for a NULL value.
	default:
		assert(0 && "Attempted to serialize value of invalid type.");
	}
}

void _RdbSaveEntity(RedisModuleIO *rdb, const Entity *e,  char **attr_map) {
	/* Format:
	 * #attributes N
	 * (name, value type, value) X N  */

	RedisModule_SaveUnsigned(rdb, e->prop_count);

	for(int i = 0; i < e->prop_count; i++) {
		EntityProperty attr = e->properties[i];
		const char *attr_name = attr_map[attr.id];
		RedisModule_SaveStringBuffer(rdb, attr_name, strlen(attr_name) + 1);
		_RdbSaveSIValue(rdb, &attr.value);
	}
}

void _RdbSaveNodes(RedisModuleIO *rdb, const Graph *g,  char **string_mapping) {
	/* Format:
	 * #nodes
	 *      ID
	 *      #labels M
	 *      (labels) X M
	 *      #properties N
	 *      (name, value type, value) X N */

	// #Nodes
	RedisModule_SaveUnsigned(rdb, Graph_NodeCount(g));

	Entity *e;
	DataBlockIterator *iter = Graph_ScanNodes(g);
	while((e = (Entity *)DataBlockIterator_Next(iter))) {
		// ID, currently ignored.
		RedisModule_SaveUnsigned(rdb, e->id);

		int l = Graph_GetNodeLabel(g, e->id);

		// #labels, currently only one label per node.
		int label_count = (l == GRAPH_NO_LABEL) ? 0 : 1;
		RedisModule_SaveUnsigned(rdb, label_count);

		// (labels) X M
		for(int i = 0; i < label_count; i ++) {
			RedisModule_SaveUnsigned(rdb, l);
		}

		// properties N
		// (name, value type, value) X N
		_RdbSaveEntity(rdb, e, string_mapping);
	}

	DataBlockIterator_Free(iter);
}

void _RdbSaveEdge(RedisModuleIO *rdb, const Graph *g, const Edge *e,
                  int r, char **string_mapping) {

	/* Format:
	* edge
	* {
	*  edge ID, currently not in use.
	*  source node ID
	*  destination node ID
	*  relation type
	* }
	* edge properties */

	EdgeID edgeID = ENTITY_GET_ID(e);
	NodeID src = Edge_GetSrcNodeID(e);
	NodeID dest = Edge_GetDestNodeID(e);

	// Edge ID.
	RedisModule_SaveUnsigned(rdb, edgeID);

	// Source node ID.
	src = _updatedID(g->nodes->deletedIdx, src);
	RedisModule_SaveUnsigned(rdb, src);

	// Destination node ID.
	dest = _updatedID(g->nodes->deletedIdx, dest);
	RedisModule_SaveUnsigned(rdb, dest);

	// Relation type.
	RedisModule_SaveUnsigned(rdb, r);

	// Edge properties.
	_RdbSaveEntity(rdb, e->entity, string_mapping);
}

void _RdbSaveEdges(RedisModuleIO *rdb, const Graph *g, char **string_mapping) {
	/* Format:
	 * #edges (N)
	 * {
	 *  edge ID, currently not in use.
	 *  source node ID
	 *  destination node ID
	 *  relation type
	 * } X N
	 * edge properties X N */

	// Sort deleted indices.
	QSORT(NodeID, g->nodes->deletedIdx, array_len(g->nodes->deletedIdx),
	      ENTITY_ID_ISLT);

	// #edges (N)
	RedisModule_SaveUnsigned(rdb, Graph_EdgeCount(g));

	for(int r = 0; r < array_len(g->_relations_map); r++) {
		Edge e;
		NodeID src;
		NodeID dest;
		EdgeID edgeID;
		GrB_Matrix M = g->_relations_map[r];
		GxB_MatrixTupleIter *it;
		GxB_MatrixTupleIter_new(&it, M);

		while(true) {
			bool depleted = false;
			GxB_MatrixTupleIter_next(it, &src, &dest, &depleted);
			e.srcNodeID = src;
			e.destNodeID = dest;

			if(depleted) break;

			GrB_Matrix_extractElement_UINT64(&edgeID, M, src, dest);
			if(SINGLE_EDGE(edgeID)) {
				edgeID = SINGLE_EDGE_ID(edgeID);
				Graph_GetEdge(g, edgeID, &e);
				_RdbSaveEdge(rdb, g, &e, r, string_mapping);
			} else {
				EdgeID *edgeIDs = (EdgeID *)edgeID;
				int edgeCount = array_len(edgeIDs);
				for(int i = 0; i < edgeCount; i++) {
					edgeID = edgeIDs[i];
					Graph_GetEdge(g, edgeID, &e);
					_RdbSaveEdge(rdb, g, &e, r, string_mapping);
				}
			}
		}

		GxB_MatrixTupleIter_free(it);
	}
}

void RdbSaveGraph(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * #nodes
	 *      ID
	 *      #labels M
	 *      (labels) X M
	 *      #properties N
	 *      (name, value type, value) X N
	 *
	 * #edges
	 *      edge ID, currently not in use.
	 *      relation type
	 *      source node ID
	 *      destination node ID
	 *      #properties N
	 *      (name, value type, value) X N
	 */

	// Dump nodes.
	_RdbSaveNodes(rdb, gc->g, gc->string_mapping);

	// Dump edges.
	_RdbSaveEdges(rdb, gc->g, gc->string_mapping);
}

void RdbLoadGraph(RedisModuleIO *rdb, GraphContext *gc) {
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
