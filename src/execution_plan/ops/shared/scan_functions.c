/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "scan_functions.h"
#include "../../../util/rmalloc.h"
#include "../../../graph/entities/qg_node.h"

// allocates and returns a new context
NodeScanCtx *NodeScanCtx_New
(
    char *alias,       // alias
    char *label,       // label
    LabelID label_id,  // label id
    QGNode *n          // node
) {
    NodeScanCtx *ctx = rm_malloc(sizeof(NodeScanCtx));
    ctx->alias = alias;
    ctx->label = label;
    ctx->label_id = label_id;
    ctx->n = n;

    return ctx;
}

// clones a context
NodeScanCtx *NodeScanCtx_Clone
(
    NodeScanCtx *ctx  // context
) {
    NodeScanCtx *clone = rm_malloc(sizeof(NodeScanCtx));
    clone->alias = ctx->alias;
    clone->label = ctx->label;
    clone->label_id = ctx->label_id;
    clone->n = QGNode_Clone(ctx->n);

    return clone;
}

// frees a context
void NodeScanCtx_Free
(
    NodeScanCtx *ctx  // context
) {
    if(ctx->n != NULL) {
        QGNode_Free(ctx->n);
        ctx->n = NULL;
    }
}
