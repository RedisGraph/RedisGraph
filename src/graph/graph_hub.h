/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "graphcontext.h"

uint CreateNode
(
	GraphContext *gc,
	Node *n,
	LabelID *labels,
	uint label_count,
	AttributeSet *props
);

uint CreateEdge
(
	GraphContext *gc,
	Edge *e,
	NodeID src,
	NodeID dst,
	int r,
	AttributeSet *props
);

uint DeleteNode
(
	GraphContext *gc,
	Node *n
);

int DeleteEdge
(
	GraphContext *gc,
	Edge *e
);

int UpdateEntity
(
	GraphContext *gc,
	GraphEntity *ge,
	AttributeSet *props,        // attribute to update
	GraphEntityType entity_type
);
