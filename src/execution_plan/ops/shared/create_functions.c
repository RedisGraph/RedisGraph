/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "create_functions.h"
#include "RG.h"
#include "../../../errors.h"
#include "../../../query_ctx.h"
#include "../../../ast/ast_shared.h"
#include "../../../datatypes/array.h"
#include "../../../graph/graph_hub.h"

// commit node blueprints
static void _CommitNodesBlueprint
(
	PendingCreations *pending
) {
	GraphContext *gc = QueryCtx_GetGraphCtx();
	Graph *g = gc->g;

	// sync policy should be set to resize to capacity, no need to sync
	ASSERT(Graph_GetMatrixPolicy(g) == SYNC_POLICY_RESIZE);

	// create missing schemas
	// this loop iterates over the CREATE pattern, e.g.
	// CREATE (p:Person)
	// as such we're not expecting a large number of iterations
	uint blueprint_node_count = array_len(pending->nodes_to_create);
	for(uint i = 0; i < blueprint_node_count; i++) {
		NodeCreateCtx *node_ctx = pending->nodes_to_create + i;
		uint label_count = array_len(node_ctx->labels);

		for(uint j = 0; j < label_count; j++) {
			const char *label = node_ctx->labels[j];
			Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);

			if(s == NULL) {
				s = AddSchema(gc, label, SCHEMA_NODE);
				pending->stats->labels_added++;
			}

			node_ctx->labelsId[j] = s->id;
			pending->node_labels[i][j] = s->id;

			// sync matrix, make sure label matrix is of the right dimensions
			Graph_GetLabelMatrix(g, Schema_GetID(s));
		}
		// sync matrix, make sure mapping matrix is of the right dimensions
		if(label_count > 0) Graph_GetNodeLabelMatrix(g);
	}
}

// commit nodes
static void _CommitNodes
(
	PendingCreations *pending
) {
	Node          *n          =  NULL;
	GraphContext  *gc         =  QueryCtx_GetGraphCtx();
	Graph         *g          =  gc->g;
	uint          node_count  =  array_len(pending->created_nodes);

	// sync policy should be set to NOP, no need to sync/resize
	ASSERT(Graph_GetMatrixPolicy(g) == SYNC_POLICY_NOP);

	for(uint i = 0; i < node_count; i++) {
		n = pending->created_nodes[i];
		int *labels = pending->node_labels[i];
		uint label_count = array_len(labels);
		AttributeSet attr = pending->node_attributes[i];

		// introduce node into graph
		pending->stats->properties_set += CreateNode(gc, n, labels, label_count,
				attr);
	}
}

// commit edge blueprints
static void _CommitEdgesBlueprint
(
	EdgeCreateCtx *blueprints
) {
	GraphContext *gc = QueryCtx_GetGraphCtx();
	Graph *g = gc->g;

	// sync policy should be set to resize to capacity, no need to sync
	ASSERT(Graph_GetMatrixPolicy(g) == SYNC_POLICY_RESIZE);

	// create missing schemas
	// this loop iterates over the CREATE pattern, e.g.
	// CREATE (p:Person)-[e:VISITED]->(q)
	// As such we're not expecting a large number of iterations
	uint blueprint_edge_count = array_len(blueprints);
	for(uint i = 0; i < blueprint_edge_count; i++) {
		EdgeCreateCtx *edge_ctx = blueprints + i;

		const char *relation = edge_ctx->relation;
		Schema *s = GraphContext_GetSchema(gc, relation, SCHEMA_EDGE);
		if(s == NULL) s = AddSchema(gc, relation, SCHEMA_EDGE);

		// calling Graph_GetRelationMatrix will make sure relationship matrix
		// is of the right dimensions
		Graph_GetRelationMatrix(g, Schema_GetID(s), false);
	}

	// call Graph_GetAdjacencyMatrix will make sure the adjacency matrix
	// is of the right dimensions
	Graph_GetAdjacencyMatrix(g, false);
}

// commit edges
static void _CommitEdges
(
	PendingCreations *pending
) {
	Edge          *e          =  NULL;
	GraphContext  *gc         =  QueryCtx_GetGraphCtx();
	Graph         *g          =  gc->g;
	uint          edge_count  =  array_len(pending->created_edges);

	// sync policy should be set to NOP, no need to sync/resize
	ASSERT(Graph_GetMatrixPolicy(g) == SYNC_POLICY_NOP);

	for(uint i = 0; i < edge_count; i++) {
		e = pending->created_edges[i];
		NodeID srcNodeID;
		NodeID destNodeID;
		AttributeSet attr = pending->edge_attributes[i];

		// Nodes which already existed prior to this query would
		// have their ID set under e->srcNodeID and e->destNodeID
		// Nodes which are created as part of this query would be
		// saved under edge src/dest pointer.
		if(e->srcNodeID != INVALID_ENTITY_ID) srcNodeID = e->srcNodeID;
		else srcNodeID = ENTITY_GET_ID(Edge_GetSrcNode(e));
		if(e->destNodeID != INVALID_ENTITY_ID) destNodeID = e->destNodeID;
		else destNodeID = ENTITY_GET_ID(Edge_GetDestNode(e));

		Schema *s = GraphContext_GetSchema(gc, e->relationship, SCHEMA_EDGE);
		// all schemas have been created in the edge blueprint loop or earlier
		ASSERT(s != NULL);
		int relation_id = Schema_GetID(s);

		pending->stats->properties_set += CreateEdge(gc, e, srcNodeID,
				destNodeID, relation_id, attr);
	}
}

