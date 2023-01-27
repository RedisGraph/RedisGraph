/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */
 /*
 This module supports RESP3 and RESP2. When replying with RESP3's map
 the RESP3 functions are used. If those are unavailable, the map is
 emulated with RESP2's arrays. When using RESP2, the booleans are
 emulated as integers having 1 or 0 value for true and false values
 correspondingly.
 */

#include "module.h"

#include "../redismodule.h"
#include "../RG.h"

static int module_reply_map_resp3
(
    RedisModuleCtx *ctx,
    const long key_value_count
) {
    ASSERT(ctx);
    if (!ctx || !RedisModule_ReplyWithMap) {
        return REDISMODULE_ERR;
    }

    return RedisModule_ReplyWithMap(ctx, key_value_count);
}

static int module_reply_map_resp2
(
    RedisModuleCtx *ctx,
    const long key_value_count
) {
    ASSERT(ctx);
    if (!ctx) {
        return REDISMODULE_ERR;
    }

    if (key_value_count == REDISMODULE_POSTPONED_LEN) {
        return RedisModule_ReplyWithArray(ctx, key_value_count);
    }

    return RedisModule_ReplyWithArray(ctx, key_value_count * 2);
}

int module_reply_map
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const long key_value_count
) {
    if (is_compact_mode) {
        REDISMODULE_ASSERT(RedisModule_ReplyWithArray(ctx, key_value_count));
    } else if (module_reply_map_resp3(ctx, key_value_count) == REDISMODULE_ERR) {
        REDISMODULE_ASSERT(module_reply_map_resp2(ctx, key_value_count));
    }

    return REDISMODULE_OK;
}

static bool module_reply_map_set_postponed_length_resp3
(
    RedisModuleCtx *ctx,
    const long length
) {
    ASSERT(ctx);
    ASSERT(length != REDISMODULE_POSTPONED_LEN);
    if (!ctx
     || length == REDISMODULE_POSTPONED_LEN
     || !RedisModule_ReplySetMapLength) {
        return false;
    }

    RedisModule_ReplySetMapLength(ctx, length);

    return true;
}

void module_reply_map_set_postponed_length
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const long length
) {
    ASSERT(ctx);
    ASSERT(length != REDISMODULE_POSTPONED_LEN);
    if (!ctx
     || length == REDISMODULE_POSTPONED_LEN) {
        return;
    }

    if (is_compact_mode) {
        RedisModule_ReplySetArrayLength(ctx, length);
    } else if (!module_reply_map_set_postponed_length_resp3(ctx, length)) {
        // In RESP2 the map is emulated via an array having twice the
        // size, as we need to store keys and values as separate array
        // entries.
        RedisModule_ReplySetArrayLength(ctx, length * 2);
    }
}

int module_reply_key_value_number
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const char *key,
    const long long value
) {
    ASSERT(ctx);
    ASSERT(key);

    if (!ctx || !key) {
        return REDISMODULE_ERR;
    }

    if (!is_compact_mode) {
        REDISMODULE_ASSERT(RedisModule_ReplyWithCString(ctx, key));
    }

    REDISMODULE_ASSERT(RedisModule_ReplyWithLongLong(ctx, value));

    return REDISMODULE_OK;
}

int module_reply_key_value_numbers
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const char *key,
    const long long *values,
    const size_t length
) {
    ASSERT(ctx);
    ASSERT(key);

    if (!ctx || !key) {
        return REDISMODULE_ERR;
    }
    if (!values || !length) {
        // Nothing to do.
        return REDISMODULE_OK;
    }

    if (!is_compact_mode) {
        REDISMODULE_ASSERT(RedisModule_ReplyWithCString(ctx, key));
    }

    REDISMODULE_ASSERT(RedisModule_ReplyWithArray(ctx, length));
    for (size_t i = 0; i < length; ++i) {
        REDISMODULE_ASSERT(RedisModule_ReplyWithLongLong(ctx, values[i]));
    }

    return REDISMODULE_OK;
}

