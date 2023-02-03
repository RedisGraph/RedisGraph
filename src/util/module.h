/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

 /*
  * This file contains the useful stuff when working from within a redis module.
  * The reply mechanisms it has support RESP3 and RESP2.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct RedisModuleCtx RedisModuleCtx;


// A helpful macro to exit immediately if a RedisModule_* function
// returns with a result different from the REDISMODULE_OK value.
//
// This macro is different from the "REDISMODULE_ASSERT" in a way that it doesn't
// use the assertion and simply exits with the error code from the calling
// function. This is useful for branching rather than ensuring a problem won't
// happen at all costs.
#define REDISMODULE_DO(doable) \
    do { \
        const int ret = doable; \
        if (ret != REDISMODULE_OK) { \
            return ret; \
        } \
    } while(0);

// A helpful macro to exit immediately if a RedisModule_* function
// returns with a result different from the REDISMODULE_OK value.
//
// This is convenient to use when the error is expected NOT to happen, as this
// macro uses an assertion to quickly point to the problematic place where an
// error happens while it shouldn't.
#define REDISMODULE_ASSERT(doable) \
    do { \
        const int ret = doable; \
        ASSERT(ret == REDISMODULE_OK \
         && "Redis module function " #doable " returned an error."); \
        if (ret != REDISMODULE_OK) { \
            return ret; \
        } \
    } while(0);


// A helpful clean-up macro which specifies the ReplyRecorder destructor which
// flushes the reply upon deletion.
#define REPLY_AUTO_FINISH __attribute__ ((__cleanup__(ReplyRecorder_Cleanup)))


// Replies with either a map or an array, depending on the compact mode flag.
// Supports RESP3 and RESP2.
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
    const long long *values,
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

// Repleis with the boolean value. Supports RESP3 and RESP2.
int module_reply_bool(
    RedisModuleCtx *ctx,
    const bool value
);

// A convenient wrapper over the reply mechanism, storing the state of the reply
// until the flush is required. Supports compact mode and doesn't need a length
// of elements inside as it is counting on its own.
//
// As all other methods for replying already easy enough, this wrapper only
// allows replies in key-value pairs, where the key is omitted when the compact
// mode is used. For the compact mode it replies with just values within an
// array, otherwise with key-value pairs within a map.
//
// A good way of using it is to create an object with the REPLY_AUTO_FINISH
// destructor, which will flush the reply and send it to the client:
//
//     ReplyRecorder recorder REPLY_AUTO_FINISH;
//
//
typedef struct ReplyRecorder {
    RedisModuleCtx *context;
    bool is_compact_mode;
    uint64_t element_count;
} ReplyRecorder;

// Initializes the reply recorder passed.
// Returns REDISMODULE_OK on success, REDISMODULE_ERR otherwise.
int ReplyRecorder_New
(
    ReplyRecorder *recorder,
    RedisModuleCtx *ctx,
    const bool is_compact_mode
);

// Adds a new number to the reply set.
int ReplyRecorder_AddNumber
(
    ReplyRecorder *recorder,
    const char *key,
    const long long value
);

// Adds a new C string to the reply set.
int ReplyRecorder_AddString
(
    ReplyRecorder *recorder,
    const char *key,
    const char *value
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

// This is a callback function for when the ReplyRecorder object goes out of
// scope (object destructor).
void ReplyRecorder_Cleanup
(
    ReplyRecorder *recorder
);
