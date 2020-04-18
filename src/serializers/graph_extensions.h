/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../graph/graph.h"

// Sets a node in the graph
void Graph_SetNode(Graph *g, NodeID id, int label, Node *n);

// Set a given edge in the graph.
void Graph_SetEdge(Graph *g, EdgeID edge_id, NodeID src, NodeID dest, int r, Edge *e);

// Marks a node ID as deleted.
void Graph_MarkNodeDeleted(Graph *g, NodeID ID);

// Marks a edge ID as deleted.
void Graph_MarkEdgeDeleted(Graph *g, EdgeID ID);

// Try adding a label matrix.
void Graph_TryAddLabelMatrix(Graph *g, int label);
