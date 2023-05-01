/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

/*
 * This file contains the useful statistics and generic information about a
 * a graph. This information is used by the "GRAPH.INFO" command.
*/


#include "RG.h"
#include "info.h"
#include "util/arr.h"
#include "util/num.h"
#include "../util/dict.h"
#include "../query_ctx.h"
#include "util/thpool/pools.h"
#include "util/circular_buffer.h"
// #include "hdr/hdr_histogram.h"

#include <string.h>
#include <sys/types.h>

#define INITIAL_QUERY_INFO_CAPACITY 100

static CircularBuffer finished_queries;
static pthread_rwlock_t finished_queries_rwlock = PTHREAD_RWLOCK_INITIALIZER;

// forward declaration
QueryInfo *QueryInfo_Clone(QueryInfo *qi);
void QueryInfo_CloneTo(const void *item_to_clone, void *destination_item,
    void *user_data);
void QueryInfo_Deleter(void *info);
typedef struct ViewFinishedQueriesCallbackData ViewFinishedQueriesCallbackData;

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
    ASSERT(lhs != NULL);

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

// increments the corresponding query type counter
// the passed parameters define the kind of query and its finish status
static void _FinishedQueryCounters_Increment
(
    FinishedQueryCounters *counters,
    const QueryExecutionTypeFlag flags,
    const QueryExecutionStatus status
) {
    ASSERT(counters != NULL);

	// write query
    if (CHECK_FLAG(flags, QueryExecutionTypeFlag_WRITE)) {
		// write query encountered error
        if (status == QueryExecutionStatus_FAILURE) {
            ++counters->write_failed_n;
		// write query timedout
        } else if (status == QueryExecutionStatus_TIMEDOUT) {
            ++counters->write_timedout_n;
        } else {
		// write query succeeded
            ++counters->write_succeeded_n;
        }
		return;
    } else {
		// read query
		// read query encountered error
        if (status == QueryExecutionStatus_FAILURE) {
            ++counters->ro_failed_n;
		// read query timedout
        } else if (status == QueryExecutionStatus_TIMEDOUT) {
            ++counters->ro_timedout_n;
        } else {
			// read query succeeded
            ++counters->ro_succeeded_n;
        }
		return;
    }

    ASSERT(false && "Handle unknown flag.");
}

static bool _Info_LockWaiting
(
    bool write
) {
    if(write) {
        return !pthread_rwlock_wrlock(&finished_queries_rwlock);
    } else {
        return !pthread_rwlock_rdlock(&finished_queries_rwlock);
    }
}

static bool _Info_UnlockWaiting(void) {
    return !pthread_rwlock_unlock(&finished_queries_rwlock);
}

static bool _Info_LockEverything
(
	Info *info
) {
	ASSERT(info != NULL);

    int res = _Info_LockWaiting(true);
    ASSERT(res == 1);
    UNUSED(res);

    return !pthread_mutex_lock(&info->mutex);
}

static bool _Info_UnlockEverything
(
	Info *info
) {
    ASSERT(info != NULL);

    int res = _Info_UnlockWaiting();
    ASSERT(res == 1);

    return !pthread_mutex_unlock(&info->mutex);
}

static void _add_finished_query
(
	QueryInfo *qi
) {
    int res = _Info_LockWaiting(true);
	ASSERT(res == 1);

    CircularBuffer_AddForce(finished_queries, (void *)&qi);

    res = _Info_UnlockWaiting();
	ASSERT(res == 1);
}

Info *Info_New(void) {
    // HACK: compensate for the main thread
    const uint64_t thread_count = ThreadPools_ThreadCount() + 1;

	Info *info = rm_malloc(sizeof(Info));

    // initialize hashmap for the waiting queries (keys are QueryInfo *)
    info->waiting_queries = HashTableCreate(&def_dt);

    // initialize working_queries array
    info->working_queries = rm_calloc(thread_count, sizeof(QueryInfo*));

    _FinishedQueryCounters_Reset(&info->counters);

    int res = pthread_mutex_init(&info->mutex, NULL);
    ASSERT(res == 0);

    return info;
}

// create the finished queries storage (circular buffer)
void Info_SetFinishedQueriesStorage
(
    const uint n  // size of circular buffer
) {
    finished_queries = CircularBuffer_New(sizeof(QueryInfo *), n,
        QueryInfo_Deleter);
}

