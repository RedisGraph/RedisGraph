/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "entity_funcs.h"
#include "../func_desc.h"
#include "../../errors.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../datatypes/map.h"
#include "../../graph/graphcontext.h"
#include "../../graph/entities/node.h"
#include "../../graph/entities/edge.h"
#include "../../graph/entities/graph_entity.h"

/* returns the id of a relationship or node. */
SIValue AR_ID(SIValue *argv, int argc) {
	if(SI_TYPE(argv[0]) == T_NULL) return SI_NullVal();
	GraphEntity *graph_entity = (GraphEntity *)argv[0].ptrval;
	return SI_LongVal(ENTITY_GET_ID(graph_entity));
}

/* returns a string representations the label of a node. */
SIValue AR_LABELS(SIValue *argv, int argc) {
	if(SI_TYPE(argv[0]) == T_NULL) return SI_NullVal();
	char *label = "";
	Node *node = argv[0].ptrval;
	GraphContext *gc = QueryCtx_GetGraphCtx();
	Graph *g = gc->g;
	int labelID = Graph_GetNodeLabel(g, ENTITY_GET_ID(node));
	if(labelID != GRAPH_NO_LABEL) label = gc->node_schemas[labelID]->name;
	return SI_ConstStringVal(label);
}

/* returns a string representation of the type of a relation. */
SIValue AR_TYPE(SIValue *argv, int argc) {
	if(SI_TYPE(argv[0]) == T_NULL) return SI_NullVal();
	char *type = "";
	Edge *e = argv[0].ptrval;
	GraphContext *gc = QueryCtx_GetGraphCtx();
	int id = Graph_GetEdgeRelation(gc->g, e);
	if(id != GRAPH_NO_RELATION) type = gc->relation_schemas[id]->name;
	return SI_ConstStringVal(type);
}

/* returns the start node of a relationship. */
SIValue AR_STARTNODE(SIValue *argv, int argc) {
	if(SI_TYPE(argv[0]) == T_NULL) return SI_NullVal();
	Edge *e = argv[0].ptrval;
	NodeID start_id = Edge_GetSrcNodeID(e);
	GraphContext *gc = QueryCtx_GetGraphCtx();
	Node *src = rm_malloc(sizeof(Node));
	*src = GE_NEW_NODE();
	// Retrieve the node from the graph.
	Graph_GetNode(gc->g, start_id, src);
	SIValue si_node = SI_Node(src);
	// Mark this value as a heap allocation so that it gets freed properly.
	SIValue_SetAllocationType(&si_node, M_SELF);
	return si_node;
}

/* returns the end node of a relationship. */
SIValue AR_ENDNODE(SIValue *argv, int argc) {
	if(SI_TYPE(argv[0]) == T_NULL) return SI_NullVal();
	Edge *e = argv[0].ptrval;
	NodeID end_id = Edge_GetDestNodeID(e);
	GraphContext *gc = QueryCtx_GetGraphCtx();
	Node *dest = rm_malloc(sizeof(Node));
	*dest = GE_NEW_NODE();
	// Retrieve the node from the graph.
	Graph_GetNode(gc->g, end_id, dest);
	SIValue si_node = SI_Node(dest);
	// Mark this value as a heap allocation so that it gets freed properly.
	SIValue_SetAllocationType(&si_node, M_SELF);
	return si_node;
}

/* returns true if the specified property exists in the node, or relationship. */
SIValue AR_EXISTS(SIValue *argv, int argc) {
	/* MATCH (n) WHERE EXISTS(n.name) RETURN n
	 * If property n.name does not exists
	 * SIValue representing NULL is returned.
	 * if n.name exists its value can not be NULL. */
	if(SIValue_IsNull(argv[0])) return SI_BoolVal(0);
	return SI_BoolVal(1);
}

SIValue _AR_NodeDegree(SIValue *argv, int argc, GRAPH_EDGE_DIR dir) {
	if(SI_TYPE(argv[0]) == T_NULL) return SI_NullVal();
	Node *n = (Node *)argv[0].ptrval;
	Edge *edges = array_new(Edge, 0);
	GraphContext *gc = QueryCtx_GetGraphCtx();

	if(argc > 1) {
		// We're interested in specific relationship type(s).
		for(int i = 1; i < argc; i++) {
			const char *label = argv[i].stringval;

			// Make sure relationship exists.
			Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_EDGE);
			if(!s) continue;

			// Accumulate edges.
			Graph_GetNodeEdges(gc->g, n, dir, s->id, &edges);
		}
	} else {
		// Get all relations, regardless of their type.
		Graph_GetNodeEdges(gc->g, n, dir, GRAPH_NO_RELATION, &edges);
	}

	SIValue res = SI_LongVal(array_len(edges));
	array_free(edges);
	return res;
}

