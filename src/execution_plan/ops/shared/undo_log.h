/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include <stdint.h>
#include "../../src/value.h"

struct PendingUpdateCtx;
typedef struct PendingUpdateCtx PendingUpdateCtx;

typedef struct {
    PendingUpdateCtx *undo_log;          // Undo log for updates, used in the case of timeout.
    uint64_t n_updates_commited;         // Number of commits which needs rollback.
} UndoLogCtx;

UndoLogCtx UndoLog_New();

void UndoLog_Free(UndoLogCtx *undo_log_ctx);

static inline void UndoLog_Inc_N_Updates_Commited(UndoLogCtx *undo_log_ctx) {
    undo_log_ctx->n_updates_commited++;
}

void UndoLog_Update(UndoLogCtx *undo_log_ctx, const PendingUpdateCtx *pending_update, const SIValue *orig_value);

void UndoLog_Rollback_Updates(void);