// add a query to the waiting list for the first time (from dispatcher)
// at this stage, no time has been previously accumulated
void Info_AddToWaiting
(
    Info *info,    // info
    QueryInfo *qi  // query info of the query starting to wait
) {
    // start the stage-timer
    TIMER_RESTART(qi->stage_timer);

    // set the stage to waiting
    qi->stage = QueryStage_WAITING;

    // acquire mutex
    bool res = _Info_LockEverything(info);
    ASSERT((void *)res != NULL);

    // add the query to the waiting dict
    HashTableAdd(info->waiting_queries, qi, qi);

    // release mutex
    res = _Info_UnlockEverything(info);
    ASSERT((void *)res != NULL);
}

// remove a query from the waiting_queries, insert it to the executing queue,
// and set its stage
void Info_IndicateQueryStartedExecution
(
    Info *info,    // info
    QueryInfo *qi  // query info that is starting the execution stage
) {
    ASSERT(qi != NULL);
    ASSERT(info != NULL);

    const int tid = ThreadPools_GetThreadID();

    // update waiting time
    QueryInfo_UpdateWaitingTime(qi);

    bool res = _Info_LockEverything(info);
	ASSERT(res == true);

	//--------------------------------------------------------------------------
	// remove query info from waiting-list
	//--------------------------------------------------------------------------

    // remove from waiting_queries
    int dict_res = HashTableDelete(info->waiting_queries, (void *)qi);
	ASSERT(dict_res == DICT_OK);

	// release mutex
    res = _Info_UnlockEverything(info);
	ASSERT(res == true);

    // set the stage
    qi->stage = QueryStage_EXECUTING;

	//--------------------------------------------------------------------------
	// add query info to executing list
	//--------------------------------------------------------------------------

    // add to working queries array
	info->working_queries[tid] = qi;
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

    // remove from working queries array
    const int tid = ThreadPools_GetThreadID();

	// clear entry
    info->working_queries[tid] = NULL;

    // start the stage-timer
    TIMER_RESTART(qi->stage_timer);

    // set the stage to waiting
    qi->stage = QueryStage_WAITING;

    // acquire mutex
    bool res = _Info_LockEverything(info);
    ASSERT(res == true);

    // add the query to the waiting dict
    HashTableAdd(info->waiting_queries, qi, qi);

    // release mutex
    res = _Info_UnlockEverything(info);
    ASSERT(res == true);
}

// indicates that the query has finished the execution and has started
// reporting the results back to the client
void Info_IndicateQueryStartedReporting
(
    Info *info  // info
) {
    ASSERT(info != NULL);

    const int tid = ThreadPools_GetThreadID();
    QueryInfo *qi = info->working_queries[tid];
	ASSERT(qi != NULL);

    qi->stage = QueryStage_REPORTING;
    QueryInfo_UpdateExecutionTime(qi);
}

// indicates that the query has finished reporting the results and is no longer
// required to be stored and kept track of
void Info_IndicateQueryFinishedReporting
(
    Info *info
) {
    ASSERT(info != NULL);

	//--------------------------------------------------------------------------
	// remove query info from working queries
	//--------------------------------------------------------------------------

    const int tid = ThreadPools_GetThreadID();
    QueryInfo *qi = info->working_queries[tid];
	ASSERT(qi != NULL);

    info->working_queries[tid] = NULL;
	ASSERT(qi->stage == QueryStage_REPORTING);

	// update stage to finished and add to finished buffer
    qi->stage = QueryStage_FINISHED;
    QueryInfo_UpdateReportingTime(qi);

    _add_finished_query(qi);
}

// indicates that the query has finished due to an error
void Info_IndicateQueryFinishedAfterError
(
    Info *info
) {
    ASSERT(info != NULL);

	//--------------------------------------------------------------------------
	// remove query info from working queries
	//--------------------------------------------------------------------------

    const int tid = ThreadPools_GetThreadID();
    QueryInfo *qi = info->working_queries[tid];
	ASSERT(qi != NULL);

    info->working_queries[tid] = NULL;

	// update stage to finished and add to finished buffer
    qi->stage = QueryStage_FINISHED;

    _add_finished_query(qi);
}

// return the number of queries currently waiting to be executed
// requires a pointer to mutable, for it changes the state of the locks
uint64_t Info_GetWaitingQueriesCount
(
	Info *info
) {
    ASSERT(info != NULL);

    bool res = _Info_LockEverything(info);
	ASSERT(res == true);

    const uint64_t count = HashTableElemCount(info->waiting_queries);

    res = _Info_UnlockEverything(info);
	ASSERT(res == true);

    return count;
}

