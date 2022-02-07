/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "graphcontext.h"

// graph hub responsible for crud operations on a graph
// while updating relevant components e.g. indexes and undo log

// create a node
// this function set the node labels and attributes
// add the node to the relevant indexes
// and add node creation operation to undo log
// return the # of attributes set
uint CreateNode
(
	GraphContext *gc,    // graph context to create the node
	Node *n,             // output node created
	LabelID *labels,     // node labels
	uint label_count,    // labels count
	AttributeSet *props  // node attributes
);

// create a edge
// this function set the edge src and dst and attributes
// add the edge to the relevant indexes
// and add edge creation operation to undo log
// return the # of attributes set
uint CreateEdge
(
	GraphContext *gc,    // graph context to create the edge
	Edge *e,             // output edge created
	NodeID src,          // edge source
	NodeID dst,          // edge destination
	int r,               // edge relation type
	AttributeSet *props  // edge attributes
);

// delete a node
// this function delete the node from the graph
// delete the node from the relevant indexes
// and add node deletion operation to undo log
// return the # of implicit edges deleted
uint DeleteNode
(
	GraphContext *gc,    // graph context to delete the node
	Node *n              // the node to be deleted
);

// delete a edge
// this function delete the edge from the graph
// delete the edge from the relevant indexes
// and add edge deletion operation to undo log
// return the # of edges deleted
int DeleteEdge
(
	GraphContext *gc,    // graph context to delete the edge
	Edge *e              // the edge to be deleted
);

// update an entity(node/edge)
// this function update the entity attributes
// update the relevant indexes of the entity
// and add entity update operations to undo log
// return the # of properties updated
int UpdateEntity
(
	GraphContext *gc,           // graph context to update the entity
	GraphEntity *ge,            // the entity to be updated
	AttributeSet *props,        // attribute to update
	GraphEntityType entity_type // the entity type (node/edge)
);
