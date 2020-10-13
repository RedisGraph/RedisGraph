/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "query_graph.h"
#include "../RG.h"
#include "../util/arr.h"
#include "../util/strcmp.h"
#include "../query_ctx.h"
#include "../schema/schema.h"
#include "../../deps/rax/rax.h"

// Sets node label and label ID
static void _QueryGraphSetNodeLabel(QGNode *n, const cypher_astnode_t *ast_entity) {
	// node label already set
	if(n->labelID != GRAPH_NO_LABEL) return;

	// Retrieve node labels from the AST entity.
	uint nlabels = cypher_ast_node_pattern_nlabels(ast_entity);
	// We currently only support 0 or 1 labels per node, so if any are specified just select the first.
	const char *label = (nlabels > 0) ? cypher_ast_label_get_name(cypher_ast_node_pattern_get_label(
																	  ast_entity, 0)) : NULL;

	// Set node label ID.
	if(label) {
		GraphContext *gc = QueryCtx_GetGraphCtx();
		Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
		uint label_id = GRAPH_UNKNOWN_LABEL;
		// If a schema is found, the AST refers to an existing label.
		if(s) label_id = s->id;
		n->label = label;
		n->labelID = label_id;
	} else {
		// Label isn't known to the graph.
		n->labelID = GRAPH_NO_LABEL;
	}
}

// Adds node to query graph
static void _QueryGraphAddNode(QueryGraph *qg, const cypher_astnode_t *ast_entity) {
	AST *ast = QueryCtx_GetAST();
	const char *alias = AST_GetEntityName(ast, ast_entity);

	/* Look up this alias in the QueryGraph.
	 * This node may already exist if it appears multiple times in query patterns. */
	QGNode *n = QueryGraph_GetNodeByAlias(qg, alias);
	if(n == NULL) {
		// Node has not been mapped; create it.
		n = QGNode_New(alias);
		QueryGraph_AddNode(qg, n);
	}

	_QueryGraphSetNodeLabel(n, ast_entity);
}

// Adds edge to query graph
static void _QueryGraphAddEdge(QueryGraph *qg, const cypher_astnode_t *ast_entity,
							   QGNode *src, QGNode *dest) {

	AST *ast = QueryCtx_GetAST();
	GraphContext *gc = QueryCtx_GetGraphCtx();
	const char *alias = AST_GetEntityName(ast, ast_entity);
	enum cypher_rel_direction dir = cypher_ast_rel_pattern_get_direction(ast_entity);

	// Each edge can only appear once in a QueryGraph.
	ASSERT(QueryGraph_GetEdgeByAlias(qg, alias) == NULL);

	QGEdge *edge = QGEdge_New(NULL, NULL, NULL, alias);
	edge->bidirectional = (dir == CYPHER_REL_BIDIRECTIONAL);

	// Add the IDs of all reltype matrixes
	uint nreltypes = cypher_ast_rel_pattern_nreltypes(ast_entity);
	for(uint i = 0; i < nreltypes; i ++) {
		const char *reltype = cypher_ast_reltype_get_name(cypher_ast_rel_pattern_get_reltype(ast_entity,
														  i));
		edge->reltypes = array_append(edge->reltypes, reltype);
		Schema *s = GraphContext_GetSchema(gc, reltype, SCHEMA_EDGE);
		if(!s) {
			// Unknown relationship
			edge->reltypeIDs = array_append(edge->reltypeIDs, GRAPH_UNKNOWN_RELATION);
			qg->unknown_reltype_ids = true;
			continue;
		}
		edge->reltypeIDs = array_append(edge->reltypeIDs, s->id);
	}

	// Incase of a variable length edge, set edge min/max hops.
	const cypher_astnode_t *range = cypher_ast_rel_pattern_get_varlength(ast_entity);
	if(range) {
		const cypher_astnode_t *start = cypher_ast_range_get_start(range);
		const cypher_astnode_t *end = cypher_ast_range_get_end(range);
		if(start) edge->minHops = AST_ParseIntegerNode(start);
		if(end) edge->maxHops = AST_ParseIntegerNode(end);
		else edge->maxHops = EDGE_LENGTH_INF;
	}

	// Build and add a QGEdge representing this entity to the QueryGraph.
	// Swap the source and destination for left-pointing relations
	if(dir != CYPHER_REL_INBOUND) QueryGraph_ConnectNodes(qg, src, dest, edge);
	else QueryGraph_ConnectNodes(qg, dest, src, edge);
}

