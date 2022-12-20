/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

/*
 * This file contains the useful statistics and generic information about a
 * a graph. This information is used by the "GRAPH.INFO" command.
*/
#include "info.h"

#include "util/arr.h"
#include "util/num.h"
#include "util/circular_buffer_nrg.h"

#include <sys/types.h>
#include "../query_ctx.h"

#include <string.h>

#include "util/thpool/pools.h"

#define INITIAL_QUERY_INFO_CAPACITY 100

#define REQUIRE_ARG_OR_RETURN(arg_name, return_value) \
    do { \
        const bool check = arg_name; \
        ASSERT(check && "#arg_name must be provided."); \
        if (!check) { \
            return return_value; \
        } \
    } while (0);

#define REQUIRE_ARG(arg_name) \
    do { \
        const bool check = arg_name; \
        ASSERT(check && "#arg_name must be provided."); \
        if (!check) { \
            return; \
        } \
    } while (0);

#define REQUIRE_TRUE(arg_name) \
    do { \
        const bool check = arg_name; \
        ASSERT(check && "#arg_name must be true."); \
        if (!check) { \
            return; \
        } \
    } while (0);

#define REQUIRE_TRUE_OR_RETURN(arg_name, return_value) \
    do { \
        const bool check = arg_name; \
        ASSERT(check && "#arg_name must be true."); \
        if (!check) { \
            return return_value; \
        } \
    } while (0);

static bool _lock_rwlock(pthread_rwlock_t *, const bool);
static bool _unlock_rwlock(pthread_rwlock_t *);

static CircularBufferNRG finished_queries;
static pthread_rwlock_t finished_queries_rwlock = PTHREAD_RWLOCK_INITIALIZER;

FinishedQueryInfo FinishedQueryInfo_FromQueryInfo(const QueryInfo info) {
    ASSERT(info.context);

    FinishedQueryInfo finished;
    memset(&finished, 0, sizeof(FinishedQueryInfo));

    finished.executing_time_milliseconds = info.executing_time_milliseconds;
    finished.received_unix_timestamp_milliseconds = info.received_unix_timestamp_milliseconds;
    finished.reporting_time_milliseconds = info.reporting_time_milliseconds;
    finished.waiting_time_milliseconds = info.waiting_time_milliseconds;

    if (info.context) {
        finished.query_string = strdup(info.context->query_data.query);
    }

    return finished;
}

void FinishedQueryInfo_Free(const FinishedQueryInfo query_info) {
    if (query_info.query_string) {
        free(query_info.query_string);
    }
}

static void _add_finished_query(const FinishedQueryInfo info) {
    const bool locked = _lock_rwlock(&finished_queries_rwlock, true);
    ASSERT(locked);
    if (!locked) {
        return;
    }
    CircularBufferNRG_Add(finished_queries, (const void*)&info);
    const bool unlocked = _unlock_rwlock(&finished_queries_rwlock);
    ASSERT(unlocked);
}

static bool _lock_rwlock(pthread_rwlock_t *lock, const bool is_write) {
    REQUIRE_ARG_OR_RETURN(lock, false);

    if (is_write) {
        return !pthread_rwlock_wrlock(lock);
    }

    return !pthread_rwlock_rdlock(lock);
}

static bool _unlock_rwlock(pthread_rwlock_t *lock) {
    REQUIRE_ARG_OR_RETURN(lock, false);

    return !pthread_rwlock_unlock(lock);
}

QueryInfo QueryInfo_New(void) {
    QueryInfo query_info = {
        .received_unix_timestamp_milliseconds = 0,
        .waiting_time_milliseconds = 0,
        .executing_time_milliseconds = 0,
        .reporting_time_milliseconds = 0,
        .context = NULL,
        .stage_timer = {}
    };
    TIMER_RESTART(query_info.stage_timer);
    return query_info;
}

