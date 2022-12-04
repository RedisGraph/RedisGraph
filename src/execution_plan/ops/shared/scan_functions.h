/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

// Storage struct for label data in node and index scans.
typedef struct {
	const char *alias;   // Alias of the node being traversed.
	const char *label;   // Label of the node being traversed.
	int label_id;        // Label ID of the node being traversed.
} NodeScanCtx;

// Instantiate a new labeled node context.
#define NODE_CTX_NEW(_alias, _label, _label_id)  \
(NodeScanCtx) {                                  \
	.alias = (_alias),                           \
	.label = (_label),                           \
	.label_id = (_label_id)                      \
}

