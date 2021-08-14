/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../graph/graph.h"

// sets a node in the graph
void Serializer_Graph_SetNode
(
	Graph *g,               // graph to add node to
	NodeID id,              // node ID
	int label,              // node label
	Node *n                 // pointer to node
);

// set a given edge in the graph
void Serializer_Graph_SetEdge
(
	Graph *g,               // graph to add edge to
	int64_t multi_edge,     // true if graph supports multi-edge
	EdgeID edge_id,         // edge ID
	NodeID src,             // edge source
	NodeID dest,            // edge destination
	int r,                  // edge relationship-type
	Edge *e                 // pointer to edge
);

// marks a node ID as deleted
void Serializer_Graph_MarkNodeDeleted
(
	Graph *g,               // graph from which to mark node as deleted
	NodeID ID               // node ID
);

// marks a edge ID as deleted
void Serializer_Graph_MarkEdgeDeleted
(
	Graph *g,               // graph from which to mark edge as deleted
	EdgeID ID               // edge ID
);

// returns the graph deleted nodes list
uint64_t *Serializer_Graph_GetDeletedNodesList
(
	Graph *g
);

// returns the graph deleted nodes list
uint64_t *Serializer_Graph_GetDeletedEdgesList
(
	Graph *g
);

=======
// Sets a node in the graph
void Serializer_Graph_SetNode
(
	Graph *g,
	NodeID id,
	int label,
	Node *n
);

// Set a given edge in the graph.
void Serializer_Graph_SetEdge
(
	Graph *g,
	bool multi_edge,
	EdgeID edge_id,
	NodeID src,
	NodeID dest,
	int r,
	Edge *e
);

// Marks a node ID as deleted.
void Serializer_Graph_MarkNodeDeleted
(
	Graph *g,
	NodeID ID
);

// Marks a edge ID as deleted.
void Serializer_Graph_MarkEdgeDeleted
(
	Graph *g,
	EdgeID ID
);

// Returns the graph deleted nodes list.
uint64_t *Serializer_Graph_GetDeletedNodesList
(
	Graph *g
);

// Returns the graph deleted nodes list.
uint64_t *Serializer_Graph_GetDeletedEdgesList
(
	Graph *g
);