// Extracts node from 'qg' and places a copy of into 'graph'
static void _QueryGraph_ExtractNode(const QueryGraph *qg, QueryGraph *graph,
									AST *ast, const cypher_astnode_t *ast_node) {

	// Validate inputs.
	ASSERT(qg != NULL && graph != NULL && ast != NULL && ast_node != NULL);

	// See if node is already in 'graph'.
	const char *alias = AST_GetEntityName(ast, ast_node);
	QGNode *n = QueryGraph_GetNodeByAlias(graph, alias);

	if(n == NULL) {
		// Node is missing from 'graph', try getting it from 'qg'.
		n = QueryGraph_GetNodeByAlias(qg, alias);
		if(n == NULL) {
			/* Node is missing from 'qg', create it.
			 * It is possible to get into a situation where we try to extract
			 * a path which contains entities that are missing from the
			 * "holistic" query graph consider:
			 * MATCH (a) WITH a WHERE (a)-[]->(:L1) in this case due to
			 * clause scoping only node 'a' is in 'qg' the filtered pattern
			 * which is being extracted from 'qg' has additional entities:
			 * an anonymous edge and node. */
			_QueryGraphAddNode(graph, ast_node);
		} else {
			// Add a clone of the original node.
			n = QGNode_Clone(n);

			// Clear node label information.
			n->label = NULL;
			n->labelID = GRAPH_NO_LABEL;

			QueryGraph_AddNode(graph, n);
			// Set node label information.
			_QueryGraphSetNodeLabel(n, ast_node);
		}
	}
}

// Extracts edge from 'qg' and places a copy of into 'graph'
static void _QueryGraph_ExtractEdge(const QueryGraph *qg, QueryGraph *graph,
									QGNode *left, QGNode *right, AST *ast, const cypher_astnode_t *ast_edge) {
	const char *alias = AST_GetEntityName(ast, ast_edge);

	// Validate input, edge shouldn't be in graph.
	ASSERT(left != NULL && right != NULL);
	ASSERT(QueryGraph_GetEdgeByAlias(graph, alias) == NULL);
	/* Unlike nodes that can show up multiple times within a pattern
	 * e.g. MATCH (a), (a:L)
	 * where each occurance might add an additional piece of information
	 * an edge can only be mentioned once, as such there's no value in
	 * cloning an edge. therefor we simply add it.*/
	_QueryGraphAddEdge(graph, ast_edge, left, right);
}

// Clones path from 'qg' into 'graph'.
static void _QueryGraph_ExtractPath(const QueryGraph *qg, QueryGraph *graph,
									const cypher_astnode_t *path) {

	// Validate input.
	ASSERT(qg != NULL && graph != NULL && path != NULL);

	const char *alias;
	AST *ast = QueryCtx_GetAST();
	const cypher_astnode_t *ast_node;
	uint nelems = cypher_ast_pattern_path_nelements(path);

	/* Introduce nodes to graph
	 * Nodes are at even indices */
	for(uint i = 0; i < nelems; i += 2) {
		ast_node = cypher_ast_pattern_path_get_element(path, i);
		_QueryGraph_ExtractNode(qg, graph, ast, ast_node);
	}

	/* Introduce edges to graph
	 * edges are at odd indices */
	for(uint i = 1; i < nelems; i += 2) {
		// Retrieve the QGNode corresponding to the node left of this edge.
		const cypher_astnode_t *l_node = cypher_ast_pattern_path_get_element(path, i - 1);
		const char *l_alias = AST_GetEntityName(ast, l_node);
		QGNode *left = QueryGraph_GetNodeByAlias(graph, l_alias);

		// Retrieve the QGNode corresponding to the node right of this edge.
		const cypher_astnode_t *r_node = cypher_ast_pattern_path_get_element(path, i + 1);
		const char *r_alias = AST_GetEntityName(ast, r_node);
		QGNode *right = QueryGraph_GetNodeByAlias(graph, r_alias);

		ast_node = cypher_ast_pattern_path_get_element(path, i);
		_QueryGraph_ExtractEdge(qg, graph, left, right, ast, ast_node);
	}
}

