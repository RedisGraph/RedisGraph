/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>
#include "encode_graph.h"

#include "../../graph.h"
#include "../../../util/arr.h"
#include "../../../util/qsort.h"
#include "../../../../deps/GraphBLAS/Include/GraphBLAS.h"
#include "../../../datatypes/array.h"

// Forward declerations.
void _RdbSaveSIArray(RedisModuleIO *rdb, const SIValue array);

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
	case T_ARRAY:
		_RdbSaveSIArray(rdb, *v);
		return;
	case T_NULL:
		return; // No data beyond the type needs to be encoded for a NULL value.
	default:
		assert(0 && "Attempted to serialize value of invalid type.");
	}
}

void _RdbSaveSIArray(RedisModuleIO *rdb, const SIValue list) {
	/* saves array as
	   unsinged : array legnth
	   array[0]
	   .
	   .
	   .
	   array[array length -1]
	 */
	uint arrayLen = SIArray_Length(list);
	RedisModule_SaveUnsigned(rdb, arrayLen);
	for(uint i = 0; i < arrayLen; i ++) {
		SIValue value = SIArray_Get(list, i);
		_RdbSaveSIValue(rdb, &value);
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
	 *      #labels M
	 *      (labels) X M
	 *      #properties N
	 *      (name, value type, value) X N */

	// #Nodes
	RedisModule_SaveUnsigned(rdb, Graph_NodeCount(g));

	Entity *e;
	DataBlockIterator *iter = Graph_ScanNodes(g);
	while((e = (Entity *)DataBlockIterator_Next(iter))) {
		int l = Graph_GetNodeLabel(g, e->id);

		// #labels, currently only one label per node.
		int label_count = (l == GRAPH_NO_LABEL) ? 0 : 1;
		RedisModule_SaveUnsigned(rdb, label_count);

		// (label)
		if(label_count) RedisModule_SaveUnsigned(rdb, l);

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
	*  source node ID
	*  destination node ID
	*  relation type
	* }
	* edge properties */

	NodeID src = Edge_GetSrcNodeID(e);
	NodeID dest = Edge_GetDestNodeID(e);

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
	 *  source node ID
	 *  destination node ID
	 *  relation type
	 * } X N
	 * edge properties X N */

	// Sort deleted indices.
	QSORT(NodeID, g->nodes->deletedIdx, array_len(g->nodes->deletedIdx), ENTITY_ID_ISLT);

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
		bool depleted = false;

		while(true) {
			GxB_MatrixTupleIter_next(it, &src, &dest, &depleted);
			if(depleted) break;

			e.srcNodeID = src;
			e.destNodeID = dest;

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

	// Dump nodes.
	_RdbSaveNodes(rdb, gc->g, gc->string_mapping);

	// Dump edges.
	_RdbSaveEdges(rdb, gc->g, gc->string_mapping);
}
