/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include <sys/types.h>

#include "query_info.h"
#include "../util/simple_timer.h"
#include "../graph/graphcontext.h"

// resets the stage timer and returns the milliseconds it had recorded before
// having been reset
static void QueryInfo_ResetTimer
(
	QueryInfo *qi
) {
    ASSERT(qi != NULL);

    TIMER_RESTART(qi->timer);
}

// returns the number of milliseconds the timer has counted
// this function reset the timer
static millis_t QueryInfo_GetCountedMilliseconds
(
	QueryInfo *qi
) {
    ASSERT(qi != NULL);

    millis_t ms = TIMER_GET_ELAPSED_MILLISECONDS(qi->timer);
    QueryInfo_ResetTimer(qi);
	return ms;
}

// creates a new, empty query info object
QueryInfo *QueryInfo_New(void) {
    QueryInfo *qi = rm_calloc(1, sizeof(QueryInfo));
    return qi;
}

// advance query's stage
// waiting   -> executing
// executing -> reporting
// reporting -> finished
void QueryInfo_AdvanceStage
(
	QueryInfo *qi  // query info
) {
	ASSERT(qi != NULL);
	qi->stage = qi->stage << 1;
}

// returns the date/time when the query was received by the module
// in milliseconds from UNIX epoch
uint64_t QueryInfo_GetReceivedTimestamp
(
	const QueryInfo *qi
) {
	ASSERT(qi != NULL);

    return qi->received_ts;
}

// returns the total time spent by a query waiting
// executing and reporting
millis_t QueryInfo_GetTotalTimeSpent
(
	const QueryInfo *qi
) {
	ASSERT(qi != NULL);

    return qi->wait_duration + qi->execution_duration + qi->report_duration;
}

// returns the time the query spent waiting
millis_t QueryInfo_GetWaitingTime
(
	const QueryInfo *qi
) {
	ASSERT(qi != NULL);

    return qi->wait_duration;
}

// returns the time the query spent executing
millis_t QueryInfo_GetExecutionTime
(
	const QueryInfo *qi
) {
	ASSERT(qi != NULL);
	
	return qi->execution_duration;
}

// returns the time the query spent reporting.
millis_t QueryInfo_GetReportingTime
(
	const QueryInfo *qi
) {
	ASSERT(qi != NULL);

    return qi->report_duration;
}

// reads the stage timer and updates the waiting time with it
millis_t QueryInfo_UpdateWaitingTime
(
	QueryInfo *qi
) {
    ASSERT(qi != NULL);

    qi->wait_duration += QueryInfo_GetCountedMilliseconds(qi);
	return qi->wait_duration;
}

// reads the stage timer and updates the execution time with it.
millis_t QueryInfo_UpdateExecutionTime
(
	QueryInfo *qi
) {
    ASSERT(qi);

    qi->execution_duration += QueryInfo_GetCountedMilliseconds(qi);
	return qi->execution_duration;
}

// reads the stage timer and updates the reporting time with it
millis_t QueryInfo_UpdateReportingTime
(
	QueryInfo *qi
) {
    ASSERT(qi != NULL);

    qi->report_duration += QueryInfo_GetCountedMilliseconds(qi);
	return qi->report_duration;
}

// sets the "utilized_cache" flag of a QueryInfo
void QueryInfo_SetUtilizedCache
(
    QueryInfo *qi,  // query info
    bool utilized   // cache utilized
) {
    qi->utilized_cache = utilized;
}

// clone a QueryInfo
QueryInfo *QueryInfo_Clone
(
    QueryInfo *qi
) {
    QueryInfo *clone = rm_calloc(1, sizeof(QueryInfo));
    if(qi->graph_name != NULL) {
        clone->graph_name = strdup(qi->graph_name);
    }
    if(qi->query_string != NULL) {
        clone->query_string = strdup(qi->query_string);
    }
    clone->stage = qi->stage;
    clone->received_ts = qi->received_ts;
    clone->wait_duration = qi->wait_duration;
    clone->utilized_cache = qi->utilized_cache;
    clone->report_duration = qi->report_duration;
    clone->execution_duration = qi->execution_duration;
    // clone timer will isn't relevant, stays {0}

    return clone;
}

// free a QueryInfo
void QueryInfo_Free
(
    QueryInfo *qi
) {
	ASSERT(qi != NULL);

    if (qi->query_string != NULL) {
        free(qi->query_string);
    }
    if (qi->graph_name != NULL) {
        free(qi->graph_name);
    }
}