QueryGraph *QueryGraph_New(uint node_cap, uint edge_cap) {
	QueryGraph *qg = rm_malloc(sizeof(QueryGraph));

	qg->nodes = array_new(QGNode *, node_cap);
	qg->edges = array_new(QGEdge *, edge_cap);
	qg->unknown_reltype_ids = false;

	return qg;
}

void QueryGraph_AddNode(QueryGraph *qg, QGNode *n) {
	qg->nodes = array_append(qg->nodes, n);
}

void QueryGraph_ConnectNodes(QueryGraph *qg, QGNode *src, QGNode *dest, QGEdge *e) {
	QGNode_ConnectNode(src, dest, e);
	e->src = src;
	e->dest = dest;
	qg->edges = array_append(qg->edges, e);
}

void QueryGraph_AddPath(QueryGraph *qg, const cypher_astnode_t *path) {
	AST *ast = QueryCtx_GetAST();
	uint nelems = cypher_ast_pattern_path_nelements(path);
	/* Introduce nodes first. Nodes are positioned at every even offset
	 * into the path (0, 2, ...) */
	for(uint i = 0; i < nelems; i += 2) {
		const cypher_astnode_t *ast_node = cypher_ast_pattern_path_get_element(path, i);
		_QueryGraphAddNode(qg, ast_node);
	}

	/* Every odd offset corresponds to an edge in a path. */
	for(uint i = 1; i < nelems; i += 2) {
		// Retrieve the QGNode corresponding to the node left of this edge.
		const cypher_astnode_t *l_node = cypher_ast_pattern_path_get_element(path, i - 1);
		const char *l_alias = AST_GetEntityName(ast, l_node);
		QGNode *left = QueryGraph_GetNodeByAlias(qg, l_alias);

		// Retrieve the QGNode corresponding to the node right of this edge.
		const cypher_astnode_t *r_node = cypher_ast_pattern_path_get_element(path, i + 1);
		const char *r_alias = AST_GetEntityName(ast, r_node);
		QGNode *right = QueryGraph_GetNodeByAlias(qg, r_alias);

		// Retrieve the AST reference to this edge.
		const cypher_astnode_t *edge = cypher_ast_pattern_path_get_element(path, i);
		_QueryGraphAddEdge(qg, edge, left, right);
	}
}

// Clones path from 'qg' into 'graph'
QueryGraph *QueryGraph_ExtractPaths(const QueryGraph *qg, const cypher_astnode_t **paths, uint n) {
	// Validate input.
	ASSERT(qg != NULL && paths != NULL);

	// Create an empty query graph.
	uint node_count = QueryGraph_NodeCount(qg);
	uint edge_count = QueryGraph_EdgeCount(qg);
	QueryGraph *graph = QueryGraph_New(node_count, edge_count);
	ASSERT(graph != NULL);

	for(int i = 0; i < n; i++) {
		const cypher_astnode_t *path = paths[i];
		_QueryGraph_ExtractPath(qg, graph, path);
	}

	return graph;
}

// Clones patterns from 'qg' into 'graph'
QueryGraph *QueryGraph_ExtractPatterns(const QueryGraph *qg,
									   const cypher_astnode_t **patterns, uint n) {

	// Validate inputs.
	ASSERT(qg != NULL && patterns != NULL);

	// Create an empty query graph.
	uint node_count = QueryGraph_NodeCount(qg);
	uint edge_count = QueryGraph_EdgeCount(qg);
	QueryGraph *graph = QueryGraph_New(node_count, edge_count);
	ASSERT(graph != NULL);

	// extract paths described by each pattern
	for(uint i = 0; i < n; i++) {
		const cypher_astnode_t *pattern = patterns[i];
		uint npaths = cypher_ast_pattern_npaths(pattern);
		for(uint j = 0; j < npaths; j ++) {
			const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, j);
			_QueryGraph_ExtractPath(qg, graph, path);
		}
	}

	return graph;
}

