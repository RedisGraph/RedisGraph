/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "all_paths.h"
#include "all_shortest_paths.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"

// make sure context level array have 'cap' available entries
static void _AllPathsCtx_EnsureLevelArrayCap
(
	AllPathsCtx *ctx,
	uint level,
	uint cap
) {
	if(cap == 0) {
		return;
	}

	uint len = array_len(ctx->levels);
	if(level < len) {
		LevelConnection *current = ctx->levels[level];
		ctx->levels[level] = array_ensure_cap(current, array_len(current) + cap);
		return;
	}

	ASSERT(level == len);
	array_append(ctx->levels, array_new(LevelConnection, cap));
}

// append given 'node' to given 'level' array
static void _AllPathsCtx_AddConnectionToLevel
(
	AllPathsCtx *ctx,  // traversal context
	uint level,        // level
	Node *node,        // node to add
	Edge *edge         // edge to add
) {
	ASSERT(level < array_len(ctx->levels));

	LevelConnection conn;
	conn.node = *node;
	if(edge != NULL) {
		conn.edge = *edge;
	}

	array_append(ctx->levels[level], conn);
}

// Check to see if context levels array has entries at position 'level'.
static bool _AllPathsCtx_LevelNotEmpty
(
	const AllPathsCtx *ctx,
	uint level
) {
	return (level < array_len(ctx->levels) && array_len(ctx->levels[level]) > 0);
}

static void _CollectNeighbors
(
	Edge **edges,
	GRAPH_EDGE_DIR dir,
	AllPathsCtx *ctx,
	Node *frontier,
	uint32_t depth
) {
	// get frontier neighbors
	for(int i = 0; i < ctx->relationCount; i++) {
		Graph_GetNodeEdges(ctx->g, frontier, dir, ctx->relationIDs[i],
				edges);
	}

	Edge *_edges = *edges;
	// add unvisited neighbors to next level
	uint32_t n = array_len(_edges);

	//--------------------------------------------------------------------------
	// apply filter to edge
	//--------------------------------------------------------------------------

	if(ctx->ft) {
		for(uint32_t i = 0; i < n; i++) {
			Edge e = _edges[i];

			// update the record with the current edge
			Record_AddEdge(ctx->r, ctx->edge_idx, e);

			// drop edge if it doesn't passes filter
			if(FilterTree_applyFilters(ctx->ft, ctx->r) != FILTER_PASS) {
				array_del_fast(_edges, i);
				i--;
				n--;
			}
		}
	}

	_AllPathsCtx_EnsureLevelArrayCap(ctx, depth, n);
}

