#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct RedisModuleCtx RedisModuleCtx;

// Helpful macros to exit immediately if a RedisModule_* function
// returns with a result different from the REDISMODULE_OK value.
#define REDISMODULE_DO(doable) \
    do { \
        const int ret = doable; \
        ASSERT(ret == REDISMODULE_OK \
         && "Redis module function " #doable " returned an error."); \
        if (ret != REDISMODULE_OK) { \
            return ret; \
        } \
    } while(0);


// Replies with either a map or an array, depending on the compact mode flag.
int module_reply_map
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const long key_value_count
);

// Sets the postponed value either for a map or an array, depending on the
// compact mode flag.
void module_reply_map_set_postponed_length
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const long length
);

// Replies with the key(C string)-value(long long) pair when the compact mode is
// off, and with just the value when the compact mode is on.
int module_reply_key_value_number
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const char *key,
    const long long value
);

// Replies with the key(C string)-value(long long) pair when the compact mode is
// off, and with just the value when the compact mode is on.
int module_reply_key_value_numbers
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const char *key,
    const int64_t *values,
    const size_t length
);

// Replies with the key(C string)-value(C string) pair when the compact mode is
// off, and with just the value when the compact mode is on.
int module_reply_key_value_string
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const char *key,
    const char *value
);


// A convinient wrapper over the reply mechanism, storing the state of the reply
// until the flush is required. Supports compact mode and doesn't need a length
// of elements inside as it is counting on its own.
//
// As all other methods for replying already easy enough, this wrapper only
// allows replies in key-value pairs, where the key is omitted when the compact
// mode is used. For the compact mode it replies with just values within an
// array, otherwise with key-value pairs within a map.
typedef struct ReplyRecorder {
    RedisModuleCtx *context;
    bool is_compact_mode;
    uint64_t element_count;
} ReplyRecorder;

// Creates a new reply recorder.
ReplyRecorder ReplyRecorder_New
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    bool *has_started
);

// Adds a new number to the reply set.
int ReplyRecorder_AddNumber
(
    ReplyRecorder *recorder,
    const char *key,
    const long long value
);

// Adds an array of numbers to the reply set.
int ReplyRecorder_AddNumbers
(
    ReplyRecorder *recorder,
    const char *key,
    const long long values[],
    const size_t values_count
);

// Finishes the reply recorder. This action sets the length of the destination
// data structure (a map or an array, depending on the compact mode), which,
// also flushes the reply to a client.
void ReplyRecorder_Finish
(
    const ReplyRecorder recorder
);