// Initialize all variables for storing pending creations.
void NewPendingCreationsContainer
(
	PendingCreations *pending,
	NodeCreateCtx *nodes,
	EdgeCreateCtx *edges
) {
	ASSERT(pending != NULL);

	pending->nodes_to_create = nodes;
	pending->edges_to_create = edges;
	pending->node_labels     = array_new(int *, 0);
	pending->created_nodes   = array_new(Node *, 0);
	pending->created_edges   = array_new(Edge *, 0);
	pending->node_attributes = array_new(AttributeSet, 0);
	pending->edge_attributes = array_new(AttributeSet, 0);
	pending->stats = NULL;
}

// Lock the graph and commit all changes introduced by the operation.
void CommitNewEntities
(
	OpBase *op,
	PendingCreations *pending
) {
	Graph *g = QueryCtx_GetGraph();
	uint node_count = array_len(pending->created_nodes);
	uint edge_count = array_len(pending->created_edges);
	if(!pending->stats) pending->stats = QueryCtx_GetResultSetStatistics();
	// Lock everything.
	QueryCtx_LockForCommit();

	if(node_count > 0) {
		Graph_AllocateNodes(g, node_count);

		// set graph matrix sync policy to resize
		// no need to perform sync
		Graph_SetMatrixPolicy(g, SYNC_POLICY_RESIZE);
		_CommitNodesBlueprint(pending);

		// set graph matrix sync policy to NOP
		// no need to perform sync/resize
		Graph_SetMatrixPolicy(g, SYNC_POLICY_NOP);
		_CommitNodes(pending);
	}

	if(edge_count > 0) {
		Graph_AllocateEdges(g, edge_count);

		// set graph matrix sync policy to resize
		// no need to perform sync
		Graph_SetMatrixPolicy(g, SYNC_POLICY_RESIZE);
		_CommitEdgesBlueprint(pending->edges_to_create);

		// set graph matrix sync policy to NOP
		// no need to perform sync/resize
		Graph_SetMatrixPolicy(g, SYNC_POLICY_NOP);
		_CommitEdges(pending);
	}

	// update statistics
	pending->stats->nodes_created          +=  node_count;
	pending->stats->relationships_created  +=  edge_count;

	// restore matrix sync policy to default
	Graph_SetMatrixPolicy(g, SYNC_POLICY_FLUSH_RESIZE);
}

// resolve the properties specified in the query into constant values
void ConvertPropertyMap
(
	GraphContext* gc,
	AttributeSet *attributes,
	Record r,
	PropertyMap *map,
	bool fail_on_null
) {
	uint property_count = array_len(map->keys);
	for(int i = 0; i < property_count; i++) {
		// note that AR_EXP_Evaluate may raise a run-time exception
		// in which case the allocations in this function will leak
		// for example, this occurs in the query:
		// CREATE (a {val: 2}), (b {val: a.val})
		SIValue val = AR_EXP_Evaluate(map->values[i], r);
		if(!(SI_TYPE(val) & SI_VALID_PROPERTY_VALUE)) {
			// this value is of an invalid type
			if(!SIValue_IsNull(val)) {
				// if the value was a complex type, emit an exception
				SIValue_Free(val);
				AttributeSet_Free(attributes);
				Error_InvalidPropertyValue();
				ErrorCtx_RaiseRuntimeException(NULL);
			}
			// the value was NULL
			// if this was prohibited in this context, raise an exception,
			// otherwise skip this value
			if(fail_on_null) {
				// emit an error and exit
				AttributeSet_Free(attributes);
				ErrorCtx_RaiseRuntimeException("Cannot merge node using null property value");
			}

			// don't add null to attrribute set
			continue;
		}

		// emit an error and exit if we're trying to add
		// an array containing an invalid type
		if(SI_TYPE(val) == T_ARRAY) {
			SIType invalid_properties = ~SI_VALID_PROPERTY_VALUE;
			bool res = SIArray_ContainsType(val, invalid_properties);
			if(res) {
				// validation failed
				SIValue_Free(val);
				AttributeSet_Free(attributes);
				Error_InvalidPropertyValue();
				ErrorCtx_RaiseRuntimeException(NULL);
			}
		}

		// set the converted attribute
		Attribute_ID attribute_id = FindOrAddAttribute(gc, map->keys[i]);
		AttributeSet_Add(attributes, attribute_id, val);
		SIValue_Free(val);
	}
}

// free all data associated with a completed create operation
void PendingCreationsFree
(
	PendingCreations *pending
) {
	if(pending->nodes_to_create) {
		uint nodes_to_create_count = array_len(pending->nodes_to_create);
		for(uint i = 0; i < nodes_to_create_count; i ++) {
			NodeCreateCtx_Free(pending->nodes_to_create[i]);
		}
		array_free(pending->nodes_to_create);
		pending->nodes_to_create = NULL;
	}

	if(pending->edges_to_create) {
		uint edges_to_create_count = array_len(pending->edges_to_create);
		for(uint i = 0; i < edges_to_create_count; i ++) {
			PropertyMap_Free(pending->edges_to_create[i].properties);
		}
		array_free(pending->edges_to_create);
		pending->edges_to_create = NULL;
	}

	if(pending->node_labels) {
		array_free(pending->node_labels);
		pending->node_labels = NULL;
	}

	if(pending->created_nodes) {
		array_free(pending->created_nodes);
		pending->created_nodes = NULL;
	}

	if(pending->created_edges) {
		array_free(pending->created_edges);
		pending->created_edges = NULL;
	}

	if(pending->node_attributes) {
		array_free(pending->node_attributes);
		pending->node_attributes = NULL;
	}

	if(pending->edge_attributes) {
		array_free(pending->edge_attributes);
		pending->edge_attributes = NULL;
	}
}
