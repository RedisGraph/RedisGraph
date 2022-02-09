/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
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
				s = GraphContext_AddSchema(gc, label, SCHEMA_NODE);
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

		// introduce node into graph
		pending->stats->properties_set += CreateNode(gc, n, labels, label_count, pending->node_properties + i);
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
		if(s == NULL) s = GraphContext_AddSchema(gc, relation, SCHEMA_EDGE);

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

		pending->stats->properties_set += CreateEdge(gc, e, srcNodeID, destNodeID, relation_id, pending->edge_properties + i);
	}
}

// Initialize all variables for storing pending creations.
PendingCreations NewPendingCreationsContainer
(
	NodeCreateCtx *nodes,
	EdgeCreateCtx *edges
) {
	PendingCreations pending;
	pending.nodes_to_create = nodes;
	pending.edges_to_create = edges;
	pending.node_labels     = array_new(int *, 0);
	pending.created_nodes   = array_new(Node *, 0);
	pending.created_edges   = array_new(Edge *, 0);
	pending.node_attributes = array_new(AttributeSet, 0);
	pending.edge_attributes = array_new(AttributeSet, 0);
	pending.stats = NULL;

	return pending;
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

	// Release lock.
	pending->stats->nodes_created += node_count;
	pending->stats->relationships_created += edge_count;

	// restore matrix sync policy to default
	Graph_SetMatrixPolicy(g, SYNC_POLICY_FLUSH_RESIZE);

	QueryCtx_UnlockCommit(op);
}

// Resolve the properties specified in the query into constant values.
void ConvertPropertyMap
(
	AttributeSet *attributes,
	Record r,
	PropertyMap *map,
	bool fail_on_null
) {
	uint property_count = array_len(map->keys);
	for(int i = 0; i < property_count; i++) {
		/* Note that AR_EXP_Evaluate may raise a run-time exception, in which case
		 * the allocations in this function will leak.
		 * For example, this occurs in the query:
		 * CREATE (a {val: 2}), (b {val: a.val}) */
		SIValue val = AR_EXP_Evaluate(map->values[i], r);
		if(!(SI_TYPE(val) & SI_VALID_PROPERTY_VALUE)) {
			// This value is of an invalid type.
			if(!SIValue_IsNull(val)) {
				// If the value was a complex type, emit an exception.
				AttributeSet_FreeAttributes(attributes);
				Error_InvalidPropertyValue();
				ErrorCtx_RaiseRuntimeException(NULL);
			}
			/* The value was NULL. If this was prohibited in this context, raise an exception,
			 * otherwise skip this value. */
			if(fail_on_null) {
				// emit an error and exit
				AttributeSet_FreeAttributes(attributes);
				ErrorCtx_RaiseRuntimeException("Cannot merge node using null property value");
			}
		}

		// emit an error and exit if we're trying to add
		// an array containing an invalid type
		if(SI_TYPE(val) == T_ARRAY) {
			SIType invalid_properties = ~SI_VALID_PROPERTY_VALUE;
			bool res = SIArray_ContainsType(val, invalid_properties);
			if(res) {
				// validation failed
				SIValue_Free(val);
				AttributeSet_FreeAttributes(attributes);
				Error_InvalidPropertyValue();
				ErrorCtx_RaiseRuntimeException(NULL);
			}
		}

		// set the converted property
		AttributeSet_AddProperty(attributes, map->keys[i], val, false);
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

	// Free all graph-committed properties associated with nodes.
	if(pending->node_properties) {
		uint prop_count = array_len(pending->node_properties);
		for(uint i = 0; i < prop_count; i ++) {
			AttributeSet_FreeAttributes(pending->node_properties + i);
		}
		array_free(pending->node_properties);
		pending->node_properties = NULL;
	}

	// Free all graph-committed properties associated with edges.
	if(pending->edge_properties) {
		uint prop_count = array_len(pending->edge_properties);
		for(uint i = 0; i < prop_count; i ++) {
			AttributeSet_FreeAttributes(pending->edge_properties + i);
		}
		array_free(pending->edge_properties);
		pending->edge_properties = NULL;
	}
}

