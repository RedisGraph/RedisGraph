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
#include "RG.h"
#include "util/num.h"
#include <sys/types.h>
#include "../query_ctx.h"
// #include "hdr/hdr_histogram.h"
#include "util/thpool/pools.h"
#include "util/circular_buffer_nrg.h"

#define INITIAL_QUERY_INFO_CAPACITY 100

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
        counters->write_failed_n     +
        counters->write_timedout_n   +
        counters->ro_failed_n        +
        counters->write_succeeded_n  +
        counters->ro_timedout_n      +
        counters->ro_succeeded_n ;
}

void FinishedQueryCounters_Add
(
    FinishedQueryCounters *lhs,
    const FinishedQueryCounters rhs
) {
    REQUIRE_ARG(lhs);

    lhs->ro_failed_n += rhs.ro_failed_n;
    lhs->ro_failed_n += rhs.ro_failed_n;
    lhs->ro_timedout_n += rhs.ro_timedout_n;
    lhs->write_failed_n += rhs.write_failed_n;
    lhs->write_succeeded_n += rhs.write_succeeded_n;
    lhs->write_timedout_n += rhs.write_timedout_n;
}

static void _FinishedQueryCounters_Reset
(
    FinishedQueryCounters *counters
) {
    ASSERT(counters != NULL);

	counters->write_failed_n     = 0;
	counters->write_timedout_n   = 0;
	counters->ro_failed_n        = 0;
	counters->write_succeeded_n  = 0;
	counters->ro_timedout_n      = 0;
	counters->ro_succeeded_n     = 0;
}

static void _FinishedQueryCounters_Increment(
    FinishedQueryCounters *counters,
    const QueryExecutionTypeFlag flags,
    const QueryExecutionStatus status
) {
    REQUIRE_ARG(counters);

    if (CHECK_FLAG(flags, QueryExecutionTypeFlag_WRITE)) {
        if (status == QueryExecutionStatus_FAILURE) {
            ++counters->write_failed_n;
        } else if (status == QueryExecutionStatus_TIMEDOUT) {
            ++counters->write_timedout_n;
        } else {
            ++counters->write_succeeded_n;
        }

        return;
    } else {
        if (status == QueryExecutionStatus_FAILURE) {
            ++counters->ro_failed_n;
        } else if (status == QueryExecutionStatus_TIMEDOUT) {
            ++counters->ro_timedout_n;
        } else {
            ++counters->ro_succeeded_n;
        }

        return;
    }

    ASSERT(false && "Handle unknown flag.");
}

FinishedQueryInfo FinishedQueryInfo_FromQueryInfo(const QueryInfo info) {
    // ASSERT(info.context);        context removed. Update accordingly.

    FinishedQueryInfo finished;
    memset(&finished, 0, sizeof(FinishedQueryInfo));

    finished.received_ts = info.received_ts;
    finished.wait_duration = info.wait_duration;
    finished.execution_duration = info.execution_duration;
    finished.report_duration = info.report_duration;

    // context removed from QueryInfo, update this accordingly
    // if (info.context) {
    //     finished.query_string = strdup(info.context->query_data.query);
    //     finished.graph_name = strdup(info.context->gc->graph_name);
    // }

    return finished;
}

millis_t _FinishedQueryInfo_GetTotalDuration(const FinishedQueryInfo info) {
    return info.execution_duration
         + info.report_duration
         + info.wait_duration;
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
    ASSERT(lock != NULL);

    return !pthread_rwlock_unlock(lock);
}

QueryInfoIterator QueryInfoIterator_NewStartingAt
(
    const QueryInfoStorage *storage,
    const uint64_t index
) {
    ASSERT(storage && "Storage has to be provided.");
    const uint64_t length = array_len(storage);
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
    ASSERT(iterator != NULL);

    const uint64_t length = array_len(iterator->storage);
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

    while (next && !next->ctx) {
        next = QueryInfoIterator_Next(iterator);
    }

    return next;
}

const QueryInfoStorage* QueryInfoIterator_GetStorage
(
    QueryInfoIterator *iterator
) {
    REQUIRE_ARG_OR_RETURN(iterator, NULL);
    ASSERT(iterator != NULL);

    return iterator->storage;
}

