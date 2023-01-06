/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */
/*
 * This file contains all the necessary data structures for the "graph.info"
 * command to operate. In order to show the statistics, it is required to store
 * and accumulate the information about different queries, their stages and the
 * time those spend in these stages. To tackle multithreading issues, either
 * atomic variables are used, or read-write locks, or the knowledge of
 * the thread ids is used to directly navigate to a certain element in linear
 * time and in a lock-free manner.
*/

#pragma once

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

#include <pthread.h>

#include "../util/simple_timer.h"

typedef struct hdr_histogram hdr_histogram;

// Checks whether the specified flag is set within the flag variable.
// Evaluates to true if set, to false otherwise.
#define CHECK_FLAG(flag_var, flag_value) \
    ((flag_var & flag_value) == flag_value)

typedef uint32_t millis_t;
typedef struct QueryCtx QueryCtx;
// Duplicate typedef from the circular buffer.
typedef bool (*CircularBufferNRG_ReadAllCallback)(void *user_data, const void *item);

// Specifies what kind of query should we count.
typedef enum QueryStatisticsFlag {
    QueryStatisticsFlag_READONLY = 0,
    QueryStatisticsFlag_WRITE = 1 << 0,
    QueryStatisticsFlag_FAIL = 1 << 1,
    QueryStatisticsFlag_TIMEOUT = 1 << 2,
} QueryStatisticsFlag;

// Holds the necessary per-query statistics.
typedef struct QueryInfo {
    // The UNIX epoch timestamp in milliseconds indicating when the query was
    // received by the module.
    uint64_t received_unix_timestamp_milliseconds;
    // The time it spent waiting.
    millis_t wait_duration;
    // The time spent on executing.
    millis_t execution_duration;
    // The time spent on reporting.
    millis_t report_duration;
    // The context of the query.
    const QueryCtx *context;
    // A timer for counting the time spent in the stages (waiting, executing,
    // reporting).
    simple_timer_t stage_timer;
} QueryInfo;

typedef struct FinishedQueryInfo {
    uint64_t received_unix_timestamp_milliseconds;
    millis_t total_wait_duration;
    millis_t total_execution_duration;
    millis_t total_report_duration;
    char *query_string;
    char *graph_name;
} FinishedQueryInfo;

typedef struct FinishedQueryCounters {
    // The number of read-only queries succeeded.
    atomic_uint_fast64_t readonly_succeeded_count;
    // The number of write queries succeeded.
    atomic_uint_fast64_t write_succeeded_count;
    // The number of read-only queries failed but not timed out.
    atomic_uint_fast64_t readonly_failed_count;
    // The number of write queries failed but not timed out.
    atomic_uint_fast64_t write_failed_count;
    // The number of read-only queries timed out.
    atomic_uint_fast64_t readonly_timedout_count;
    // The number of write queries timed out.
    atomic_uint_fast64_t write_timedout_count;
} FinishedQueryCounters;

// Returns the total number of queries recorded.
uint64_t FinishedQueryCounters_GetTotalCount(const FinishedQueryCounters);

FinishedQueryInfo FinishedQueryInfo_FromQueryInfo(const QueryInfo info);
void FinishedQueryInfo_Free(const FinishedQueryInfo query_info);

// Creates a new, empty query info object.
QueryInfo QueryInfo_New(void);
// Assigns the query context to the query info.
void QueryInfo_SetQueryContext(QueryInfo *, const QueryCtx *);
// Returns the query context associated with the query info.
const QueryCtx* QueryInfo_GetQueryContext(const QueryInfo *);
// Returns true if the query info object is valid and can be worked with.
bool QueryInfo_IsValid(const QueryInfo *);
// Returns the date/time when the query was received by the module, in
// milliseconds from UNIX epoch.
uint64_t QueryInfo_GetReceivedTimestamp(const QueryInfo);
// Returns the total time spent by a query waiting, executing and reporting.
millis_t QueryInfo_GetTotalTimeSpent(const QueryInfo, bool *is_ok);
// Returns the time the query spent waiting.
millis_t QueryInfo_GetWaitingTime(const QueryInfo);
// Returns the time the query spent executing.
millis_t QueryInfo_GetExecutionTime(const QueryInfo);
// Returns the time the query spent reporting.
millis_t QueryInfo_GetReportingTime(const QueryInfo);
// Reads the stage timer and updates the waiting time with it.
void QueryInfo_UpdateWaitingTime(QueryInfo *info);
// Reads the stage timer and updates the execution time with it.
void QueryInfo_UpdateExecutionTime(QueryInfo *info);
// Reads the stage timer and updates the reporting time with it.
void QueryInfo_UpdateReportingTime(QueryInfo *info);
// Resets the stage timer and returns the milliseconds it had recorded before
// having been reset.
millis_t QueryInfo_ResetStageTimer(QueryInfo *);

