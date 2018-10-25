/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef GRAPH_H
#define GRAPH_H

#include <pthread.h>

#include "entities/node.h"
#include "entities/edge.h"
#include "../redismodule.h"
#include "../util/uthash.h"
#include "../util/triemap/triemap.h"
#include "../util/datablock/datablock.h"
#include "../util/datablock/datablock_iterator.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"

#define GRAPH_DEFAULT_NODE_CAP 16384    // Default number of nodes a graph can hold before resizing.
#define GRAPH_DEFAULT_RELATION_CAP 16   // Default number of different relationships a graph can hold before resizing.
#define GRAPH_DEFAULT_LABEL_CAP 16      // Default number of different labels a graph can hold before resizing.
#define GRAPH_NO_LABEL -1               // Labels are numbered [0-N], -1 represents no label.
#define GRAPH_NO_RELATION -1            // Relations are numbered [0-N], -1 represents no relation.

typedef struct {
    DataBlock *nodes;               // Graph nodes stored in blocks.
    DataBlock *edges;               // Graph edges stored in blocks.
    Edge *_edgesHashTbl;            // Hash table containing edges.
    GrB_Matrix adjacency_matrix;    // Adjacency matrix, holds all graph connections.
    GrB_Matrix *_relations;         // Relation matrices.
    size_t relation_count;          // Number of relation matrices.
    GrB_Matrix *_labels;            // Label matrices.
    size_t label_count;             // Number of label matrices.    
    pthread_mutex_t _mutex;         // Mutex for accessing critical sections.
} Graph;

// Create a new graph.
Graph *Graph_New (
    size_t n    // Initial number of nodes in the graph.
);

// Returns number of nodes in the graph.
size_t Graph_NodeCount(const Graph *g);

// Returns number of edges in the graph.
size_t Graph_EdgeCount(const Graph *g);

// Creates N new nodes.
// Returns node iterator.
void Graph_CreateNodes (
    Graph* g,               // Graph for which nodes will be added.
    size_t n,               // Number of nodes to create.
    int* labels,            // Lables Node i with label i.
    DataBlockIterator **it  // [Optional] iterator over new nodes.
);

// Connects src[i] to dest[i] with edge of type relation[i].
void Graph_ConnectNodes (
        Graph *g,                   // Graph in which connections are formed. 
        EdgeDesc *connections,      // Array of triplets (src_id, dest_id, relation).
        size_t connectionCount,     // Number of elements in connections array.
        DataBlockIterator **edges   // Pointer to edge iterator.
);

// Retrieves node with given id from graph,
// Returns NULL if node wasn't found.
Node *Graph_GetNode (
    const Graph *g,
    NodeID id
);

// Retrieves edge with given id from graph,
// Returns NULL if edge wasn't found.
Edge *Graph_GetEdge (
    const Graph *g,
    EdgeID id
);

// Retrieves edges connecting source to destination,
// relation is optional, pass GRAPH_NO_RELATION if you do not care
// of edge type.
void Graph_GetEdgesConnectingNodes (
    const Graph *g,
    NodeID src,
    NodeID dest,
    int relation,
    Vector *edges
);

typedef enum {
    GRAPH_EDGE_DIR_INCOMING,
    GRAPH_EDGE_DIR_OUTGOING,
    GRAPH_EDGE_DIR_BOTH,
} GRAPH_EDGE_DIR;
// Get node edges.
void Graph_GetNodeEdges (
    const Graph *g,
    const Node *n,
    Vector *edges,
    GRAPH_EDGE_DIR dir,
    int edgeType
);

// Removes a set of nodes and all of their connections
// within the graph.
void Graph_DeleteNodes(Graph *g, NodeID *IDs, size_t IDCount);

// Removes edge connecting src node to dest node.
void Graph_DeleteEdges (
    Graph *g,
    EdgeID *IDs,
    size_t IDCount
);

// Label all nodes between start_node_id and end_node_id
// with given label.
void Graph_LabelNodes (
    Graph *g,
    NodeID start_node_id,
    NodeID end_node_id,
    int label
);

// Retrieves a node iterator which can be used to access
// every node in the graph.
DataBlockIterator *Graph_ScanNodes (
    const Graph *g
);

// Retrieves an edge iterator which can be used to access
// every edge in the graph.
DataBlockIterator *Graph_ScanEdges (
    const Graph *g
);

// Creates a new label matrix, returns id given to label.
int Graph_AddLabel (
    Graph *g
);

// Retrieves the adjacency matrix.
// Matrix is resized if its size doesn't match graph's node count.
GrB_Matrix Graph_GetAdjacencyMatrix (
    const Graph *g
);

// Retrieves a label matrix.
// Matrix is resized if its size doesn't match graph's node count.
GrB_Matrix Graph_GetLabel (
    const Graph *g,     // Graph from which to get adjacency matrix.
    int label           // Label described by matrix.
);

// Retrieves a typed adjacency matrix.
// Matrix is resized if its size doesn't match graph's node count.
GrB_Matrix Graph_GetRelation (
    const Graph *g,     // Graph from which to get adjacency matrix.
    int relation        // Relation described by matrix.
);

// Creates a new relation matrix, returns id given to relation.
int Graph_AddRelationType (
    Graph *g
);

// Commit GraphBLAS pending operations.
void Graph_CommitPendingOps (
    Graph *g
);

// Free graph.
void Graph_Free (
    Graph *g
);

#endif