/* Returns the number of incoming edges for given node. */
SIValue AR_INCOMEDEGREE(SIValue *argv, int argc) {
	if(SI_TYPE(argv[0]) == T_NULL) return SI_NullVal();
	return _AR_NodeDegree(argv, argc, GRAPH_EDGE_DIR_INCOMING);
}

/* Returns the number of outgoing edges for given node. */
SIValue AR_OUTGOINGDEGREE(SIValue *argv, int argc) {
	if(SI_TYPE(argv[0]) == T_NULL) return SI_NullVal();
	return _AR_NodeDegree(argv, argc, GRAPH_EDGE_DIR_OUTGOING);
}

SIValue AR_PROPERTY(SIValue *argv, int argc) {
	// return NULL for missing graph entity
	if(SI_TYPE(argv[0]) == T_NULL) return SI_NullVal();

	// AR_PROPERTY may be invoked from AR_SUBSCRIPT in a case like:
	// WITH {val: 5} AS map RETURN map["val"]
	// As such, we need to validate the argument's type independently of the invocation validation.
	if(SI_TYPE(argv[1]) != T_STRING) {
		// String indexes are only permitted on maps, not arrays.
		Error_SITypeMismatch(argv[1], T_STRING);
		return SI_NullVal();
	}

	// inputs:
	// argv[0] - node/edge/map
	// argv[1] - property string
	// argv[2] - property index

	//--------------------------------------------------------------------------
	// Process inputs
	//--------------------------------------------------------------------------

	SIValue obj = argv[0];

	if(SI_TYPE(obj) & SI_GRAPHENTITY) {
		// retrieve entity property
		GraphEntity *graph_entity = (GraphEntity *)obj.ptrval;
		const char *prop_name     = argv[1].stringval;
		Attribute_ID prop_idx     = argv[2].longval;

		// We have the property string, attempt to look up the index now.
		if(prop_idx == ATTRIBUTE_UNKNOWN) {
			GraphContext *gc = QueryCtx_GetGraphCtx();
			prop_idx = GraphContext_GetAttributeID(gc, prop_name);
		}

		// Retrieve the property.
		SIValue *value = GraphEntity_GetProperty(graph_entity, prop_idx);
		return SI_ConstValue(*value);
	} else {
		// retrieve map key
		SIValue key = argv[1];
		SIValue value;

		Map_Get(obj, key, &value);
		// Return a volatile copy of the value, as it may be heap-allocated.
		return SI_ShareValue(value);
	}
}

void Register_EntityFuncs() {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 1);
	types = array_append(types, T_NULL | T_NODE | T_EDGE);
	func_desc = AR_FuncDescNew("id", AR_ID, 1, 1, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_NULL | T_NODE);
	func_desc = AR_FuncDescNew("labels", AR_LABELS, 1, 1, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_NULL | T_EDGE);
	func_desc = AR_FuncDescNew("type", AR_TYPE, 1, 1, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_NULL | T_EDGE);
	func_desc = AR_FuncDescNew("startNode", AR_STARTNODE, 1, 1, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_NULL | T_EDGE);
	func_desc = AR_FuncDescNew("endNode", AR_ENDNODE, 1, 1, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_NULL | SI_ALL);
	func_desc = AR_FuncDescNew("exists", AR_EXISTS, 1, 1, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	types = array_append(types, T_NULL | T_NODE);
	types = array_append(types, T_STRING);
	func_desc = AR_FuncDescNew("indegree", AR_INCOMEDEGREE, 1, VAR_ARG_LEN, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	types = array_append(types, T_NULL | T_NODE);
	types = array_append(types, T_STRING);
	func_desc = AR_FuncDescNew("outdegree", AR_OUTGOINGDEGREE, 1, VAR_ARG_LEN, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 3);
	types = array_append(types, T_NULL | T_NODE | T_EDGE | T_MAP);
	types = array_append(types, T_STRING);
	types = array_append(types, T_INT64);
	func_desc = AR_FuncDescNew("property", AR_PROPERTY, 3, 3, types, true, false);
	AR_RegFunc(func_desc);
}