void QueryInfo_SetQueryContext
(
    QueryInfo *query_info,
    const struct QueryCtx *query_ctx
) {
    REQUIRE_ARG(query_info);

    query_info->context = query_ctx;
}

const QueryCtx* QueryInfo_GetQueryContext(const QueryInfo *query_info) {
    REQUIRE_ARG_OR_RETURN(query_info, NULL);
    return query_info->context;
}

bool QueryInfo_IsValid(const QueryInfo *query_info) {
    return QueryInfo_GetQueryContext(query_info);
}

uint64_t QueryInfo_GetReceivedTimestamp(const QueryInfo info) {
    return info.received_unix_timestamp_milliseconds;
}

uint64_t QueryInfo_GetTotalTimeSpent(const QueryInfo info, bool *is_ok) {
    uint64_t total_time_spent = 0;

    if (!checked_add_u64(
        total_time_spent,
        info.waiting_time_milliseconds,
        &total_time_spent)) {
        // We have a value overflow.
        if (is_ok) {
            *is_ok = false;
            return 0;
        }
    }

    if (!checked_add_u64(
        total_time_spent,
        info.executing_time_milliseconds,
        &total_time_spent)) {
        // We have a value overflow.
        if (is_ok) {
            *is_ok = false;
            return 0;
        }
    }

    if (!checked_add_u64(
        total_time_spent,
        info.reporting_time_milliseconds,
        &total_time_spent)) {
        // We have a value overflow.
        if (is_ok) {
            *is_ok = false;
            return 0;
        }
    }

    return total_time_spent;
}

uint64_t QueryInfo_GetWaitingTime(const QueryInfo info) {
    return info.waiting_time_milliseconds;
}

uint64_t QueryInfo_GetExecutionTime(const QueryInfo info) {
    return info.executing_time_milliseconds;
}

uint64_t QueryInfo_GetReportingTime(const QueryInfo info) {
    return info.reporting_time_milliseconds;
}

void QueryInfo_UpdateWaitingTime(QueryInfo *info) {
    REQUIRE_ARG(info);
    info->waiting_time_milliseconds += QueryInfo_ResetStageTimer(info);
}

void QueryInfo_UpdateExecutionTime(QueryInfo *info) {
    REQUIRE_ARG(info);
    info->executing_time_milliseconds += QueryInfo_ResetStageTimer(info);
}

void QueryInfo_UpdateReportingTime(QueryInfo *info) {
    REQUIRE_ARG(info);
    info->reporting_time_milliseconds += QueryInfo_ResetStageTimer(info);
}

uint64_t QueryInfo_ResetStageTimer(QueryInfo *info) {
    REQUIRE_ARG_OR_RETURN(info, 0);
    const uint64_t milliseconds_spent
        = TIMER_GET_ELAPSED_MILLISECONDS(info->stage_timer);
    TIMER_RESTART(info->stage_timer);
    return milliseconds_spent;
}

QueryInfoStorage QueryInfoStorage_NewWithCapacity(const uint64_t capacity) {
    QueryInfoStorage storage;
    storage.queries = array_new(QueryInfo, capacity);
    return storage;
}

QueryInfoStorage QueryInfoStorage_NewWithLength(const uint64_t length) {
    QueryInfoStorage storage;
    storage.queries = array_newlen(QueryInfo, length);

    return storage;
}

QueryInfoStorage QueryInfoStorage_New() {
    return QueryInfoStorage_NewWithCapacity(INITIAL_QUERY_INFO_CAPACITY);
}

void QueryInfoStorage_Clear(QueryInfoStorage *storage) {
    REQUIRE_ARG(storage);

    array_clear(storage->queries);
}

void QueryInfoStorage_Free(QueryInfoStorage *storage) {
    REQUIRE_ARG(storage);

    array_free(storage->queries);
}

