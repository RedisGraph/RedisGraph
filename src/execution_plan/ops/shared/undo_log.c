/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "undo_log.h"
#include "query_ctx.h"
#include "update_functions.h"

UndoLogCtx UndoLog_New() {
    UndoLogCtx ctx;
    ctx.undo_log = array_new(PendingUpdateCtx, 0);
    ctx.n_updates_commited = 0;
    return ctx;
}

void UndoLog_Free(UndoLogCtx *undo_log_ctx) {
    array_free(undo_log_ctx->undo_log);
    undo_log_ctx->n_updates_commited = 0;
    undo_log_ctx->undo_log = NULL;
}

// Adds to the undo_log the undo for the pending_update
// orig_value: The original value which pending_update is about to override
void UndoLog_Update(UndoLogCtx *undo_log_ctx, const PendingUpdateCtx *pending_update, const SIValue *orig_value) {
    // The undo is equal to pending_update except for the new value
    PendingUpdateCtx undo_update = *pending_update;
    undo_update.new_value = *orig_value;
    undo_log_ctx->undo_log = array_append(undo_log_ctx->undo_log, undo_update);
}

// Rollback the updates taken place by current query.
// This function is being called when timeout take place and the execution plan already drained
void UndoLog_Rollback_Updates(void) {
    QueryCtx *ctx = QueryCtx_GetQueryCtx();
    ASSERT(ctx);
    if(ctx->undo_log_ctx.n_updates_commited == 0) return;

    // We like to rollback only the commited updated so removing the uncommited ones.
    // Please note that n_updates_commited is inc before the update is being commited so we might
    // Rollback update that haven't being commited and it's ok because the undo updates are idempotent.
    ctx->undo_log_ctx.undo_log = array_trimm(ctx->undo_log_ctx.undo_log, ctx->undo_log_ctx.n_updates_commited, ARR_CAP_NOSHRINK);

    // Reverse the undo_log since we need to commit the update in reverse order for rollback correctness
    array_reverse(ctx->undo_log_ctx.undo_log);

    // lock everything
	QueryCtx_LockForCommit();
	{
		CommitUpdates(ctx->gc, QueryCtx_GetResultSetStatistics(), ctx->undo_log_ctx.undo_log, false, NULL);
	}
	// release lock
    // Here the execution plan is already drained so undo can pretend it's the last writer
	QueryCtx_UnlockCommit(ctx->internal_exec_ctx.last_writer);
}
