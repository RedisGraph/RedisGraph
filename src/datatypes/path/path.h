/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../graph/entities/node.h"
#include "../../graph/entities/edge.h"

typedef struct {
	Node *nodes;    // Nodes in paths.
	Edge *edges;    // Edges in path.
} Path;

/**
 * @brief  Creates a new Path with given capacity.
 * @param  len: length parameter.
 * @retval A pointer to an empty path with given capacity.
 */
Path *Path_New(size_t len);

void Path_EnsureLen(Path *p, size_t len);

/**
 * @brief  Appends a node to the path.
 * @param  *p: Path address.
 * @param  n: Given Node.
 */
void Path_AppendNode(Path *p, Node n);

/**
 * @brief  Appends a relationship to the path.
 * @param  *p: Path address.
 * @param  e: Given edge.
 */
void Path_AppendEdge(Path *p, Edge e);

void Path_SetNode(Path *p, uint i, Node n);

void Path_SetEdge(Path *p, uint i, Edge e);

/**
 * @brief  Returns a refernce to a node in the specific index.
 * @note   Index refers to the node's position in the path, regarding the nodes order left-to-right.
 *         Assertion error will be thrown if index is out of bounds.
 * @param  p: Path.
 * @param  index: Node position.
 * @retval Node refernce.
 */
Node *Path_GetNode(const Path *p, int index);

/**
 * @brief  Returns a refernce to an edge in the specific index.
 * @note   Index refers to the edge's position in the path, regarding the edges order left-to-right.
 *         Assertion error will be thrown if index is out of bounds.
 * @param  p: Path.
 * @param  index: Edge position.
 * @retval Edge reference.
 */
Edge *Path_GetEdge(const Path *p, int index);

/**
 * @brief  Removes the last node from the path.
 * @note   This action cause an inconsistency between the last edge and the new last node.
 *         Those actions should execute consecutively.
 * @param  p: Path.
 * @retval The removed node.
 */
Node Path_PopNode(Path *p);

/**
 * @brief  Removes the last edge from the path.
 * @note   This action cause an inconsistency between the last node and the new last edge.
 *         Those actions should execute consecutively.
 * @param  p: Path.
 * @retval The removed edge.
 */
Edge Path_PopEdge(Path *p);

/**
 * @brief  Returns the last node in the path.
 * @note   This function does not check for path boundaries. Do not use in case of empty path.
 * @param  p: Path.
 * @retval Last node in the path.
 */
Node Path_Head(Path *p);

/**
 * @brief  Returns the amount of nodes in the path.
 * @param  p: path.
 * @retval Amount of nodes in the path.
 */
size_t Path_NodeCount(const Path *p);

/**
 * @brief  Returns the amount of edges in the path.
 * @param  p: path.
 * @retval Amount of edges in the path.
 */
size_t Path_EdgeCount(const Path *p);

/**
 * @brief  Returns the path length - amount of edges.
 * @note   For path with one node, the length is zero. For n nodes, the length is n-1.
 * @param  p: path.
 * @retval Path length.
 */
size_t Path_Len(const Path *p);

/**
 * @brief  Returns if a path contains a node.
 * @param  p: Path
 * @param  *n: Node
 * @retval True if the node is in the path, false otherwise.
 */
bool Path_ContainsNode(const Path *p, Node *n);

/**
 * @brief  Clones a path.
 * @param  p: Origin path.
 * @retval A pointer to a path struct with newly allocated array copies.
 */
Path *Path_Clone(const Path *p);

/**
 * @brief  Reverse the order of the path.
 * @param  p: Path.
 * @retval None
 */
void Path_Reverse(Path *p);

void Path_Clear(Path *p);

/**
 * @brief  Deletes the path nodes and edges arrays.
 * @note   Do not delete path allocation itself.
 * @param  p: Path pointer.
 */
void Path_Free(Path *p);