uint32_t QueryInfoStorage_ValidCount(const QueryInfoStorage *storage) {
    REQUIRE_ARG_OR_RETURN(storage, 0);

    const uint32_t length = QueryInfoStorage_Length(storage);

    uint64_t valid_elements_count = 0;
    for (uint32_t i = 0; i < length; ++i) {
        QueryInfo *info = QueryInfoStorage_Get(storage, i);
        REQUIRE_ARG_OR_RETURN(info, 0);
        if (QueryInfo_IsValid(info)) {
            ++valid_elements_count;
        }
    }

    return valid_elements_count;
}

uint32_t QueryInfoStorage_Length(const QueryInfoStorage *storage) {
    REQUIRE_ARG_OR_RETURN(storage, 0);
    return array_len(storage->queries);
}

void QueryInfoStorage_Add(QueryInfoStorage *storage, const QueryInfo info) {
    REQUIRE_ARG(storage);
    // TODO check for duplicates?
    array_append(storage->queries, info);
}

void QueryInfoStorage_SetCapacity
(
    QueryInfoStorage *storage,
    const uint32_t capacity
) {
    REQUIRE_ARG(storage);

    storage->queries = array_reset_cap(storage->queries, capacity);
    ASSERT(storage->queries && "Couldn't reset capacity.");
}

bool QueryInfoStorage_Set
(
    QueryInfoStorage *storage,
    const uint64_t index,
    const QueryInfo value
) {
    REQUIRE_ARG_OR_RETURN(storage, false);
    const bool is_enough_space = index < QueryInfoStorage_Length(storage);
    REQUIRE_TRUE_OR_RETURN(is_enough_space, false);

    QueryInfo *info = array_elem(storage->queries, index);
    memcpy(info, &value, sizeof(QueryInfo));

    return true;
}

QueryInfo* QueryInfoStorage_Get
(
    const QueryInfoStorage *storage,
    const uint64_t index
) {
    REQUIRE_ARG_OR_RETURN(storage, false);
    const bool is_enough_space = index < QueryInfoStorage_Length(storage);
    REQUIRE_TRUE_OR_RETURN(is_enough_space, false);

    return array_elem(storage->queries, index);
}

static bool _QueryInfoStorage_ResetAll(QueryInfoStorage *storage) {
    REQUIRE_ARG_OR_RETURN(storage, false);

    for (uint64_t i = 0; i < QueryInfoStorage_Length(storage); ++i) {
        if (!QueryInfoStorage_ResetElement(storage, i)) {
            return false;
        }
    }

    return true;
}

bool QueryInfoStorage_ResetElement
(
    QueryInfoStorage *storage,
    const uint64_t index
) {
    REQUIRE_ARG_OR_RETURN(storage, false);
    const bool is_enough_space = index < QueryInfoStorage_Length(storage);
    REQUIRE_TRUE_OR_RETURN(is_enough_space, false);

    memset(QueryInfoStorage_Get(storage, index), 0, sizeof(QueryInfo));

    return true;
}

bool QueryInfoStorage_Remove(QueryInfoStorage *storage, const QueryInfo *info) {
    return QueryInfoStorage_RemoveByContext(
        storage,
        QueryInfo_GetQueryContext(info));
}

bool QueryInfoStorage_RemoveByContext
(
    QueryInfoStorage *storage,
    const struct QueryCtx *context
) {
    REQUIRE_ARG_OR_RETURN(storage, false);
    REQUIRE_ARG_OR_RETURN(context, false);

    for (uint32_t i = 0; i < array_len(storage->queries); ++i) {
        const QueryInfo query_info = storage->queries[i];
        if (QueryInfo_GetQueryContext(&query_info) == context) {
            array_del(storage->queries, i);
            return true;
        }
    }

    return false;
}