typedef struct QueryInfoStorage {
    QueryInfo *queries;
} QueryInfoStorage;

// Creates a new query info storage with the specified capacity.
QueryInfoStorage QueryInfoStorage_NewWithCapacity(const uint64_t capacity);
// Creates a new query info storage with default capacity.
QueryInfoStorage QueryInfoStorage_New();
// Clears the storage by removing all the queries stored.
void QueryInfoStorage_Clear(QueryInfoStorage *);
// Deallocates the storage.
void QueryInfoStorage_Free(QueryInfoStorage *);
// Returns the number of elements which are valid within the storage.
uint32_t QueryInfoStorage_ValidCount(const QueryInfoStorage *);
// Returns the current length of the storage (in elements).
uint32_t QueryInfoStorage_Length(const QueryInfoStorage *);
// Adds a query info object.
void QueryInfoStorage_Add(QueryInfoStorage *, const QueryInfo);
// Sets the capacity of the storage to the specified one. If there were more
// elements than the specified capacity, those are removed (cut off).
void QueryInfoStorage_SetCapacity(QueryInfoStorage *, const uint32_t);
// Sets the value of an existing element within the array.
bool QueryInfoStorage_Set(QueryInfoStorage *, const uint64_t, const QueryInfo);
// Returns a pointer to an already existing value in the storage at provided
// index.
QueryInfo* QueryInfoStorage_Get(const QueryInfoStorage *, const uint64_t);
// Resets the element value in the index.
bool QueryInfoStorage_ResetElement(QueryInfoStorage *, const uint64_t);
// Returns true if the element has successfully been removed.
bool QueryInfoStorage_Remove(QueryInfoStorage *, const QueryInfo *);
// Returns true if the element has successfully been removed.
bool QueryInfoStorage_RemoveByContext(QueryInfoStorage *, const QueryCtx *);

// An abstraction for iteration over QueryInfo objects.
typedef struct QueryInfoIterator {
    const QueryInfoStorage *storage;
    uint64_t current_index;
    // True if at least one Next*/Get* was called.
    bool has_started;
} QueryInfoIterator;

// Returns a new iterator over the provided storage starting with the
// provided index. The created iterator doesn't have to be freed.
QueryInfoIterator QueryInfoIterator_NewStartingAt
(
    const QueryInfoStorage *,
    const uint64_t
);
// Returns a new iterator over the provided storage starting from the beginning.
QueryInfoIterator QueryInfoIterator_New(const QueryInfoStorage *);
// Returns a pointer to the next element from the current, NULL if the end has
// been reached.
QueryInfo* QueryInfoIterator_Next(QueryInfoIterator *);
// Returns a pointer to the next valid element from the current, NULL if the end
// has been reached.
QueryInfo* QueryInfoIterator_NextValid(QueryInfoIterator *);
// Returns a pointer to the underlying storage being iterated over.
const QueryInfoStorage* QueryInfoIterator_GetStorage(QueryInfoIterator *);
// Returns current element being pointed at by the iterator.
QueryInfo* QueryInfoIterator_Get(QueryInfoIterator *);
// Returns the number of elements which are still to be iterated over.
uint32_t QueryInfoIterator_Length(const QueryInfoIterator *);
// Returns true if the iterator has no more elements to iterate over.
bool QueryInfoIterator_IsExhausted(const QueryInfoIterator *);

