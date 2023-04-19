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
void QueryInfoDeleter(void *user_data, void *info);
QueryInfo *QueryInfo_Clone(QueryInfo *qi);
void QueryInfo_CloneTo(const void *item_to_clone, void *destination_item,
    void *user_data);

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

static void _add_finished_query
(
	const QueryInfo *qi
) {
    int res = pthread_rwlock_wrlock(&finished_queries_rwlock);
	ASSERT(res == 0);

    CircularBuffer_AddForce(finished_queries, (const void*)qi);

    res = pthread_rwlock_unlock(&finished_queries_rwlock);
	ASSERT(res == 0);
}

static bool _Info_LockEverything
(
	Info *info
) {
	ASSERT(info != NULL);

	return !pthread_mutex_lock(&info->mutex);
}

static bool _Info_UnlockEverything
(
	Info *info
) {
    ASSERT(info != NULL);
    return !pthread_mutex_unlock(&info->mutex);
}

// fake hash function
// hash of key is simply key
static uint64_t _nop_hash
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
    static dictType dt = { _nop_hash, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL};
    info->waiting_queries = HashTableCreate(&dt);

    // initialize working_queries array
    info->working_queries = rm_calloc(thread_count, sizeof(QueryInfo*));

    _FinishedQueryCounters_Reset(&info->counters);

    int res = pthread_mutex_init(&info->mutex, NULL);
    ASSERT(res == 0);

    return info;
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

// return the number of queries being currently executed
// requires a pointer to mutable, for it changes the state of the locks
uint64_t Info_GetExecutingQueriesCount
(
	Info *info
) {
    ASSERT(info != NULL);

    bool res = _Info_LockEverything(info);
	ASSERT(res == true);

    uint64_t count = 0;
    uint n = ThreadPools_ThreadCount() + 1;

	for(uint i = 0; i < n; i++) {
		QueryInfo *qi = info->working_queries[i];
        if(qi != NULL && qi->stage == QueryStage_EXECUTING) {
            count++;
        }
	}

	res = _Info_UnlockEverything(info);
	ASSERT(res == true);

    return count;
}

uint64_t Info_GetReportingQueriesCount(Info *info) {
    ASSERT(info != NULL);

    bool res = _Info_LockEverything(info);
    ASSERT(res == true);

    uint64_t count = 0;
    uint n = ThreadPools_ThreadCount() + 1;

	for(uint i = 0; i < n; i++) {
		QueryInfo *qi = info->working_queries[i];
        if(qi != NULL && qi->stage == QueryStage_REPORTING) {
            count++;
        }
	}

    res = _Info_UnlockEverything(info);
    ASSERT(res == true);

    return count;
}

// return the total number of queries currently queued or being executed
uint64_t Info_GetTotalQueriesCount
(
	Info *info
) {
    ASSERT(info != NULL);

	const uint64_t waiting   = Info_GetWaitingQueriesCount(info);
	const uint64_t executing = Info_GetExecutingQueriesCount(info);
	const uint64_t reporting = Info_GetReportingQueriesCount(info);

    return waiting + executing + reporting;
}

// return the maximum registered time a query was spent waiting
// taking into account all currently waiting, executing and reporting queries
millis_t Info_GetMaxQueryWaitTime
(
    Info *info
) {
    ASSERT(info != NULL);

	uint n = ThreadPools_ThreadCount() + 1;
	QueryInfo *qi = NULL;
	millis_t max_time = 0;

    bool res = _Info_LockEverything(info);
    ASSERT(res == true);

    dictIterator *it = HashTableGetIterator(info->waiting_queries);
    while((qi = (QueryInfo*)HashTableNext(it)) != NULL) {
		QueryInfo_UpdateWaitingTime(qi);
        max_time = MAX(max_time, QueryInfo_GetWaitingTime(qi));
    }

    HashTableReleaseIterator(it);

	for(uint i = 0; i < n; i++) {
		qi = info->working_queries[i];
		if(qi != NULL) {
			max_time = MAX(max_time, QueryInfo_GetWaitingTime(qi));
		}
	}

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

// locks the info object for external reading. Only one concurrent read is
// allowed at the same time
bool Info_Lock
(
    Info *info
) {
    return _Info_LockEverything(info);
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
    QueryInfoStorage *storage  // result container
) {
    QueryInfoStorage st = *storage;

    _Info_LockEverything(info);

    // get the number of queries to traverse and copy (waiting or executing)
    uint n_queries = (stage == QueryStage_WAITING ?
        HashTableElemCount(info->waiting_queries) :
        ThreadPools_ThreadCount() + 1);

    //--------------------------------------------------------------------------
    // waiting queries
    //--------------------------------------------------------------------------
    QueryInfo *qi;
    dictIterator *it = HashTableGetIterator(info->waiting_queries);
    while((qi = (QueryInfo*)HashTableNext(it)) != NULL) {
		array_append(st, QueryInfo_Clone(qi));
    }

    //--------------------------------------------------------------------------
    // executing queries
    //--------------------------------------------------------------------------
    for(uint i = 0; i < n_queries; i++) {
        // append a clone of the current query to st
        if(info->working_queries[i] != NULL) {
            array_append(st, QueryInfo_Clone(info->working_queries[i]));
        }
    }

    _Info_UnlockEverything(info);
}

// views the circular buffer of finished queries
void Info_ViewAllFinishedQueries
(
    CircularBufferNRG_ReadAllCallback callback,
    void *user_data
) {
    ASSERT(finished_queries);
    if (!finished_queries) {
        return;
    }

    int res = pthread_rwlock_rdlock(&finished_queries_rwlock);
    ASSERT(res == 0);

	int n = CircularBuffer_ItemCount(finished_queries);
	QueryInfo *queries = rm_malloc(sizeof(QueryInfo*) * n);

	CircularBuffer_ReadAll(finished_queries, queries, n);

    CircularBufferNRG_ViewAll(finished_queries, callback, user_data);

	rm_free(queries);
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