QueryInfoIterator QueryInfoIterator_NewStartingAt
(
    QueryInfoStorage *storage,
    const uint64_t index
) {
    ASSERT(storage && "Storage has to be provided");
    const uint64_t length = QueryInfoStorage_Length(storage);
    const bool is_index_valid = index < length;
    if (length > 0) {
        ASSERT(is_index_valid && "Index is going out of bounds.");
        if (!storage || !is_index_valid) {
            const QueryInfoIterator iterator = {
                .storage = NULL,
                .current_index = 0,
                .has_started = false
            };
            return iterator;
        }
    }

    const QueryInfoIterator iterator = {
        .storage = storage,
        .current_index = index,
        .has_started = false
    };
    return iterator;
}

QueryInfoIterator QueryInfoIterator_New(QueryInfoStorage *storage) {
    return QueryInfoIterator_NewStartingAt(storage, 0);
}

QueryInfo* QueryInfoIterator_Next(QueryInfoIterator *iterator) {
    REQUIRE_ARG_OR_RETURN(iterator, NULL);

    const uint64_t length = QueryInfoStorage_Length(iterator->storage);
    if (iterator->current_index >= length) {
        return NULL;
    }

    if (!iterator->has_started) {
        return QueryInfoIterator_Get(iterator);
    }

    const uint64_t next_index = iterator->current_index + 1;
    const bool is_index_valid = next_index < length;

    if (!is_index_valid) {
        return NULL;
    }

    iterator->current_index = next_index;
    return QueryInfoIterator_Get(iterator);
}

QueryInfo* QueryInfoIterator_NextValid(QueryInfoIterator *iterator) {
    QueryInfo *next = QueryInfoIterator_Next(iterator);

    while (next && !QueryInfo_IsValid(next)) {
        next = QueryInfoIterator_Next(iterator);
    }

    return next;
}

QueryInfoStorage * const QueryInfoIterator_GetStorage
(
    QueryInfoIterator *iterator
) {
    REQUIRE_ARG_OR_RETURN(iterator, NULL);

    return iterator->storage;
}

QueryInfo* QueryInfoIterator_Get(QueryInfoIterator *iterator) {
    REQUIRE_ARG_OR_RETURN(iterator, NULL);
    iterator->has_started = true;
    return QueryInfoStorage_Get(iterator->storage, iterator->current_index);
}

uint32_t QueryInfoIterator_Length(const QueryInfoIterator *iterator) {
    REQUIRE_ARG_OR_RETURN(iterator, 0);
    return QueryInfoStorage_Length(iterator->storage) - iterator->current_index;
}

bool QueryInfoIterator_IsExhausted(const QueryInfoIterator *iterator) {
    return QueryInfoIterator_Length(iterator) > 0;
}

static bool _Info_LockEverything(Info *info, const bool is_write) {
    REQUIRE_ARG_OR_RETURN(info, false);
    return _lock_rwlock(&info->inverse_global_lock, !is_write);
}

static bool _Info_UnlockEverything(Info *info) {
    REQUIRE_ARG_OR_RETURN(info, false);
    return _unlock_rwlock(&info->inverse_global_lock);
}

static bool _Info_LockWaitingQueries(Info *info, const bool is_write) {
    REQUIRE_ARG_OR_RETURN(info, false);
    return _lock_rwlock(&info->waiting_queries_rwlock, is_write);
}

static bool _Info_UnlockWaitingQueries(Info *info) {
    REQUIRE_ARG_OR_RETURN(info, false);
    return _unlock_rwlock(&info->waiting_queries_rwlock);
}

bool Info_New(Info *info) {
    REQUIRE_ARG_OR_RETURN(info, false);
    // Compensate for the main thread.
    const uint64_t thread_count = ThreadPools_ThreadCount() + 1;

    info->waiting_queries = QueryInfoStorage_New();
    info->executing_queries_per_thread
        = QueryInfoStorage_NewWithLength(thread_count);
    info->reporting_queries_per_thread
        = QueryInfoStorage_NewWithLength(thread_count);
    _QueryInfoStorage_ResetAll(&info->executing_queries_per_thread);
    _QueryInfoStorage_ResetAll(&info->reporting_queries_per_thread);

    bool lock_initialized = !pthread_rwlock_init(
        &info->waiting_queries_rwlock,
        NULL);

    REQUIRE_TRUE_OR_RETURN(lock_initialized, false);

   lock_initialized = !pthread_rwlock_init(
        &info->inverse_global_lock,
        NULL);

    REQUIRE_TRUE_OR_RETURN(lock_initialized, false);

    return true;
}