// Build a query graph from MATCH clauses.
QueryGraph *BuildQueryGraph(const AST *ast) {
	uint node_count;
	uint edge_count;
	// The initial node and edge arrays will be large enough to accommodate all AST entities
	// (which is overkill, consider reducing)
	node_count = edge_count = raxSize(ast->referenced_entities);
	QueryGraph *qg = QueryGraph_New(node_count, edge_count);

	// Collect all path AST nodes.
	const cypher_astnode_t **paths = AST_GetTypedNodes(ast->root, CYPHER_AST_PATTERN_PATH);
	uint n = array_len(paths);
	for(uint i = 0; i < n; i++) {
		const cypher_astnode_t *path = paths[i];
		QueryGraph_AddPath(qg, path);
	}

	array_free(paths);
	return qg;
}

void QueryGraph_MergeGraphs(QueryGraph *to, QueryGraph *from) {
	uint node_count = QueryGraph_NodeCount(from);
	uint edge_count = QueryGraph_EdgeCount(from);

	for(uint i = 0; i < node_count; i++) {
		QGNode *n = from->nodes[i];
		/* If the entity already exists in the "to" graph, do nothing.
		 * We could have more complex logic to merge entity data, but this is not
		 * currently necessary as this logic only benefits toString calls like EXPLAIN. */
		if(QueryGraph_GetNodeByAlias(to, n->alias)) continue;
		// New entity, clone and add it.
		QueryGraph_AddNode(to, QGNode_Clone(n));
	}

	for(uint i = 0; i < edge_count; i++) {
		QGEdge *e = from->edges[i];
		/* If the entity already exists in the "to" graph, do nothing.
		 * We could have more complex logic to merge entity data, but this is not
		 * currently necessary as this logic only benefits toString calls like EXPLAIN. */
		if(QueryGraph_GetEdgeByAlias(to, e->alias)) continue;

		// Retrieve the edge's endpoints in the "to" graph.
		QGNode *src = QueryGraph_GetNodeByAlias(to, e->src->alias);
		QGNode *dest = QueryGraph_GetNodeByAlias(to, e->dest->alias);
		// Clone and add the unmatched edge.
		QGEdge *clone_edge = QGEdge_Clone(e);
		QueryGraph_ConnectNodes(to, src, dest, clone_edge);
	}
}

QGNode *QueryGraph_GetNodeByAlias(const QueryGraph *qg, const char *alias) {
	uint node_count = QueryGraph_NodeCount(qg);
	for(uint i = 0; i < node_count; i ++) {
		if(!RG_STRCMP(qg->nodes[i]->alias, alias)) return qg->nodes[i];
	}
	return NULL;
}

QGEdge *QueryGraph_GetEdgeByAlias(const QueryGraph *qg, const char *alias) {
	uint edge_count = QueryGraph_EdgeCount(qg);
	for(uint i = 0; i < edge_count; i ++) {
		if(!RG_STRCMP(qg->edges[i]->alias, alias)) return qg->edges[i];
	}
	return NULL;
}

EntityType QueryGraph_GetEntityTypeByAlias(const QueryGraph *qg, const char *alias) {
	if(QueryGraph_GetNodeByAlias(qg, alias) != NULL) return ENTITY_NODE;
	if(QueryGraph_GetEdgeByAlias(qg, alias) != NULL) return ENTITY_EDGE;
	return ENTITY_UNKNOWN;
}

void QueryGraph_ResolveUnknownRelIDs(QueryGraph *qg) {
	// No unknown relationships - no need to updated.
	if(!qg->unknown_reltype_ids) return;

	Schema *s = NULL;
	bool unkown_relationships = false;
	GraphContext *gc = QueryCtx_GetGraphCtx();
	uint edge_count = QueryGraph_EdgeCount(qg);

	// Update edges.
	for(uint i = 0; i < edge_count; i++) {
		QGEdge *edge = qg->edges[i];
		uint rel_types_count = array_len(edge->reltypeIDs);
		for(uint j = 0; j < rel_types_count; j++) {
			if(edge->reltypeIDs[j] == GRAPH_UNKNOWN_RELATION) {
				s = GraphContext_GetSchema(gc, edge->reltypes[j], SCHEMA_EDGE);
				if(s) edge->reltypeIDs[j] = s->id;
				else unkown_relationships = true; // Cannot update the unkown relationship.
			}
		}
	}

	qg->unknown_reltype_ids = unkown_relationships;
}