// count the amount of executing and reporting queries
void Info_GetExecutingReportingQueriesCount
(
    Info *info,           // info
    uint64_t *executing,  // [OUTPUT] amount of executing queries
    uint64_t *reporting   // [OUTPUT] amount of reporting queries
) {
    ASSERT(info != NULL);

    uint n = ThreadPools_ThreadCount() + 1;

    ASSERT(res == true);

	for(uint i = 0; i < n; i++) {
		QueryInfo *qi = info->working_queries[i];
        if(qi != NULL) {
            if(qi->stage == QueryStage_REPORTING) {
                (*reporting)++;
            } else if(qi->stage == QueryStage_EXECUTING) {
                (*executing)++;
            }
        }
	}

    ASSERT(res == true);
}

// return the total number of queries currently queued or being executed
uint64_t Info_GetTotalQueriesCount
(
	Info *info
) {
    ASSERT(info != NULL);

    GraphContext *gc = QueryCtx_GetGraphCtx();

    uint64_t executing = 0;
    uint64_t reporting = 0;
    uint64_t waiting = Info_GetWaitingQueriesCount(gc->info);
    Info_GetExecutingReportingQueriesCount(gc->info, &executing, &reporting);

    return waiting + executing + reporting;
}

// return the maximum registered time a query spent waiting
millis_t Info_GetMaxQueryWaitTime
(
    Info *info
) {
    ASSERT(info != NULL);

	QueryInfo *qi = NULL;
	millis_t max_time = 0;

	// TODO: lock waiting
    bool res = _Info_LockEverything(info);
    ASSERT(res == true);

    dictIterator *it = HashTableGetIterator(info->waiting_queries);
    while((qi = (QueryInfo*)HashTableNext(it)) != NULL) {
		QueryInfo_UpdateWaitingTime(qi);
		// TODO: consider introducing if condition...
        max_time = MAX(max_time, QueryInfo_GetWaitingTime(qi));
    }

    HashTableReleaseIterator(it);

    res = _Info_UnlockEverything(info);
    ASSERT(res == true);

    return max_time;
}

// increments the corresponding query type counter
// the passed parameters define the kind of query and its finish status
void Info_IncrementNumberOfQueries
(
    Info *info,
    const QueryExecutionTypeFlag flags,
    const QueryExecutionStatus status
) {
    ASSERT(info != NULL);

    _FinishedQueryCounters_Increment(&info->counters, flags, status);
}

// unlocks the info object from exclusive external reading
bool Info_Unlock(Info *info) {
    return _Info_UnlockEverything(info);
}

dict* Info_GetWaitingQueries(Info *info) {
    ASSERT(info != NULL);
    return info->waiting_queries;
}

// stores clones of queries of a certain state among the waiting and the
// executing stages in storage
void Info_GetQueries
(
    Info *info,                 // info
    QueryStage stage,           // wanted stage
    QueryInfoStorage *storage   // result container
) {
    // QueryInfoStorage st = *storage;
    bool waiting = (stage == QueryStage_WAITING);

    bool res = _Info_LockEverything(info);
    ASSERT(res == true);

    // get the number of queries to traverse and copy (waiting or executing)
    uint n_queries = (waiting ? HashTableElemCount(info->waiting_queries) :
                                ThreadPools_ThreadCount() + 1);

    if(waiting) {
        //--------------------------------------------------------------------------
        // waiting queries
        //--------------------------------------------------------------------------
        QueryInfo *qi;
        dictIterator *it = HashTableGetIterator(info->waiting_queries);
        while((qi = (QueryInfo*)HashTableNext(it)) != NULL) {
            array_append(*storage, QueryInfo_Clone(qi));
        }
    } else {
        //--------------------------------------------------------------------------
        // executing queries
        //--------------------------------------------------------------------------
        for(uint i = 0; i < n_queries; i++) {
            // append a clone of the current query to st
            if(info->working_queries[i] != NULL) {
                array_append(*storage, QueryInfo_Clone(info->working_queries[i]));
            }
        }
    }

    res = _Info_UnlockEverything(info);
    ASSERT(res == true);
}

// views the circular buffer of finished queries
void Info_ViewFinishedQueries
(
    CircularBuffer_ReadCallback callback,  // callback
    void *user_data,                       // additional data for callback
    uint n_items                           // number of items to view
) {
    ASSERT(finished_queries);
    if (!finished_queries) {
        return;
    }

    int res = _Info_LockWaiting(false);
    ASSERT(res == 1);

	// read items from the circular buffer, calling callback on each one
    CircularBuffer_TraverseCBFromLast(finished_queries, n_items, callback,
        user_data);

    res = _Info_UnlockWaiting();
    ASSERT(res == 1);
}

void Info_Free
(
	Info *info
) {
    ASSERT(info != NULL);

    rm_free(info->working_queries);

    HashTableRelease(info->waiting_queries);

    int res = pthread_mutex_destroy(&info->mutex);
    ASSERT(res == 0);

	rm_free(info);
}