QueryInfo* QueryInfoIterator_Get(QueryInfoIterator *iterator) {
    ASSERT(iterator != NULL);
    iterator->has_started = true;
    return array_elem(*iterator->storage, iterator->current_index);
}

uint32_t QueryInfoIterator_Length(const QueryInfoIterator *iterator) {
    ASSERT(iterator != NULL);
    return array_len(iterator->storage) - iterator->current_index;
}

bool QueryInfoIterator_IsExhausted(const QueryInfoIterator *iterator) {
    return QueryInfoIterator_Length(iterator) > 0;
}

static bool _Info_LockEverything(Info *info, const bool is_write) {
	ASSERT(info     != NULL);
    ASSERT(iterator != NULL);
    return _lock_rwlock(&info->mutex, !is_write);
}

static bool _Info_UnlockEverything(Info *info) {
    REQUIRE_ARG_OR_RETURN(info, false);
    return _unlock_rwlock(&info->mutex);
}

static bool _Info_LockWaitingQueries(Info *info, const bool is_write) {
    REQUIRE_ARG_OR_RETURN(info, false);
    return _lock_rwlock(&info->waiting_queries_rwlock, is_write);
}

static bool _Info_UnlockWaitingQueries(Info *info) {
    REQUIRE_ARG_OR_RETURN(info, false);
    return _unlock_rwlock(&info->waiting_queries_rwlock);
}

static void _Info_WLockWaitingQueries
(
	Info *info
) {
    ASSERT(info != NULL);

	int res = pthread_rwlock_wrlock(&info->waiting_queries_rwlock);
	ASSERT(res == 0);
}

static void _InfoRLockWaitingQueries
(
	Info *info
) {
    ASSERT(info != NULL);

	int res = pthread_rwlock_rdlock(&info->waiting_queries_rwlock);
	ASSERT(res == 0);
}

static void _Info_UnlockWaitingQueries
(
	Info *info
) {
    ASSERT(info != NULL);
	// TODO: assert lock is acquired
    _unlock_rwlock(&info->waiting_queries_rwlock);
}

// fake hash function
// hash of key is simply key
static uint64_t nop_hash
(
	const void *key
) {
	return ((uint64_t)key);
}

Info *Info_New(void) {
    // HACK: compensate for the main thread
    const uint64_t thread_count = ThreadPools_ThreadCount() + 1;

	Info *info = rm_malloc(sizeof(Info));

    // initialize hashmap for the waiting queries (keys are QueryInfo *)
    static dictType dt = { nop_hash, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL};
    info->waiting_queries = HashTableCreate(&dt);

    info->waiting_queries = array_new(QueryInfo*, 128);
    info->working_queries = array_newlen(QueryInfo*, thread_count);

    _FinishedQueryCounters_Reset(&info->counters);
	memset(info->working_queries, 0, sizeof(QueryInfo) * thread_count);

    int res = pthread_rwlock_init(&info->waiting_queries_rwlock, NULL);
    ASSERT(res == 0);

    res = pthread_rwlock_init(&info->mutex, NULL);
    ASSERT(res == 0);

    return info;
}

// remove a query from the waiting_queries, insert it to the executing queue,
// and set its stage
void Info_IndicateQueryStartedExecution
(
    Info *info,    // info
    QueryInfo *qi  // query info that is starting the execution stage
) {
    ASSERT(ctx  != NULL);
    ASSERT(info != NULL);

    ASSERT(_Info_LockEverything(info, true));
    ASSERT(_Info_LockWaitingQueries(info, true));

    // update waiting time
    QueryInfo_UpdateWaitingTime(qi);
    QueryInfo_ResetStageTimer(qi);

    // remove from waiting_queries (TODO: add case that the query isn't found --> error)
    HashTableDelete(info->waiting_queries, (void *)qi);

    // set the stage
    qi->stage = QueryStage_EXECUTING;

    const int thread_id = ThreadPools_GetThreadID();

    // add to working queries array
    QueryInfo *working_q_info = array_elem(info->working_queries, thread_id);
    memcpy(working_q_info, &qi, sizeof(QueryInfo));

    ASSERT(_Info_UnlockWaitingQueries(info) != NULL);
    ASSERT(_Info_UnlockEverything(info) != NULL);
}