QueryGraph *QueryGraph_Clone(const QueryGraph *qg) {
	uint node_count = QueryGraph_NodeCount(qg);
	uint edge_count = QueryGraph_EdgeCount(qg);
	QueryGraph *clone = QueryGraph_New(node_count, edge_count);

	// Clone nodes.
	for(uint i = 0; i < node_count; i++) {
		// Clones node without its edges.
		QGNode *n = QGNode_Clone(qg->nodes[i]);
		QueryGraph_AddNode(clone, n);
	}

	// Clone edges.
	for(uint i = 0; i < edge_count; i++) {
		QGEdge *e = qg->edges[i];
		QGNode *src = QueryGraph_GetNodeByAlias(clone, e->src->alias);
		QGNode *dest = QueryGraph_GetNodeByAlias(clone, e->dest->alias);
		ASSERT(src != NULL && dest != NULL);
		QGEdge *clone_edge = QGEdge_Clone(e);
		QueryGraph_ConnectNodes(clone, src, dest, clone_edge);
	}

	return clone;
}

QGNode *QueryGraph_RemoveNode(QueryGraph *qg, QGNode *n) {
	ASSERT(qg != NULL && n != NULL);

	/* Remove node from query graph.
	 * Remove and free all edges associated with node. */
	uint incoming_edge_count = array_len(n->incoming_edges);
	for(uint i = 0; i < incoming_edge_count; i++) {
		QGEdge *e = n->incoming_edges[i];
		QueryGraph_RemoveEdge(qg, e);
		QGEdge_Free(e);
	}

	uint outgoing_edge_count = array_len(n->outgoing_edges);
	for(uint i = 0; i < outgoing_edge_count; i++) {
		QGEdge *e = n->outgoing_edges[i];
		QueryGraph_RemoveEdge(qg, e);
		QGEdge_Free(e);
	}

	// Remove node from graph nodes.
	uint node_count = QueryGraph_NodeCount(qg);
	uint i = 0;
	for(; i < node_count; i++) {
		if(n == qg->nodes[i]) {
			array_del_fast(qg->nodes, i);
			break;
		}
	}

	return n;
}

QGEdge *QueryGraph_RemoveEdge(QueryGraph *qg, QGEdge *e) {
	ASSERT(qg != NULL && e != NULL);

	// Disconnect nodes connected by edge.
	QGNode_RemoveOutgoingEdge(e->src, e);
	QGNode_RemoveIncomingEdge(e->dest, e);

	/* Remove edge from query graph. */
	uint edge_count = QueryGraph_EdgeCount(qg);
	uint i = 0;
	for(; i < edge_count; i++) {
		if(e == qg->edges[i]) {
			array_del_fast(qg->edges, i);
			break;
		}
	}
	return e;
}

QueryGraph **QueryGraph_ConnectedComponents(const QueryGraph *qg) {
	QGNode *n;                              // Current node.
	QGNode **q = array_new(QGNode *, 1);    // Node frontier.
	void *seen;                             // Has node been visited?
	QueryGraph *g = QueryGraph_Clone(qg);   // Clone query graph.
	rax *visited;                           // Dictionary of visited nodes.
	QueryGraph **connected_components;      // List of connected components.

	// At least one connected component (the original graph).
	connected_components = array_new(QueryGraph *, 1);

	// As long as there are nodes to process.
	while(true) {
		visited = raxNew();

		// Get a random node and add it to the frontier.
		QGNode *s = g->nodes[0];
		q = array_append(q, s);

		// As long as there are nodes in the frontier.
		while(array_len(q) > 0) {
			n = array_pop(q);

			// Mark n as visited.
			if(!raxInsert(visited, (unsigned char *)n->alias, strlen(n->alias), NULL, NULL)) {
				// We've already processed n.
				continue;
			}

			// Expand node N by visiting all of its neighbors
			for(int i = 0; i < array_len(n->outgoing_edges); i++) {
				QGEdge *e = n->outgoing_edges[i];
				seen = raxFind(visited, (unsigned char *)e->dest->alias, strlen(e->dest->alias));
				if(seen == raxNotFound) q = array_append(q, e->dest);
			}
			for(int i = 0; i < array_len(n->incoming_edges); i++) {
				QGEdge *e = n->incoming_edges[i];
				seen = raxFind(visited, (unsigned char *)e->src->alias, strlen(e->src->alias));
				if(seen == raxNotFound) q = array_append(q, e->src);
			}
		}

		/* Visited comprise the connected component defined by S.
		 * Remove all non-reachable nodes from current connected component.
		 * Remove connected component from graph. */
		QueryGraph *cc = QueryGraph_Clone(g);
		uint node_count = QueryGraph_NodeCount(g);
		for(uint i = 0; i < node_count; i++) {
			void *reachable;
			n = g->nodes[i];
			reachable = raxFind(visited, (unsigned char *)n->alias, strlen(n->alias));

			/* If node is reachable, which means it is part of the
			 * connected component, then remove it from the graph,
			 * otherwise, node isn't reachable, not part of the
			 * connected component. */
			if(reachable != raxNotFound) {
				QGNode *removed = QueryGraph_RemoveNode(g, n);
				QGNode_Free(removed);
			} else {
				n = QueryGraph_GetNodeByAlias(cc, n->alias);
				QueryGraph_RemoveNode(cc, n);
				QGNode_Free(n);
			}
		}

		connected_components = array_append(connected_components, cc);

		// Clear visited dict for next iteration.
		raxFree(visited);

		// Exit when graph is empty.
		if(QueryGraph_NodeCount(g) == 0) break;
	}

	array_free(q);
	QueryGraph_Free(g);

	return connected_components;
}