int module_reply_key_value_string
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const char *key,
    const char *value
) {
    ASSERT(ctx);
    ASSERT(key);
    ASSERT(value);

    if (!ctx || !key || !value) {
        return REDISMODULE_ERR;
    }

    if (!is_compact_mode) {
        REDISMODULE_ASSERT(RedisModule_ReplyWithCString(ctx, key));
    }

    REDISMODULE_ASSERT(RedisModule_ReplyWithCString(ctx, value));

    return REDISMODULE_OK;
}

static int module_reply_bool_resp3
(
    RedisModuleCtx *ctx,
    const bool value
) {
    ASSERT(ctx);
    if (!ctx || !RedisModule_ReplyWithBool) {
        return REDISMODULE_ERR;
    }

    return RedisModule_ReplyWithBool(ctx, value);
}

static int module_reply_bool_resp2
(
    RedisModuleCtx *ctx,
    const bool value
) {
    ASSERT(ctx);
    if (!ctx || !RedisModule_ReplyWithLongLong) {
        return REDISMODULE_ERR;
    }

    return RedisModule_ReplyWithLongLong(ctx, value ? 1 : 0);
}

int module_reply_bool(
    RedisModuleCtx *ctx,
    const bool value
) {
    ASSERT(ctx);
    if (!ctx) {
        return REDISMODULE_ERR;
    }

    if (module_reply_bool_resp3(ctx, value) == REDISMODULE_ERR) {
        return module_reply_bool_resp2(ctx, value);
    }

    return REDISMODULE_OK;
}

int ReplyRecorder_New
(
    ReplyRecorder *recorder,
    RedisModuleCtx *ctx,
    const bool is_compact_mode
) {
    ASSERT(recorder && ctx && "Recorder and ctx should be passed.");
    if (!recorder || !ctx) {
        return REDISMODULE_ERR;
    }

    recorder->context = ctx;
    recorder->is_compact_mode = is_compact_mode;
    recorder->element_count = 0;

    REDISMODULE_ASSERT(module_reply_map(
        ctx,
        is_compact_mode,
        REDISMODULE_POSTPONED_LEN
    ));

    return REDISMODULE_OK;
}

int ReplyRecorder_AddNumber
(
    ReplyRecorder *recorder,
    const char *key,
    const long long value
) {
    ASSERT(recorder && key);
    if (!recorder || !recorder->context) {
        return REDISMODULE_ERR;
    }
    REDISMODULE_ASSERT(module_reply_key_value_number(
        recorder->context,
        recorder->is_compact_mode,
        key,
        value
    ));

    ++recorder->element_count;

    return REDISMODULE_OK;
}

int ReplyRecorder_AddString
(
    ReplyRecorder *recorder,
    const char *key,
    const char *value
) {
    ASSERT(recorder && recorder->context && key && value);
    if (!recorder || !recorder->context || !key || !value) {
        return REDISMODULE_ERR;
    }
    REDISMODULE_ASSERT(module_reply_key_value_string(
        recorder->context,
        recorder->is_compact_mode,
        key,
        value
    ));

    ++recorder->element_count;

    return REDISMODULE_OK;
}

int ReplyRecorder_AddNumbers
(
    ReplyRecorder *recorder,
    const char *key,
    const long long values[],
    const size_t values_count
) {
    ASSERT(recorder && recorder->context);
    if (!recorder || !recorder->context) {
        return REDISMODULE_ERR;
    }

    REDISMODULE_ASSERT(module_reply_key_value_numbers(
        recorder->context,
        recorder->is_compact_mode,
        key,
        values,
        values_count
    ));

    ++recorder->element_count;

    return REDISMODULE_OK;
}

void ReplyRecorder_Finish(const ReplyRecorder recorder) {
    module_reply_map_set_postponed_length(
        recorder.context,
        recorder.is_compact_mode,
        recorder.element_count
    );
}

void ReplyRecorder_Cleanup
(
    ReplyRecorder *recorder
) {
    ASSERT(recorder);
    if (!recorder) {
        return;
    }
    ReplyRecorder_Finish(*recorder);
}