typedef struct Info {
    // Storage for query information for waiting queries.
    QueryInfoStorage waiting_queries;
    // Synchronisation primitive to use when doing anything with the
    // waiting_queries collection, as it is supposed to be touched concurrently.
    pthread_rwlock_t waiting_queries_rwlock;
    // Storage for query information for queries being executed.
    QueryInfoStorage executing_queries_per_thread;
    // Storage for query information for reporting currently queries.
    QueryInfoStorage reporting_queries_per_thread;
    // Maximum registered time a query was spent waiting, executing and
    // reporting the results.
    atomic_uint_fast64_t max_query_pipeline_time;
    // Finished query counters with states.
    FinishedQueryCounters finish_query_counters;
    // Statistics for the wait durations in milliseconds.
    hdr_histogram *wait_durations;
    // Statistics for the execution durations in milliseconds.
    hdr_histogram *execution_durations;
    // Statistics for the report durations in milliseconds.
    hdr_histogram *report_durations;
    // A global lock for the object. Used as an inverse lock - allows parallel
    // writers but just one reader. This is done that way as the parallel
    // writes are guaranteed to happen lock-free or without race conditions,
    // so the locking for writes isn't really required. But what is required is
    // locking for reads, and to allow that, and rwlock is used inversely.
    pthread_rwlock_t inverse_global_lock;
} Info;

// Create a new info structure. Returns true on successful creation.
bool Info_New(Info *);
// Reset an already existing info structure.
void Info_Reset(Info *);
// Free the info structure's contents. Returns true on successful free.
bool Info_Free(Info *);
// Insert a query information into the info structure. The query is supposed
// to be added right after being successfully parsed and known to the module,
// and before it starts being executed.
void Info_AddWaitingQueryInfo
(
    Info *,
    const QueryCtx *,
    const millis_t wait_duration,
    const uint64_t received_unix_timestamp_milliseconds
);
// Indicates that the provided query has finished waiting and stated being
// executed.
void Info_IndicateQueryStartedExecution
(
    Info *,
    const QueryCtx *
);
// Indicates that the query has finished the execution and has started
// reporting the results back to the client.
void Info_IndicateQueryStartedReporting
(
    Info *,
    const QueryCtx *
);
// Indicates that the query has finished reporting the results and is no longer
// required to be stored and kept track of.
void Info_IndicateQueryFinishedReporting
(
    Info *,
    const QueryCtx *
);
// Return the total number of queries currently queued or being executed.
// Requires a pointer to mutable, for it changes the state of the locks.
uint64_t Info_GetTotalQueriesCount(Info *);
// Return the number of queries currently waiting to be executed.
// Requires a pointer to mutable, for it changes the state of the locks.
uint64_t Info_GetWaitingQueriesCount(Info *);
// Return the number of queries being currently executed.
// Requires a pointer to mutable, for it changes the state of the locks.
uint64_t Info_GetExecutingQueriesCount(Info *);
// Return the number of queries currently reporting results back.
// Requires a pointer to mutable, for it changes the state of the locks.
uint64_t Info_GetReportingQueriesCount(Info *);
// Return the maximum registered time a query was spent waiting, executing and
// reporting the results.
// Requires a pointer to mutable, for it changes the state of the locks.
millis_t Info_GetMaxQueryWaitTime(Info *);
// Incremenents the corresponding query type counter. The passed flag
// defines the kind of query and its finish status.
void Info_IncrementNumberOfQueries(Info *, const QueryStatisticsFlag);
// Returns the finished query counters from the passed info object.
FinishedQueryCounters Info_GetFinishedQueryCounters(const Info);
// Locks the info object for external reading. Only one concurrent read is
// allowed at the same time.
bool Info_Lock(Info *);
// Unlocks the info object from exclusive external reading.
bool Info_Unlock(Info *);
// Returns a pointer to the underlying storage for all the waiting queries.
// Must be accessed within the Info_Lock and Info_Unlock.
QueryInfoStorage* Info_GetWaitingQueriesStorage(Info *info);
// Returns a pointer to the underlying reporting queries storage per thread.
// Must be accessed within the Info_Lock and Info_Unlock.
QueryInfoStorage* Info_GetExecutingQueriesStorage(Info *info);
// Returns a pointer to the underlying reporting queries storage per thread.
// Must be accessed within the Info_Lock and Info_Unlock.
QueryInfoStorage* Info_GetReportingQueriesStorage(Info *info);
// Resizes the finished queries storage.
void Info_SetCapacityForFinishedQueriesStorage(const uint32_t count);
// Views the circular buffer of finished queries.
void Info_ViewAllFinishedQueries
(
    CircularBufferNRG_ReadAllCallback callback,
    void *user_data
);