// traverse from the frontier node in the specified direction and add all
// encountered nodes and edges
void addNeighbors
(
	AllPathsCtx *ctx,
	LevelConnection *frontier,
	uint32_t depth,
	GRAPH_EDGE_DIR dir
) {
	Node n;
	Edge *e = NULL;
	uint32_t i = 0;
	uint32_t count = 0;

	switch(dir) {
		case GRAPH_EDGE_DIR_OUTGOING:
			_CollectNeighbors(&ctx->outgoing, GRAPH_EDGE_DIR_OUTGOING, ctx,
					&frontier->node, depth);

			//------------------------------------------------------------------
			// introduce outgoing entities
			//------------------------------------------------------------------

			count = array_len(ctx->outgoing);
			for(i = 0; i < count; i++) {
				e = ctx->outgoing + i;

				// set the neighbor node
				Graph_GetNode(ctx->g, Edge_GetDestNodeID(e), &n);

				// add node and edge to the frontier
				_AllPathsCtx_AddConnectionToLevel(ctx, depth, &n, e);
			}

			array_clear(ctx->outgoing);

			break;
		case GRAPH_EDGE_DIR_INCOMING:
			_CollectNeighbors(&ctx->incoming, GRAPH_EDGE_DIR_INCOMING, ctx,
					&frontier->node, depth);

			//------------------------------------------------------------------
			// introduce incoming entities
			//------------------------------------------------------------------

			count = array_len(ctx->incoming);
			for(i = 0; i < count; i++) {
				e = ctx->incoming + i;

				// set the neighbor node
				Graph_GetNode(ctx->g, Edge_GetSrcNodeID(e), &n);

				// add node and edge to the frontier
				_AllPathsCtx_AddConnectionToLevel(ctx, depth, &n, e);
			}

			array_clear(ctx->incoming);

			break;
		case GRAPH_EDGE_DIR_BOTH:
			_CollectNeighbors(&ctx->incoming, GRAPH_EDGE_DIR_INCOMING, ctx,
					&frontier->node, depth);
			_CollectNeighbors(&ctx->outgoing, GRAPH_EDGE_DIR_OUTGOING, ctx,
					&frontier->node, depth);

			//------------------------------------------------------------------
			// remove duplicates
			//------------------------------------------------------------------

			for(int i = 0; i < array_len(ctx->outgoing); i++) {
				EntityID out_id = ENTITY_GET_ID(ctx->outgoing + i);
				for(int j = 0; j < array_len(ctx->incoming); j++) {
					EntityID in_id = ENTITY_GET_ID(ctx->incoming + j);
					if(out_id == in_id) {
						array_del_fast(ctx->outgoing, i);
						i--;
						break;
					}
				}
			}

			//------------------------------------------------------------------
			// introduce outgoing entities
			//------------------------------------------------------------------

			count = array_len(ctx->outgoing);
			for(i = 0; i < count; i++) {
				e = ctx->outgoing + i;

				// set the neighbor node
				Graph_GetNode(ctx->g, Edge_GetDestNodeID(e), &n);

				// add node and edge to the frontier
				_AllPathsCtx_AddConnectionToLevel(ctx, depth, &n, e);
			}

			//------------------------------------------------------------------
			// introduce incoming entities
			//------------------------------------------------------------------

			count = array_len(ctx->incoming);
			for(i = 0; i < count; i++) {
				e = ctx->incoming + i;

				// set the neighbor node
				Graph_GetNode(ctx->g, Edge_GetSrcNodeID(e), &n);

				// add node and edge to the frontier
				_AllPathsCtx_AddConnectionToLevel(ctx, depth, &n, e);
			}

			array_clear(ctx->incoming);
			array_clear(ctx->outgoing);
			break;
		default:
			ASSERT(false && "encountered unexpected traversal direction in AllPaths");
			break;
	}
}

AllPathsCtx *AllPathsCtx_New
(
	Node *src,
	Node *dst,
	Graph *g,
	int *relationIDs,
	int relationCount,
	GRAPH_EDGE_DIR dir,
	uint minLen,
	uint maxLen,
	Record r,
	FT_FilterNode *ft,
	uint edge_idx,
	bool shortest_paths
) {
	ASSERT(src != NULL);

	AllPathsCtx *ctx = rm_malloc(sizeof(AllPathsCtx));
	ctx->g        = g;
	ctx->r        = r;
	ctx->ft       = ft;
	ctx->dir      = dir;
	ctx->edge_idx = edge_idx;

	// Cypher variable path "[:*min..max]"" specifies edge count
	// While the path constructed here contains only nodes.
	// As such a path which require min..max edges
	// should contain min+1..max+1 nodes.
	ctx->minLen         = minLen + 1;
	ctx->maxLen         = maxLen + 1;
	ctx->relationIDs    = relationIDs;
	ctx->relationCount  = relationCount;
	ctx->levels         = array_new(LevelConnection *, 1);
	ctx->path           = Path_New(1);
	ctx->incoming       = array_new(Edge, 32);
	ctx->outgoing       = array_new(Edge, 32);
	ctx->dst            = dst;
	ctx->shortest_paths = shortest_paths;
	ctx->visited        =  NULL;

	_AllPathsCtx_EnsureLevelArrayCap(ctx, 0, 1);
	_AllPathsCtx_AddConnectionToLevel(ctx, 0, src, NULL);

	if(ctx->shortest_paths) {
		if(dst == NULL) {
			// If the destination is NULL due to a scenario like a
			// failed optional match, no results will be produced
			ctx->maxLen = 0;
			return ctx;
		}
		// get the the minimum length between src and dst
		// then start the traversal from dst to src
		int min_path_len = AllShortestPaths_FindMinimumLength(ctx, src, dst);
		ctx->minLen = min_path_len;
		ctx->maxLen = min_path_len;
		ctx->dst    = src;
		if(dir == GRAPH_EDGE_DIR_INCOMING) {
			ctx->dir = GRAPH_EDGE_DIR_OUTGOING;
		} else if(dir == GRAPH_EDGE_DIR_OUTGOING) {
			ctx->dir = GRAPH_EDGE_DIR_INCOMING;
		}
		_AllPathsCtx_AddConnectionToLevel(ctx, 0, dst, NULL);
	}

	// in case we have filter tree validate that we can access the filtered edge
	ASSERT(!ctx->ft || ctx->edge_idx < Record_length(ctx->r));
	return ctx;
}

