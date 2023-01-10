#include "module.h"

#include "../redismodule.h"
#include "../RG.h"

int module_reply_map
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const long key_value_count
) {
    ASSERT(ctx);
    if (!ctx) {
        return REDISMODULE_ERR;
    }

    if (is_compact_mode) {
        REDISMODULE_DO(RedisModule_ReplyWithArray(ctx, key_value_count));
    } else {
        REDISMODULE_DO(RedisModule_ReplyWithMap(ctx, key_value_count));
    }

    return REDISMODULE_OK;
}

void module_reply_map_set_postponed_length
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const long length
) {
    ASSERT(ctx);
    ASSERT(length != REDISMODULE_POSTPONED_LEN);
    if (!ctx || length == REDISMODULE_POSTPONED_LEN) {
        return;
    }

    if (is_compact_mode) {
        RedisModule_ReplySetArrayLength(ctx, length);
    } else {
        RedisModule_ReplySetMapLength(ctx, length);
    }

    return;
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
        REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, key));
    }

    REDISMODULE_DO(RedisModule_ReplyWithLongLong(ctx, value));

    return REDISMODULE_OK;
}

int module_reply_key_value_numbers
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const char *key,
    const int64_t *values,
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
        REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, key));
    }

    REDISMODULE_DO(RedisModule_ReplyWithArray(ctx, length));
    for (size_t i = 0; i < length; ++i) {
        REDISMODULE_DO(RedisModule_ReplyWithLongLong(ctx, values[i]));
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
        REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, key));
    }

    REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, value));

    return REDISMODULE_OK;
}

ReplyRecorder ReplyRecorder_New
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    bool *has_started
) {
    const ReplyRecorder recorder = {
        .context = ctx,
        .is_compact_mode = is_compact_mode,
        .element_count = 0
    };

    const int status = module_reply_map(
        ctx,
        is_compact_mode,
        REDISMODULE_POSTPONED_LEN
    );

    ASSERT(status == REDISMODULE_OK);
    if (has_started) {
        *has_started = status == REDISMODULE_OK;
    }

    return recorder;
}

int ReplyRecorder_AddNumber
(
    ReplyRecorder *recorder,
    const char *key,
    const long long value
) {
    ASSERT(recorder);
    if (!recorder || !recorder->context) {
        return REDISMODULE_ERR;
    }
    return module_reply_key_value_number(
        recorder->context,
        recorder->is_compact_mode,
        key,
        value
    );
}

int ReplyRecorder_AddNumbers
(
    ReplyRecorder *recorder,
    const char *key,
    const long long values[],
    const size_t values_count
) {
    ASSERT(recorder);
    if (!recorder || !recorder->context) {
        return REDISMODULE_ERR;
    }

    return module_reply_key_value_numbers(
        recorder->context,
        recorder->is_compact_mode,
        key,
        values,
        values_count
    );
}

void ReplyRecorder_Finish(const ReplyRecorder recorder) {
    module_reply_map_set_postponed_length(
        recorder.context,
        recorder.is_compact_mode,
        recorder.element_count
    );
}
