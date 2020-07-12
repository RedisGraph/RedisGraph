/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "create_functions.h"
#include "../../../query_ctx.h"

// Add properties to the GraphEntity.
static inline void _AddProperties(ResultSetStatistics *stats, GraphEntity *ge,
								  PendingProperties *props) {
	for(int i = 0; i < props->property_count; i++) {
		GraphEntity_AddProperty(ge, props->attr_keys[i], props->values[i]);
	}

	if(stats) stats->properties_set += props->property_count;
}

/* Commit insertions. */
static void _CommitNodes(PendingCreations *pending) {
	Node *n;
	GraphContext *gc = QueryCtx_GetGraphCtx();
	Graph *g = gc->g;

	/* Create missing schemas.
	 * this loop iterates over the CREATE pattern, e.g.
	 * CREATE (p:Person)
	 * As such we're not expecting a large number of iterations. */
	uint blueprint_node_count = array_len(pending->nodes_to_create);
	for(uint i = 0; i < blueprint_node_count; i++) {
		NodeCreateCtx *node_ctx = pending->nodes_to_create + i;

		const char *label = node_ctx->node->label;
		if(label) {
			if(GraphContext_GetSchema(gc, label, SCHEMA_NODE) == NULL) {
				Schema *s = GraphContext_AddSchema(gc, label, SCHEMA_NODE);
				pending->stats->labels_added++;
			}
		}
	}

	uint node_count = array_len(pending->created_nodes);
	Graph_AllocateNodes(g, node_count);

	for(uint i = 0; i < node_count; i++) {
		n = pending->created_nodes[i];
		Schema *s = NULL;

		// Get label ID.
		int labelID = GRAPH_NO_LABEL;
		if(n->label != NULL) {
			s = GraphContext_GetSchema(gc, n->label, SCHEMA_NODE);
			assert(s);
			labelID = s->id;
		}

		// Introduce node into graph.
		Graph_CreateNode(g, labelID, n);

		if(pending->node_properties[i]) _AddProperties(pending->stats, (GraphEntity *)n,
														   pending->node_properties[i]);

		if(s && Schema_HasIndices(s)) Schema_AddNodeToIndices(s, n);
	}
}

static void _CommitEdges(PendingCreations *pending) {
	Edge *e;
	GraphContext *gc = QueryCtx_GetGraphCtx();
	Graph *g = gc->g;

	/* Create missing schemas.
	 * this loop iterates over the CREATE pattern, e.g.
	 * CREATE (p:Person)-[e:VISITED]->(q)
	 * As such we're not expecting a large number of iterations. */
	uint blueprint_edge_count = array_len(pending->edges_to_create);
	for(uint i = 0; i < blueprint_edge_count; i++) {
		EdgeCreateCtx *edge_ctx = pending->edges_to_create + i;

		const char **reltypes = edge_ctx->edge->reltypes;
		if(reltypes) {
			uint reltype_count = array_len(reltypes);
			for(uint j = 0; j < reltype_count; j ++) {
				const char *reltype = reltypes[j];
				if(GraphContext_GetSchema(gc, reltype, SCHEMA_EDGE) == NULL) {
					Schema *s = GraphContext_AddSchema(gc, reltype, SCHEMA_EDGE);
				}
			}
		}
	}

	uint edge_count = array_len(pending->created_edges);
	Graph_AllocateEdges(g, edge_count);

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

		Schema *schema = GraphContext_GetSchema(gc, e->relationship, SCHEMA_EDGE);
		if(!schema) schema = GraphContext_AddSchema(gc, e->relationship, SCHEMA_EDGE);
		int relation_id = schema->id;

		assert(Graph_ConnectNodes(g, srcNodeID, destNodeID, relation_id, e));

		if(pending->edge_properties[i]) _AddProperties(pending->stats, (GraphEntity *)e,
														   pending->edge_properties[i]);
	}
}

// Initialize all variables for storing pending creations.
PendingCreations NewPendingCreationsContainer(NodeCreateCtx *nodes, EdgeCreateCtx *edges) {
	PendingCreations pending;
	pending.nodes_to_create = nodes;
	pending.edges_to_create = edges;
	pending.created_nodes = array_new(Node *, 0);
	pending.created_edges = array_new(Edge *, 0);
	pending.node_properties = array_new(PendingProperties *, 0);
	pending.edge_properties = array_new(PendingProperties *, 0);
	pending.stats = NULL;

	return pending;
}

// Lock the graph and commit all changes introduced by the operation.
void CommitNewEntities(OpBase *op, PendingCreations *pending) {
	Graph *g = QueryCtx_GetGraph();
	uint node_count = array_len(pending->created_nodes);
	uint edge_count = array_len(pending->created_edges);
	if(!pending->stats) pending->stats = QueryCtx_GetResultSetStatistics();
	// Lock everything.
	QueryCtx_LockForCommit();

	/* Set sync policy to resize to capacity only for node introduction
	 * as only node creation can have an effect on matrix dimensions. */
	Graph_SetMatrixPolicy(g, RESIZE_TO_CAPACITY);
	if(node_count > 0) _CommitNodes(pending);

	/* Reset sync policy to minimum space to avoid further matrix resize:
	 * From capacity to actual node count.
	 * Recall that edge creation/deletion doesn't have an effect on matrix dimensions. */
	Graph_SetMatrixPolicy(g, SYNC_AND_MINIMIZE_SPACE);
	if(edge_count > 0) _CommitEdges(pending);
	// Release lock.
	pending->stats->nodes_created += node_count;
	pending->stats->relationships_created += edge_count;
	QueryCtx_UnlockCommit(op);
}

// Resolve the properties specified in the query into constant values.
PendingProperties *ConvertPropertyMap(Record r, const PropertyMap *map) {
	PendingProperties *converted = rm_malloc(sizeof(PendingProperties));
	converted->property_count = map->property_count;
	converted->attr_keys = map->keys; // This pointer can be copied directly.
	converted->values = rm_malloc(sizeof(SIValue) * map->property_count);
	for(int i = 0; i < map->property_count; i++) {
		converted->values[i] = AR_EXP_Evaluate(map->values[i], r);
	}

	return converted;
}

// Free the properties that have been committed to the graph
void PendingPropertiesFree(PendingProperties *props) {
	if(props == NULL) return;
	// The 'keys' array belongs to the original PropertyMap, so shouldn't be freed here.
	for(uint j = 0; j < props->property_count; j ++) {
		SIValue_Free(props->values[j]);
	}
	rm_free(props->values);
	rm_free(props);
}

// Free all data associated with a completed create operation.
void PendingCreationsFree(PendingCreations *pending) {
	if(pending->nodes_to_create) {
		uint nodes_to_create_count = array_len(pending->nodes_to_create);
		for(uint i = 0; i < nodes_to_create_count; i ++) {
			PropertyMap_Free(pending->nodes_to_create[i].properties);
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
			PendingPropertiesFree(pending->node_properties[i]);
		}
		array_free(pending->node_properties);
		pending->node_properties = NULL;
	}

	// Free all graph-committed properties associated with edges.
	if(pending->edge_properties) {
		uint prop_count = array_len(pending->edge_properties);
		for(uint i = 0; i < prop_count; i ++) {
			PendingPropertiesFree(pending->edge_properties[i]);
		}
		array_free(pending->edge_properties);
		pending->edge_properties = NULL;
	}
}

