/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "module.h"

#include "../redismodule.h"
#include "../RG.h"

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
    const long key_value_count
) {
    int ret = module_reply_map_resp2(ctx, key_value_count);
    ASSERT(ret == REDISMODULE_OK);
    UNUSED(ret);

    return REDISMODULE_OK;
}

void module_reply_map_set_postponed_length
(
    RedisModuleCtx *ctx,
    const long length
) {
    ASSERT(ctx);
    ASSERT(length != REDISMODULE_POSTPONED_LEN);
    if (!ctx || length == REDISMODULE_POSTPONED_LEN) {
        return;
    }

    RedisModule_ReplySetArrayLength(ctx, length * 2);
}

int module_reply_key_value_number
(
    RedisModuleCtx *ctx,
    const char *key,
    const long long value
) {
    ASSERT(ctx);
    ASSERT(key);

    if (!ctx || !key) {
        return REDISMODULE_ERR;
    }

    REDISMODULE_ASSERT(RedisModule_ReplyWithCString(ctx, key));

    REDISMODULE_ASSERT(RedisModule_ReplyWithLongLong(ctx, value));

    return REDISMODULE_OK;
}

int module_reply_key_value_numbers
(
    RedisModuleCtx *ctx,
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

    REDISMODULE_ASSERT(RedisModule_ReplyWithCString(ctx, key));

    REDISMODULE_ASSERT(RedisModule_ReplyWithArray(ctx, length));
    for (size_t i = 0; i < length; ++i) {
        REDISMODULE_ASSERT(RedisModule_ReplyWithLongLong(ctx, values[i]));
    }

    return REDISMODULE_OK;
}

int module_reply_key_value_string
(
    RedisModuleCtx *ctx,
    const char *key,
    const char *value
) {
    ASSERT(ctx);
    ASSERT(key);
    ASSERT(value);

    REDISMODULE_ASSERT(RedisModule_ReplyWithCString(ctx, key));

    REDISMODULE_ASSERT(RedisModule_ReplyWithCString(ctx, value));

    return REDISMODULE_OK;
}

int module_reply_key_value_bool
(
    RedisModuleCtx *ctx,
    const char *key,
    const bool value
) {
    ASSERT(ctx);
    ASSERT(key);

    REDISMODULE_ASSERT(RedisModule_ReplyWithCString(ctx, key));
    REDISMODULE_ASSERT(RedisModule_ReplyWithLongLong(ctx, value ? 1 : 0));

    return REDISMODULE_OK;
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
    ASSERT(ctx != NULL);

    return module_reply_bool_resp2(ctx, value);

    return REDISMODULE_OK;
}

int ReplyRecorder_New
(
    ReplyRecorder *recorder,
    RedisModuleCtx *ctx
) {
    ASSERT(recorder && ctx && "Recorder and ctx should be passed.");
    if (!recorder || !ctx) {
        return REDISMODULE_ERR;
    }

    recorder->context = ctx;
    recorder->element_count = 0;

    REDISMODULE_ASSERT(module_reply_map(
        ctx,
        REDISMODULE_POSTPONED_LEN
    ));

    return REDISMODULE_OK;
}

int ReplyRecorder_AddBool
(
    ReplyRecorder *recorder,
    const char *key,
    const bool value
) {
    ASSERT(recorder && key);
    if (!recorder || !recorder->context) {
        return REDISMODULE_ERR;
    }

    REDISMODULE_ASSERT(module_reply_key_value_bool(
        recorder->context,
        key,
        value
    ));

    recorder->element_count++;

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
