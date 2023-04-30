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
#include <sys/types.h>

#include "query_info.h"
#include "../util/num.h"
#include "../util/dict.h"
#include "../util/simple_timer.h"
#include "../util/circular_buffer.h"

typedef enum QueryExecutionStatus QueryExecutionStatus;
typedef enum QueryExecutionTypeFlag QueryExecutionTypeFlag;

#define MILLIS_T_MAX UINT32_MAX
typedef struct QueryCtx QueryCtx;
// duplicate typedef from the circular buffer
typedef void (*CircularBuffer_ReadCallback)(const void *item, void *user_data);

typedef QueryInfo** QueryInfoStorage;

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

// create the finished queries storage (circular buffer)
void Info_SetFinishedQueriesStorage
(
    const uint n  // size of circular buffer
);

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

// indicates that the query has finished reporting the results and is no longer
// required to be stored and kept track of
void Info_IndicateQueryFinishedReporting
(
    Info *info
);

// indicates that the query has finished due to an error
void Info_IndicateQueryFinishedAfterError
(
    Info *info
);

// return the number of queries currently waiting to be executed
// requires a pointer to mutable, for it changes the state of the locks
uint64_t Info_GetWaitingQueriesCount
(
	Info *info
);

// count the amount of executing and reporting queries
void Info_GetExecutingReportingQueriesCount
(
    Info *info,           // info
    uint64_t *executing,  // [OUTPUT] amount of executing queries
    uint64_t *reporting   // [OUTPUT] amount of reporting queries
);

// return the total number of queries currently queued or being executed
uint64_t Info_GetTotalQueriesCount
(
	Info *info
);

// return the maximum registered time a query was spent waiting
// taking into account all currently waiting, executing and reporting queries
millis_t Info_GetMaxQueryWaitTime
(
    Info *info
);

// increments the corresponding query type counter
// the passed parameters define the kind of query and its finish status
void Info_IncrementNumberOfQueries
(
    Info *info,
    const QueryExecutionTypeFlag,
    const QueryExecutionStatus
);

// Unlocks the info object from exclusive external reading.
bool Info_Unlock
(
    Info *info
);

// stores clones of queries of a certain state among the waiting and the
// executing stages in storage
void Info_GetQueries
(
    Info *info,                 // info
    QueryStage stage,           // wanted stage
    QueryInfoStorage *storage  // result container
);

// views the circular buffer of finished queries
void Info_ViewFinishedQueries
(
    CircularBuffer_ReadCallback callback,  // callback
    void *user_data,                       // additional data for callback
    uint n_items                           // number of items to view
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

// free the info structure's content
void Info_Free
(
	Info *info
);