void Info_Free
(
	Info *info
) {
    ASSERT(info != NULL);

    array_free(&info->working_queries);

    HashTableRelease(info->waiting_queries);

    int res = pthread_rwlock_destroy(&info->waiting_queries_rwlock);
    ASSERT(res == 0);

    res = pthread_rwlock_destroy(&info->mutex);
    ASSERT(res == 0);
}

static bool _Info_MoveQueryInfoBetweenStorages
(
    QueryInfoStorage *from,
    QueryInfoStorage *to,
    const uint64_t index
) {
    REQUIRE_ARG_OR_RETURN(from, false);
    REQUIRE_ARG_OR_RETURN(to, false);
    QueryInfo *from_element = array_elem(*from, index);
    QueryInfo *info = array_elem(*to, index);
    memcpy(info, from_element, sizeof(QueryInfo));
    memset(array_elem(*from, index), 0, sizeof(QueryInfo));

    return true;
}

// transitions a query from executing to waiting
// the queryInfo in the index corresponding to the thread_id will be removed
// from the `working_queries` array of the Info struct.
void Info_executing_to_waiting
(
    Info *info,     // info
    QueryInfo *qi   // query info
) {
    // update the elapsed time in executing state, and restart timer
    qi->execution_duration += simple_toc(qi->stage_timer);
    TIMER_RESTART(qi->stage_timer);

    // change state
    qi->stage = QueryStage_WAITING;

    // remove from working queries array
    const int thread_id = ThreadPools_GetThreadID();
    QueryInfo *info = array_elem(info->working_queries, thread_id);
    memset(info, 0, sizeof(QueryInfo));

    // add qi to the waiting queries hashmap
    hashtableAdd(info->waiting_queries, qi);
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
    QueryInfo *query_info = array_elem(
        info->working_queries,
        thread_id);
    ASSERT(query_info && QueryInfo_GetQueryContext(query_info) == context);
    if (!query_info || QueryInfo_GetQueryContext(query_info) != context) {
        REQUIRE_TRUE(_Info_UnlockEverything(info));
        return;
    }
    query_info->stage = QueryStage_REPORTING;
    QueryInfo_UpdateExecutionTime(query_info);
    QueryInfo_ResetStageTimer(query_info);
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
    QueryInfo *query_info = array_elem(
        info->working_queries,
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
    memset(array_elem(info->working_queries, thread_id), 0, sizeof(QueryInfo));
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
    ASSERT(info != NULL);

    ASSERT(_Info_LockWaitingQueries(info, false) != NULL);

    const uint64_t count = array_len(&info->waiting_queries);

    ASSERT(_Info_UnlockWaitingQueries(info) != NULL);

    return count;
}

uint64_t Info_GetExecutingQueriesCount(Info *info) {
    REQUIRE_ARG_OR_RETURN(info, 0);

    REQUIRE_TRUE_OR_RETURN(_Info_LockEverything(info, true), 0);

    QueryInfoIterator iterator = QueryInfoIterator_New(&info->working_queries);
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

    QueryInfoIterator iterator = QueryInfoIterator_New(&info->working_queries);
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
        dictIterator *it = HashTableGetIterator(info->waiting_queries);
        dictEntry *entry;
		while((entry  = HashTableNext(it)) != NULL) {
            max_time = MAX(max_time, QueryInfo_GetWaitingTime((const QueryInfo *)entry));
        }
        ASSERT(_Info_UnlockWaitingQueries(info) != NULL);
        HashTableReleaseIterator(it);
    }
    {
        QueryInfoIterator iterator = QueryInfoIterator_New(&info->working_queries);
        while ((query = QueryInfoIterator_NextValid(&iterator)) != NULL) {
            max_time = MAX(max_time, QueryInfo_GetWaitingTime((const QueryInfo *)query));
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
        &info->counters,
        flags,
        status
    );
}

FinishedQueryCounters Info_GetFinishedQueryCounters(const Info info) {
    return info.counters;
}

bool Info_Lock(Info *info) {
    return _Info_LockEverything(info, false);
}

bool Info_Unlock(Info *info) {
    return _Info_UnlockEverything(info);
}

dict* Info_GetWaitingQueries(Info *info) {
    ASSERT(info != NULL);
    return info->waiting_queries;
}

QueryInfoStorage* Info_GetWorkingQueriesStorage(Info *info) {
    ASSERT(info != NULL);
    return &info->working_queries;
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