void Info_Reset(Info *info) {
    REQUIRE_ARG(info);

    // TODO delete if there is nothing to reset?
}

bool Info_Free(Info *info) {
    REQUIRE_ARG_OR_RETURN(info, false);
    REQUIRE_TRUE_OR_RETURN(_Info_LockEverything(info, true), false);

    QueryInfoStorage_Free(&info->waiting_queries);
    QueryInfoStorage_Free(&info->executing_queries_per_thread);
    QueryInfoStorage_Free(&info->reporting_queries_per_thread);
    bool lock_destroyed = !pthread_rwlock_destroy(
        &info->waiting_queries_rwlock);
    ASSERT(lock_destroyed);
    lock_destroyed = !pthread_rwlock_destroy(
        &info->inverse_global_lock);
    ASSERT(lock_destroyed);

    REQUIRE_TRUE_OR_RETURN(_Info_UnlockEverything(info), false);

    return true;
}

void Info_AddWaitingQueryInfo
(
    Info *info,
    const struct QueryCtx *query_context,
    const uint64_t waiting_time_milliseconds,
    const uint64_t received_unix_timestamp_milliseconds
) {
    REQUIRE_ARG(info);

    REQUIRE_TRUE(_Info_LockEverything(info, true));
    REQUIRE_TRUE(_Info_LockWaitingQueries(info, true));

    QueryInfo query_info = QueryInfo_New();
    QueryInfo_SetQueryContext(&query_info, query_context);
    query_info.waiting_time_milliseconds += waiting_time_milliseconds;
    query_info.received_unix_timestamp_milliseconds = received_unix_timestamp_milliseconds;
    QueryInfoStorage_Add(&info->waiting_queries, query_info);

    REQUIRE_TRUE(_Info_UnlockWaitingQueries(info));
    REQUIRE_TRUE(_Info_UnlockEverything(info));
}

bool _Info_MoveQueryInfoBetweenStorages
(
    QueryInfoStorage *from,
    QueryInfoStorage *to,
    const uint64_t index
) {
    REQUIRE_ARG_OR_RETURN(from, false);
    REQUIRE_ARG_OR_RETURN(to, false);
    QueryInfo *from_element = QueryInfoStorage_Get(from, index);
    const bool set = QueryInfoStorage_Set(to, index, *from_element);
    REQUIRE_ARG_OR_RETURN(set, false);
    const bool reset = QueryInfoStorage_ResetElement(from, index);
    REQUIRE_ARG_OR_RETURN(reset, false);

    return true;
}

void Info_IndicateQueryStartedExecution
(
    Info *info,
    const struct QueryCtx *context
) {
    REQUIRE_ARG(info);
    REQUIRE_ARG(context);
    REQUIRE_TRUE(_Info_LockEverything(info, true));
    REQUIRE_TRUE(_Info_LockWaitingQueries(info, true));

    // This effectively moves the query info object from the waiting queue
    // to the executing queue, recording the time spent waiting.

    QueryInfoStorage storage = info->waiting_queries;
    const int thread_id = ThreadPools_GetThreadID();

    for (uint32_t i = 0; i < array_len(storage.queries); ++i) {
        QueryInfo query_info = storage.queries[i];
        if (QueryInfo_GetQueryContext(&query_info) == context) {
            QueryInfo_UpdateWaitingTime(&query_info);
            array_del(storage.queries, i);
            const bool set = QueryInfoStorage_Set(
                &info->executing_queries_per_thread,
                thread_id,
                query_info);
            ASSERT(set);
            if (!set) {
                _Info_UnlockWaitingQueries(info);
                _Info_UnlockEverything(info);
                return;
            }
            break;
        }
    }

    REQUIRE_TRUE(_Info_UnlockWaitingQueries(info));
    REQUIRE_TRUE(_Info_UnlockEverything(info));
}

