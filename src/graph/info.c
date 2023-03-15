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

#include <string.h>
#include "util/arr.h"
#include "util/num.h"
#include <sys/types.h>
#include "../query_ctx.h"
// #include "hdr/hdr_histogram.h"
#include "util/thpool/pools.h"
#include "util/circular_buffer_nrg.h"

#define INITIAL_QUERY_INFO_CAPACITY 100

#define REQUIRE_ARG_OR_RETURN(arg_name, return_value) \
    do { \
        const bool check = arg_name; \
        ASSERT(check && "#arg_name must be provided."); \
        if (!check) { \
            return return_value; \
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

// This specifies the unlocking destructor for the ScopedRwlock object.
// It is convenient to use when there are multiple branches of code which lead
// to an exit from the function, to avoid writing the unlocking code if the
// locking primitive is locked. It also reduces possible mistakes when the
// unlock is forgotten to be done as it is done automatically when this is used.
#define SCOPED_RWLOCK __attribute__ ((__cleanup__(_rwlock_cleanup)))

static bool _lock_rwlock(pthread_rwlock_t *, const bool);
static bool _unlock_rwlock(pthread_rwlock_t *);

// An rwlock wrapper to be used with the SCOPED_RWLOCK macro.
typedef struct ScopedRwlock {
    pthread_rwlock_t *lock;
    bool is_locked;
    bool is_write;
} ScopedRwlock;

static ScopedRwlock ScopedRwlock_New
(
    pthread_rwlock_t *lock,
    const bool is_write
) {
    ASSERT(lock);

    const ScopedRwlock scoped = {
        .lock = lock,
        .is_write = is_write,
        .is_locked = lock ? _lock_rwlock(lock, is_write) : false
    };

    return scoped;
}

// The cleanup function to be used by the `SCOPED_RWLOCK` macro for automatically
// unlocking an rwlock.
static void _rwlock_cleanup
(
    ScopedRwlock *scoped_lock
) {
    if (scoped_lock && scoped_lock->lock && scoped_lock->is_locked) {
        ASSERT(_unlock_rwlock(scoped_lock->lock));
    }
}

static CircularBufferNRG finished_queries;
static pthread_rwlock_t finished_queries_rwlock = PTHREAD_RWLOCK_INITIALIZER;

// returns the total number of queries recorded
uint64_t FinishedQueryCounters_GetTotalCount
(
	const FinishedQueryCounters *counters  // counters to sum up
) {
    return 
		   counters->write_failed_count       +
		   counters->write_timedout_count     +
		   counters->readonly_failed_count    +
		   counters->write_succeeded_count    +
		   counters->readonly_timedout_count  +
		   counters->readonly_succeeded_count ;
}

void FinishedQueryCounters_Add
(
    FinishedQueryCounters *lhs,
    const FinishedQueryCounters rhs
) {
    REQUIRE_ARG(lhs);

    lhs->readonly_failed_count += rhs.readonly_failed_count;
    lhs->readonly_succeeded_count += rhs.readonly_succeeded_count;
    lhs->readonly_timedout_count += rhs.readonly_timedout_count;
    lhs->write_failed_count += rhs.write_failed_count;
    lhs->write_succeeded_count += rhs.write_succeeded_count;
    lhs->write_timedout_count += rhs.write_timedout_count;
}

static void _FinishedQueryCounters_Reset
(
    FinishedQueryCounters *counters
) {
    REQUIRE_ARG(counters);

    counters->readonly_failed_count = 0;
    counters->readonly_succeeded_count = 0;
    counters->readonly_timedout_count = 0;
    counters->write_failed_count = 0;
    counters->write_succeeded_count = 0;
    counters->write_timedout_count = 0;
}

static void _FinishedQueryCounters_Increment(
    FinishedQueryCounters *counters,
    const QueryExecutionTypeFlag flags,
    const QueryExecutionStatus status
) {
    REQUIRE_ARG(counters);

    if (CHECK_FLAG(flags, QueryExecutionTypeFlag_WRITE)) {
        if (status == QueryExecutionStatus_FAILURE) {
            ++counters->write_failed_count;
        } else if (status == QueryExecutionStatus_TIMEDOUT) {
            ++counters->write_timedout_count;
        } else {
            ++counters->write_succeeded_count;
        }

        return;
    } else {
        if (status == QueryExecutionStatus_FAILURE) {
            ++counters->readonly_failed_count;
        } else if (status == QueryExecutionStatus_TIMEDOUT) {
            ++counters->readonly_timedout_count;
        } else {
            ++counters->readonly_succeeded_count;
        }

        return;
    }

    ASSERT(false && "Handle unknown flag.");
}

FinishedQueryInfo FinishedQueryInfo_FromQueryInfo(const QueryInfo info) {
    ASSERT(info.context);

    FinishedQueryInfo finished;
    memset(&finished, 0, sizeof(FinishedQueryInfo));

    finished.received_unix_timestamp_milliseconds = info.received_unix_timestamp_milliseconds;
    finished.total_wait_duration = info.wait_duration;
    finished.total_execution_duration = info.execution_duration;
    finished.total_report_duration = info.report_duration;

    if (info.context) {
        finished.query_string = strdup(info.context->query_data.query);
        finished.graph_name = strdup(info.context->gc->graph_name);
    }

    return finished;
}

millis_t _FinishedQueryInfo_GetTotalDuration(const FinishedQueryInfo info) {
    return info.total_execution_duration
         + info.total_report_duration
         + info.total_wait_duration;
}

void FinishedQueryInfo_Free(const FinishedQueryInfo query_info) {
    if (query_info.query_string) {
        free(query_info.query_string);
    }
    if (query_info.graph_name) {
        free(query_info.graph_name);
    }
}

static void _add_finished_query(const FinishedQueryInfo info) {
    ScopedRwlock lock SCOPED_RWLOCK = ScopedRwlock_New(
        &finished_queries_rwlock,
        true);

    ASSERT(lock.is_locked);
    if (unlikely(!lock.is_locked)) {
        return;
    }

    if (unlikely(!finished_queries)) {
        return;
    }

    CircularBufferNRG_Add(finished_queries, (const void*)&info);
}

static bool _lock_rwlock
(
    pthread_rwlock_t *lock,
    const bool is_write
) {
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
    const QueryInfoStorage *storage,
    const uint64_t index
) {
    ASSERT(storage && "Storage has to be provided.");
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

QueryInfoIterator QueryInfoIterator_New(const QueryInfoStorage *storage) {
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

const QueryInfoStorage* QueryInfoIterator_GetStorage
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

Info *Info_New(void) {
	Info *info = rm_malloc(sizeof(Info));

    // HACK: compensate for the main thread
    const uint64_t thread_count = ThreadPools_ThreadCount() + 1;

    info->waiting_queries = QueryInfoStorage_New();
    info->working_queries = QueryInfoStorage_NewWithLength(thread_count);
    _QueryInfoStorage_ResetAll(&info->working_queries);

    _FinishedQueryCounters_Reset(&info->finished_query_counters);

    bool lock_initialized = !pthread_rwlock_init(
        &info->waiting_queries_rwlock,
        NULL);

    REQUIRE_TRUE_OR_RETURN(lock_initialized, false);

    lock_initialized = !pthread_rwlock_init(
        &info->inverse_global_lock,
        NULL);

    REQUIRE_TRUE_OR_RETURN(lock_initialized, false);

    // REQUIRE_TRUE_OR_RETURN(Statistics_New(&info->statistics), false);

    return info;
}

void Info_Reset(Info *info) {
    REQUIRE_ARG(info);

    _FinishedQueryCounters_Reset(&info->finished_query_counters);
    // Statistics_Reset(&info->statistics);
}

bool Info_Free(Info *info) {
    REQUIRE_ARG_OR_RETURN(info, false);

    QueryInfoStorage_Free(&info->waiting_queries);
    QueryInfoStorage_Free(&info->working_queries_per_thread);
    int lock_destroyed = pthread_rwlock_destroy(
        &info->waiting_queries_rwlock);
    ASSERT(lock_destroyed == 0 && "Waiting queries rwlock destroy error.");
    lock_destroyed = pthread_rwlock_destroy(
        &info->inverse_global_lock);
    ASSERT(lock_destroyed == 0 && "Global rwlock destroy error.");

    // Statistics_Free(&info->statistics);

    return lock_destroyed == 0;
}

void Info_AddWaitingQueryInfo
(
    Info *info,
    const struct QueryCtx *query_context,
    const millis_t wait_duration,
    const uint64_t received_unix_timestamp_milliseconds
) {
    REQUIRE_ARG(info);

    REQUIRE_TRUE(_Info_LockEverything(info, true));
    REQUIRE_TRUE(_Info_LockWaitingQueries(info, true));

    QueryInfo query_info = QueryInfo_New();
    QueryInfo_SetQueryContext(&query_info, query_context);
    query_info.wait_duration += wait_duration;
    query_info.received_unix_timestamp_milliseconds = received_unix_timestamp_milliseconds;
    query_info.stage = QueryStage_WAITING;
    QueryInfoStorage_Add(&info->waiting_queries, query_info);

    REQUIRE_TRUE(_Info_UnlockWaitingQueries(info));
    REQUIRE_TRUE(_Info_UnlockEverything(info));
}

static bool _Info_MoveQueryInfoBetweenStorages
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
            QueryInfo_ResetStageTimer(&query_info);
            array_del(storage.queries, i);
            query_info.stage = QueryStage_EXECUTING;
            const bool set = QueryInfoStorage_Set(
                &info->working_queries_per_thread,
                thread_id,
                query_info);
            // Statistics_RecordWaitDuration(&info->statistics, QueryInfo_GetWaitingTime(query_info));
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

    // This effectively moves the query info object from the executing queue
    // to the reporting queue, recording the time spent executing.

    const int thread_id = ThreadPools_GetThreadID();
    QueryInfo *query_info = QueryInfoStorage_Get(
        &info->working_queries_per_thread,
        thread_id);
    ASSERT(query_info && QueryInfo_GetQueryContext(query_info) == context);
    if (!query_info || QueryInfo_GetQueryContext(query_info) != context) {
        REQUIRE_TRUE(_Info_UnlockEverything(info));
        return;
    }
    query_info->stage = QueryStage_REPORTING;
    QueryInfo_UpdateExecutionTime(query_info);
    QueryInfo_ResetStageTimer(query_info);
    // Statistics_RecordExecutionDuration(&info->statistics, QueryInfo_GetExecutionTime(*query_info));
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
        &info->working_queries_per_thread,
        thread_id);
    const bool is_ours
        = query_info && QueryInfo_GetQueryContext(query_info) == context
        && query_info->stage == QueryStage_REPORTING;
    ASSERT(is_ours);
    if (!is_ours) {
        REQUIRE_TRUE(_Info_UnlockEverything(info));
        return;
    }
    query_info->stage = QueryStage_FINISHED;
    QueryInfo_UpdateReportingTime(query_info);
    QueryInfo_ResetStageTimer(query_info);
    // Statistics_RecordReportDuration(&info->statistics, QueryInfo_GetReportingTime(*query_info));
    FinishedQueryInfo finished = FinishedQueryInfo_FromQueryInfo(*query_info);
    const millis_t total_duration = _FinishedQueryInfo_GetTotalDuration(finished);
    // Statistics_RecordTotalDuration(&info->statistics, total_duration);
    _add_finished_query(finished);
    QueryInfoStorage_ResetElement(
        &info->working_queries_per_thread,
        thread_id);
    REQUIRE_TRUE(_Info_UnlockEverything(info));
}

uint64_t Info_GetTotalQueriesCount(Info *info) {
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

uint64_t Info_GetWaitingQueriesCount(Info *info) {
    REQUIRE_ARG_OR_RETURN(info, 0);

    REQUIRE_TRUE_OR_RETURN(_Info_LockWaitingQueries(info, false), 0);

    const uint64_t count = QueryInfoStorage_Length(&info->waiting_queries);

    REQUIRE_TRUE_OR_RETURN(_Info_UnlockWaitingQueries(info), 0);

    return count;
}

uint64_t Info_GetExecutingQueriesCount(Info *info) {
    REQUIRE_ARG_OR_RETURN(info, 0);

    REQUIRE_TRUE_OR_RETURN(_Info_LockEverything(info, true), 0);

    QueryInfoIterator iterator = QueryInfoIterator_New(&info->working_queries_per_thread);
    uint64_t count = 0;

    QueryInfo *query_info = NULL;
    while ((query_info = QueryInfoIterator_NextValid(&iterator)) != NULL) {
        if (query_info->stage == QueryStage_EXECUTING) {
            ++count;
        }
    }

    REQUIRE_TRUE_OR_RETURN(_Info_UnlockEverything(info), 0);

    return count;
}

uint64_t Info_GetReportingQueriesCount(Info *info) {
    REQUIRE_ARG_OR_RETURN(info, 0);

    REQUIRE_TRUE_OR_RETURN(_Info_LockEverything(info, true), 0);

    QueryInfoIterator iterator = QueryInfoIterator_New(&info->working_queries_per_thread);
    uint64_t count = 0;

    QueryInfo *query_info = NULL;
    while ((query_info = QueryInfoIterator_NextValid(&iterator)) != NULL) {
        if (query_info->stage == QueryStage_REPORTING) {
            ++count;
        }
    }

    REQUIRE_TRUE_OR_RETURN(_Info_UnlockEverything(info), 0);

    return count;
}

millis_t Info_GetMaxQueryWaitTime(Info *info) {
    REQUIRE_ARG_OR_RETURN(info, 0);
    _Info_LockEverything(info, false);

    millis_t max_time = 0;
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
        QueryInfoIterator iterator = QueryInfoIterator_New(&info->working_queries_per_thread);
        while ((query = QueryInfoIterator_NextValid(&iterator)) != NULL) {
            max_time = MAX(max_time, QueryInfo_GetWaitingTime(*query));
        }
    }

    _Info_UnlockEverything(info);

    return max_time;
}

void Info_IncrementNumberOfQueries
(
    Info *info,
    const QueryExecutionTypeFlag flags,
    const QueryExecutionStatus status
) {
    REQUIRE_ARG(info);

    _FinishedQueryCounters_Increment(
        &info->finished_query_counters,
        flags,
        status
    );
}

FinishedQueryCounters Info_GetFinishedQueryCounters(const Info info) {
    return info.finished_query_counters;
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

QueryInfoStorage* Info_GetWorkingQueriesStorage(Info *info) {
    REQUIRE_ARG_OR_RETURN(info, NULL);
    return &info->working_queries_per_thread;
}

static void _FinishedQueryInfoDeleter(void *user_data, void *info) {
    UNUSED(user_data);
    ASSERT(info);
    if (info) {
        FinishedQueryInfo *query_info = (FinishedQueryInfo*)info;
        FinishedQueryInfo_Free(*query_info);
    }
}

static void _FinishedQueryInfoClone
(
    const void *item_to_clone,
    void *destination_item,
    void *user_data
) {
    ASSERT(item_to_clone);
    ASSERT(destination_item);
    UNUSED(user_data);

    FinishedQueryInfo *source = (FinishedQueryInfo*)item_to_clone;
    FinishedQueryInfo *destination = (FinishedQueryInfo*)destination_item;

    *destination = *source;

    if (destination->query_string) {
        destination->query_string = strdup(destination->query_string);
    }

    if (destination->graph_name) {
        destination->graph_name = strdup(destination->graph_name);
    }
}

void Info_SetCapacityForFinishedQueriesStorage(const uint32_t count) {
    ScopedRwlock lock SCOPED_RWLOCK = ScopedRwlock_New(
        &finished_queries_rwlock,
        true
    );

    ASSERT(lock.is_locked);
    if (!lock.is_locked) {
        return;
    }

    if (!finished_queries) {
        finished_queries = CircularBufferNRG_New(sizeof(FinishedQueryInfo), count);
        CircularBufferNRG_SetDeleter(
            finished_queries,
            _FinishedQueryInfoDeleter,
            NULL);
        CircularBufferNRG_SetItemClone(
            finished_queries,
            _FinishedQueryInfoClone,
            NULL
        );
    } else {
        CircularBufferNRG_SetCapacity(&finished_queries, count);
    }
}

void Info_ViewAllFinishedQueries
(
    CircularBufferNRG_ReadAllCallback callback,
    void *user_data
) {
    ASSERT(finished_queries);
    if (!finished_queries) {
        return;
    }

    ScopedRwlock lock SCOPED_RWLOCK = ScopedRwlock_New(
        &finished_queries_rwlock,
        false);

    ASSERT(lock.is_locked);
    if (!lock.is_locked) {
        return;
    }

    CircularBufferNRG_ViewAll(finished_queries, callback, user_data);
}
