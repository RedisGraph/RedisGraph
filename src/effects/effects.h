/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "../undo_log/undo_log.h"
#include "../graph/graphcontext.h"

#define EFFECTS_VERSION 1  // current effects encoding/decoding version

// size of the field in the structure
#define	fldsiz(name, field) \
	(sizeof(((struct name *)0)->field))

// types of effects
typedef enum {
	EFFECT_UNKNOWN = 0,    // unknown effect
	EFFECT_UPDATE_NODE,    // node update
	EFFECT_UPDATE_EDGE,    // edge update
	EFFECT_CREATE_NODE,    // node creation
	EFFECT_CREATE_EDGE,    // edge creation
	EFFECT_DELETE_NODE,    // node deletion
	EFFECT_DELETE_EDGE,    // edge deletion
	EFFECT_SET_LABELS,     // set labels
	EFFECT_REMOVE_LABELS,  // remove labels
	EFFECT_ADD_SCHEMA,     // schema addition
	EFFECT_ADD_ATTRIBUTE,  // add attribute
} EffectType;

//------------------------------------------------------------------------------
// effects
//------------------------------------------------------------------------------

// compute required effects buffer byte size from undo-log
size_t ComputeBufferSize
(
	const UndoLog undolog
);

// create a list of effects from the undo-log
u_char *Effects_FromUndoLog
(
	UndoLog log,
	size_t *l
);

// applys effects encoded in buffer
void Effects_Apply
(
	GraphContext *gc,          // graph to operate on
	const char *effects_buff,  // encoded effects
	size_t l                   // size of buffer
);