void Info_IndicateQueryStartedReporting
(
    Info *info,
    const struct QueryCtx *context
) {
    REQUIRE_ARG(info);
    REQUIRE_ARG(context);
    REQUIRE_TRUE(_Info_LockEverything(info, true));

    // This effectively moves the query info object from the executing queue
    // to the reporting queue, recording the time spent executing.

    const int thread_id = ThreadPools_GetThreadID();
    QueryInfo *query_info = QueryInfoStorage_Get(
        &info->executing_queries_per_thread,
        thread_id);
    ASSERT(query_info && QueryInfo_GetQueryContext(query_info) == context);
    if (!query_info || QueryInfo_GetQueryContext(query_info) != context) {
        REQUIRE_TRUE(_Info_UnlockEverything(info));
        return;
    }
    QueryInfo_UpdateExecutionTime(query_info);

    const bool moved = _Info_MoveQueryInfoBetweenStorages(
        &info->executing_queries_per_thread,
        &info->reporting_queries_per_thread,
        thread_id);
    ASSERT(moved);
    REQUIRE_TRUE(_Info_UnlockEverything(info));
}

void Info_IndicateQueryFinishedReporting
(
    Info *info,
    const struct QueryCtx *context
) {
    REQUIRE_ARG(info);
    REQUIRE_ARG(context);
    REQUIRE_TRUE(_Info_LockEverything(info, true));

    // This effectively removes the query info object from the reporting queue.

    const int thread_id = ThreadPools_GetThreadID();
    QueryInfo *query_info = QueryInfoStorage_Get(
        &info->reporting_queries_per_thread,
        thread_id);
    const bool is_ours
        = query_info && QueryInfo_GetQueryContext(query_info) == context;
    ASSERT(is_ours);
    if (!is_ours) {
        REQUIRE_TRUE(_Info_UnlockEverything(info));
        return;
    }
    QueryInfo_UpdateReportingTime(query_info);
    FinishedQueryInfo finished = FinishedQueryInfo_FromQueryInfo(*query_info);
    _add_finished_query(finished);
    QueryInfoStorage_ResetElement(
        &info->reporting_queries_per_thread,
        thread_id);
    REQUIRE_TRUE(_Info_UnlockEverything(info));
}

uint64_t Info_GetTotalQueriesCount(const Info* info) {
    REQUIRE_ARG_OR_RETURN(info, 0);

    const uint64_t waiting = Info_GetWaitingQueriesCount(info);
    const uint64_t executing = Info_GetExecutingQueriesCount(info);
    const uint64_t reporting = Info_GetReportingQueriesCount(info);

    uint64_t total = waiting;
    bool added = checked_add_u64(total, executing, &total);
    REQUIRE_TRUE_OR_RETURN(added, 0);
    added = checked_add_u64(total, reporting, &total);
    REQUIRE_TRUE_OR_RETURN(added, 0);

    return total;
}

uint64_t Info_GetWaitingQueriesCount(const Info *info) {
    REQUIRE_ARG_OR_RETURN(info, 0);

    REQUIRE_TRUE_OR_RETURN(_Info_LockWaitingQueries(info, false), 0);

    const uint64_t count = QueryInfoStorage_Length(&info->waiting_queries);

    REQUIRE_TRUE_OR_RETURN(_Info_UnlockWaitingQueries(info), 0);

    return count;
}

