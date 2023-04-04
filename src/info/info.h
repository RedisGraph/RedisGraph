/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

// this file contains all the necessary data structures for the "graph.info"
// command to operate
// in order to show the statistics, it is required to store
// and accumulate the information about different queries, their stages and the
// time those spend in these stages
// to tackle multithreading issues, either atomic variables are used,
// or read-write locks, or the knowledge of the thread ids is used to directly
// navigate to a certain element in linear time and in a lock-free manner

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdatomic.h>

#include "query_info.h"
#include "../util/num.h"
#include "../util/dict.h"
#include "../util/simple_timer.h"

typedef enum QueryExecutionStatus QueryExecutionStatus;
typedef enum QueryExecutionTypeFlag QueryExecutionTypeFlag;

#define MILLIS_T_MAX UINT32_MAX
typedef struct QueryCtx QueryCtx;
// duplicate typedef from the circular buffer
typedef bool (*CircularBufferNRG_ReadAllCallback)(void *user_data,
                                                  const void *item);
typedef QueryInfo* QueryInfoStorage;

// an abstraction for iteration over QueryInfo objects
typedef struct QueryInfoIterator {
    const QueryInfoStorage storage;
    uint64_t current_index;
    bool has_started; // true if at least one Next*/Get* was called
} QueryInfoIterator;

// holds a successfull query info
typedef struct FinishedQueryInfo {
    uint64_t received_ts;         // query received timestamp
    millis_t wait_duration;       // waiting time
    millis_t execution_duration;  // executing time
    millis_t report_duration;     // reporting time
    char *query_string;           // query string
    char *graph_name;             // graph name
} FinishedQueryInfo;

// holds query statistics per graph
typedef struct FinishedQueryCounters {
    atomic_uint_fast64_t ro_succeeded_n;     // # read-only queries succeeded
    atomic_uint_fast64_t write_succeeded_n;  // # write queries succeeded
    atomic_uint_fast64_t ro_failed_n;        // # RO queries failed
    atomic_uint_fast64_t write_failed_n;     // # write queries failed
    atomic_uint_fast64_t ro_timedout_n;      // # RO queries timed out
    atomic_uint_fast64_t write_timedout_n;   // # write queries timed out
} FinishedQueryCounters;

// information about a graph
typedef struct Info {
    dict *waiting_queries;                     // waiting queries
    QueryInfoStorage working_queries;         // executing and reporting queries
    atomic_uint_fast64_t max_query_time;      // slowest query time
    FinishedQueryCounters counters;           // counters with states
    pthread_mutex_t mutex;                    // info lock
} Info;

// create a new info structure
// returns true on successful creation
Info *Info_New(void);

// add a query to the waiting list for the first time (from dispatcher)
// at this stage, no time has been previously accumulated
void Info_AddToWaiting
(
    Info *info,    // info
    QueryInfo *qi  // query info of the query starting to wait
);

// remove a query from the waiting_queries, insert it to the executing queue,
// and set its stage
void Info_IndicateQueryStartedExecution
(
    Info *info,    // info
    QueryInfo *qi  // query info that is starting the execution stage
);

// transitions a query from executing to waiting
// the queryInfo in the index corresponding to the thread_id will be removed
// from the `working_queries` array of the Info struct.
void Info_executing_to_waiting
(
    Info *info,     // info
    QueryInfo *qi   // query info
);

// indicates that the query has finished the execution and has started
// reporting the results back to the client
void Info_IndicateQueryStartedReporting
(
    Info *info  // info
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
// Incremenents the corresponding query type counter. The passed parameters
// define the kind of query and its finish status.
void Info_IncrementNumberOfQueries(Info *, const QueryExecutionTypeFlag, const QueryExecutionStatus);
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
// Returns a pointer to the underlying working queries storage per thread.
// Must be accessed within the Info_Lock and Info_Unlock.
QueryInfoStorage* Info_GetWorkingQueriesStorage(Info *info);
// Resizes the finished queries storage.
void Info_SetCapacityForFinishedQueriesStorage(const uint32_t count);
// Views the circular buffer of finished queries.
void Info_ViewAllFinishedQueries
(
    CircularBufferNRG_ReadAllCallback callback,
    void *user_data
);
// returns the total number of queries recorded
uint64_t FinishedQueryCounters_GetTotalCount
(
	const FinishedQueryCounters *counters  // counters to sum up
);

// adds the counts from another counters object
void FinishedQueryCounters_Add
(
    FinishedQueryCounters *lhs,
    const FinishedQueryCounters rhs
);

FinishedQueryInfo FinishedQueryInfo_FromQueryInfo(const QueryInfo info);
void FinishedQueryInfo_Free(const FinishedQueryInfo query_info);

// Returns a new iterator over the provided storage starting with the
// provided index. The created iterator doesn't have to be freed.
QueryInfoIterator QueryInfoIterator_NewStartingAt
(
    const QueryInfoStorage,
    const uint64_t
);
// Returns a new iterator over the provided storage starting from the beginning.
QueryInfoIterator QueryInfoIterator_New(const QueryInfoStorage);
// Returns a pointer to the next element from the current, NULL if the end has
// been reached.
QueryInfo* QueryInfoIterator_Next(QueryInfoIterator *);
// Returns a pointer to the next valid element from the current, NULL if the end
// has been reached.
QueryInfo* QueryInfoIterator_NextValid(QueryInfoIterator *);
// Returns a pointer to the underlying storage being iterated over.
const QueryInfoStorage QueryInfoIterator_GetStorage(QueryInfoIterator *);
// Returns current element being pointed at by the iterator.
QueryInfo* QueryInfoIterator_Get(QueryInfoIterator *);
// Returns the number of elements which are still to be iterated over.
uint32_t QueryInfoIterator_Length(const QueryInfoIterator *);
// Returns true if the iterator has no more elements to iterate over.
bool QueryInfoIterator_IsExhausted(const QueryInfoIterator *);

// free the info structure's content
void Info_Free
(
	Info *info
);