uint QueryGraph_NodeCount(const QueryGraph *qg) {
	return array_len(qg->nodes);
}

/* Retrieve the number of edges in a QueryGraph. */
uint QueryGraph_EdgeCount(const QueryGraph *qg) {
	return array_len(qg->edges);
}

GrB_Matrix QueryGraph_MatrixRepresentation(const QueryGraph *qg) {
	ASSERT(qg != NULL);

	// Make a clone of the given graph as we're about to modify it.
	QueryGraph *qg_clone = QueryGraph_Clone(qg);

	// Give an ID for each node, abuse of `labelID`.
	uint node_count = QueryGraph_NodeCount(qg_clone);
	for(uint i = 0; i < node_count; i++) qg_clone->nodes[i]->labelID = i;

	GrB_Matrix m;   // Matrix representation of QueryGraph.
	GrB_Info res = GrB_Matrix_new(&m, GrB_BOOL, node_count, node_count);
	UNUSED(res);
	ASSERT(res == GrB_SUCCESS);

	// Build matrix representation of query graph.
	for(uint i = 0; i < node_count; i++) {
		const QGNode *n = qg_clone->nodes[i];
		GrB_Index src = n->labelID;
		uint outgoing_degree = QGNode_OutgoingDegree(n);

		for(uint j = 0; j < outgoing_degree; j++) {
			const QGEdge *e = n->outgoing_edges[j];
			GrB_Index dest = e->dest->labelID;
			// Populate `m`.
			res = GrB_Matrix_setElement_BOOL(m, true, src, dest);
			ASSERT(res == GrB_SUCCESS);
		}
	}

	QueryGraph_Free(qg_clone);
	return m;
}

void QueryGraph_Print(const QueryGraph *qg) {
	char *buff = calloc(1024, sizeof(char));

	uint node_count = QueryGraph_NodeCount(qg);
	uint edge_count = QueryGraph_EdgeCount(qg);

	for(int i = 0; i < node_count; i++) {
		QGNode *n = qg->nodes[i];
		if(QGNode_IncomeDegree(n) + QGNode_OutgoingDegree(n) == 0) {
			// Floating node.
			asprintf(&buff, "%s%s;\n", buff, n->alias);
		}
	}

	for(int i = 0; i < edge_count; i++) {
		QGEdge *e = qg->edges[i];
		asprintf(&buff, "%s%s -> %s;\n", buff, e->src->alias, e->dest->alias);
	}

	printf("%s\n", buff);
	free(buff);
}

/* Frees entire graph. */
void QueryGraph_Free(QueryGraph *qg) {
	if(!qg) return;

	/* Free QueryGraph nodes. */
	uint nodeCount = QueryGraph_NodeCount(qg);
	for(uint i = 0; i < nodeCount; i ++) {
		QGNode_Free(qg->nodes[i]);
	}

	/* Free QueryGraph edges. */
	uint edgeCount = QueryGraph_EdgeCount(qg);
	for(uint i = 0; i < edgeCount; i ++) {
		QGEdge_Free(qg->edges[i]);
	}

	array_free(qg->nodes);
	array_free(qg->edges);
	rm_free(qg);
}