uint64_t Info_GetExecutingQueriesCount(const Info *info) {
    REQUIRE_ARG_OR_RETURN(info, 0);

    REQUIRE_TRUE_OR_RETURN(_Info_LockEverything(info, true), 0);

    const uint64_t count
        = QueryInfoStorage_ValidCount(&info->executing_queries_per_thread);

    REQUIRE_TRUE_OR_RETURN(_Info_UnlockEverything(info), 0);

    return count;
}

uint64_t Info_GetReportingQueriesCount(const Info *info) {
    REQUIRE_ARG_OR_RETURN(info, 0);

    REQUIRE_TRUE_OR_RETURN(_Info_LockEverything(info, true), 0);

    const uint64_t count
        = QueryInfoStorage_ValidCount(&info->reporting_queries_per_thread);

    REQUIRE_TRUE_OR_RETURN(_Info_UnlockEverything(info), 0);

    return count;
}

uint64_t Info_GetMaxQueryWaitTime(const Info *info) {
    REQUIRE_ARG_OR_RETURN(info, 0);
    _Info_LockEverything(info, false);

    uint64_t max_time = 0;
    QueryInfo *query = NULL;
    {
        REQUIRE_TRUE_OR_RETURN(_Info_LockWaitingQueries(info, false), 0);
        QueryInfoIterator iterator = QueryInfoIterator_New(&info->waiting_queries);
        while ((query = QueryInfoIterator_NextValid(&iterator)) != NULL) {
            max_time = MAX(max_time, QueryInfo_GetWaitingTime(*query));
        }
        REQUIRE_TRUE_OR_RETURN(_Info_UnlockWaitingQueries(info), 0);
    }
    {
        QueryInfoIterator iterator = QueryInfoIterator_New(&info->executing_queries_per_thread);
        while ((query = QueryInfoIterator_NextValid(&iterator)) != NULL) {
            max_time = MAX(max_time, QueryInfo_GetWaitingTime(*query));
        }
    }
    {
        QueryInfoIterator iterator = QueryInfoIterator_New(&info->reporting_queries_per_thread);
        while ((query = QueryInfoIterator_NextValid(&iterator)) != NULL) {
            max_time = MAX(max_time, QueryInfo_GetWaitingTime(*query));
        }
    }

    _Info_UnlockEverything(info);

    return max_time;
}

bool Info_Lock(Info *info) {
    return _Info_LockEverything(info, false);
}

bool Info_Unlock(Info *info) {
    return _Info_UnlockEverything(info);
}

QueryInfoStorage* Info_GetWaitingQueriesStorage(Info *info) {
    REQUIRE_ARG_OR_RETURN(info, NULL);
    return &info->waiting_queries;
}

QueryInfoStorage* Info_GetExecutingQueriesStorage(Info *info) {
    REQUIRE_ARG_OR_RETURN(info, NULL);
    return &info->executing_queries_per_thread;
}

QueryInfoStorage* Info_GetReportingQueriesStorage(Info *info) {
    REQUIRE_ARG_OR_RETURN(info, NULL);
    return &info->reporting_queries_per_thread;
}

static void _FinishedQueryInfoDeleter(void *info) {
    ASSERT(info);
    if (info) {
        FinishedQueryInfo *query_info = (FinishedQueryInfo*)info;
        FinishedQueryInfo_Free(*query_info);
    }
}

void Info_SetCapacityForFinishedQueriesStorage(const uint32_t count) {
    const bool locked = _lock_rwlock(&finished_queries_rwlock, true);
    ASSERT(locked);
    if (!locked) {
        return;
    }
    if (!finished_queries) {
        finished_queries = CircularBufferNRG_New(sizeof(FinishedQueryInfo), count);
        CircularBufferNRG_SetDeleter(
            finished_queries,
            _FinishedQueryInfoDeleter,
            NULL);
    } else {
        CircularBufferNRG_SetCapacity(&finished_queries, count);
    }
    const bool unlocked = _unlock_rwlock(&finished_queries_rwlock);
    ASSERT(unlocked);
}
