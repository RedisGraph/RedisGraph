/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "../../../graph/entities/node.h"

// Storage struct for label data in node and index scans.
typedef struct {
	QGNode *n;          // node to scan (might hold multiple labels)
	LabelID label_id;   // label ID of the node being traversed
	const char *alias;  // alias of the node being traversed
	const char *label;  // label of the node being traversed
} NodeScanCtx;

// instantiate a new labeled node context
#define NODE_CTX_NEW(_alias, _label, _label_id, _n)  \
(NodeScanCtx) {                                      \
	.alias = (_alias),                               \
	.label = (_label),                               \
	.label_id = (_label_id),                         \
	.n = (_n)                                        \
}

#define NODE_CTX_CLONE(_ctx)       \
({                                 \
	NodeScanCtx ctx = _ctx;        \
	ctx.n = QGNode_Clone(_ctx.n);  \
	ctx;                           \
})

#define NODE_CTX_FREE(_ctx)	(QGNode_Free(_ctx.n))
