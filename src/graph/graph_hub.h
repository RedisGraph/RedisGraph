/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "graphcontext.h"

// graph hub responsible for crud operations on a graph
// while updating relevant components e.g. indexes and undo log

// create a node
// set the node labels and attributes
// add the node to the relevant indexes
// add node creation operation to undo-log
// return the # of attributes set
uint CreateNode
(
	GraphContext *gc, // graph context to create the node
	Node *n,          // output node created
	LabelID *labels,  // node labels
	uint label_count, // labels count
	AttributeSet set  // node attributes
);

// create an edge
// set the edge src, dst endpoints and attributes
// add the edge to the relevant indexes
// add edge creation operation to undo-log
// return the # of attributes set
uint CreateEdge
(
	GraphContext *gc, // graph context to create the edge
	Edge *e,          // output edge created
	NodeID src,       // edge source
	NodeID dst,       // edge destination
	int r,            // edge relation type
	AttributeSet set  // edge attributes
);

// delete a node
// remove the node from the relevant indexes
// add node deletion operation to undo-log
// return 1 on success, 0 otherwise
uint DeleteNode
(
	GraphContext *gc,  // graph context to delete the node
	Node *n            // the node to be deleted
);

// delete an edge
// delete the edge from the graph
// delete the edge from the relevant indexes
// add edge deletion operation to undo-log
// return the # of edges deleted
int DeleteEdge
(
	GraphContext *gc,  // graph context to delete the edge
	Edge *e            // the edge to be deleted
);

// update an entity(node/edge)
// update the entity attributes
// update the relevant indexes of the entity
// add entity update operations to undo log
void UpdateEntityProperties
(
	GraphContext *gc,             // graph context to update the entity
	GraphEntity *ge,              // the entity to be updated
	const AttributeSet set,       // attributes to update
	GraphEntityType entity_type,  // the entity type (node/edge)
	uint *props_set_count,        // number of properties set (out param)
	uint *props_removed_count     // number of properties removed (out param)
);


// this function sets the labels given in the rax "labels" to the given node
// creates the label matrix if not exists
// adds node to the label matrix
// updates the relevant indexes of the entity
void UpdateNodeLabels
(
	GraphContext *gc,            // graph context to update the entity
	Node *node,                  // the node to be updated
	const char **add_labels,     // labels to add to the node
	const char **remove_labels,  // labels to add to the node
	uint *labels_added_count,    // number of labels added (out param)
	uint *labels_removed_count   // number of labels removed (out param)
);

// Adds a schema to the graph. The schema is tracked by the undo log
// so in case of error it will be deleted.
Schema *AddSchema
(
	GraphContext *gc,             // graph context to add the schema
	const char *label,            // schema label
	SchemaType t                  // schema type (node/edge)
);

// Find or adding attribute. If there is a need to add an attribute to the graph
// the attribute is tracked by the undo log so in case of error it will be deleted.
Attribute_ID FindOrAddAttribute
(
	GraphContext *gc,             // graph context to add the attribute
	const char *attribute         // attribute name
);