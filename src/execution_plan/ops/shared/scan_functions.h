/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

// Storage struct for label data in node and index scans.
typedef struct {
	const char *alias;   // Alias of the node being traversed.
	const char *label;   // Label of the node being traversed.
	int label_id;        // Label ID of the node being traversed.
} LabeledNodeCtx;

// Instantiate a new labeled node context.
#define NODE_CTX_NEW(_alias, _label, _label_id)  \
(LabeledNodeCtx) {                               \
	.alias = (_alias),                           \
	.label = (_label),                           \
	.label_id = (_label_id)                      \
}