static Path *_AllPathsCtx_NextPath
(
	AllPathsCtx *ctx
) {
	// as long as path is not empty OR there are neighbors to traverse
	while(Path_NodeCount(ctx->path) ||         // path not empty
		  _AllPathsCtx_LevelNotEmpty(ctx, 0))  // additional nodes to consider
	{
		uint32_t depth = Path_NodeCount(ctx->path);

		// can we advance?
		if(_AllPathsCtx_LevelNotEmpty(ctx, depth)) {
			// get a new frontier
			LevelConnection frontierConnection = array_pop(ctx->levels[depth]);
			Edge frontierEdge = frontierConnection.edge;
			Node frontierNode = frontierConnection.node;

			// if depth is 0 this is the source node
			// there is no leading edge to it
			// for depth > 0 for each frontier node, there is a leading edge
			// do not allow the same edge to appear on the path more than once
			if(depth > 0) {
				if(Path_ContainsEdge(ctx->path, &frontierEdge)) {
					continue;
				} else {
					Path_AppendEdge(ctx->path, frontierEdge);
				}
			}

			// add frontier to path
			Path_AppendNode(ctx->path, frontierNode);

			// update path depth
			depth++;

			// introduce neighbors only if path depth < maximum path length
			if(depth < ctx->maxLen) {
				addNeighbors(ctx, &frontierConnection, depth, ctx->dir);
			}

			// see if we can return path.
			// note: that further calls to this function will continue
			// to operate on this path
			// so it is essential that the caller does not modify it
			// if future features like an algorithm API use this routine
			// they should either be responsible for memory safety
			// or a memory-safe boolean/routine should be offered
			if(depth >= ctx->minLen && depth <= ctx->maxLen) {
				if(ctx->dst != NULL) {
					// destination node is specified
					// make sure we've found requested destination
					Node dst = Path_Head(ctx->path);
					if(ENTITY_GET_ID(ctx->dst) != ENTITY_GET_ID(&dst)) {
						continue;
					}
				}
				return ctx->path;
			}
		} else {
			// no way to advance, backtrack
			Path_PopNode(ctx->path);
			if(Path_EdgeCount(ctx->path) > 0) {
				Path_PopEdge(ctx->path);
			}
		}
	}

	// couldn't find a path
	return NULL;
}

Path *AllPathsCtx_NextPath
(
	AllPathsCtx *ctx
) {
	if(!ctx || ctx->maxLen == 0) {
		return NULL;
	}

	if(ctx->shortest_paths) {
		return AllShortestPaths_NextPath(ctx);
	} else {
		return _AllPathsCtx_NextPath(ctx);
	}
}

void AllPathsCtx_Free
(
	AllPathsCtx *ctx
) {
	if(!ctx) {
		return;
	}

	uint32_t levelsCount = array_len(ctx->levels);
	for(int i = 0; i < levelsCount; i++) {
		array_free(ctx->levels[i]);
	}

	Path_Free(ctx->path);
	array_free(ctx->levels);
	array_free(ctx->incoming);
	array_free(ctx->outgoing);

	if(ctx->visited != NULL) {
		GrB_Vector_free(&ctx->visited);
	}

	rm_free(ctx);
	ctx = NULL;
}

