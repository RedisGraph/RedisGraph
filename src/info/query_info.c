/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

// creates a new, empty query info object
QueryInfo *QueryInfo_New(void) {
    QueryInfo *query_info = rm_malloc(sizeof(QueryInfo));

	query_info->context            = NULL;
	query_info->stage_timer        = {}
	query_info->received_ts        = 0;
	query_info->wait_duration      = 0;
	query_info->report_duration    = 0;
	query_info->execution_duration = 0;

    TIMER_RESTART(query_info->stage_timer);

    return query_info;
}

// assigns the query context to the query info
void QueryInfo_SetQueryContext
(
    QueryInfo *qi,
    const struct QueryCtx *ctx
) {
    ASSERT(qi != NULL);
    ASSERT(ctx != NULL);

    query_info->context = ctx;
}

// returns the query context associated with the query info
const QueryCtx* QueryInfo_GetQueryContext
(
	const QueryInfo *qi
) {
    ASSERT(qi != NULL);

    return qi->context;
}

// returns true if the query info object is valid and can be worked with
bool QueryInfo_IsValid
(
	const QueryInfo *qi
) {
	ASSERT(qi != NULL);

    return QueryInfo_GetQueryContext(qi) != NULL;
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

// returns the time the query spent waiting
millis_t QueryInfo_GetWaitingTime
(
	const QueryInfo *qi
) {
	ASSERT(qi != NULL);

    return qi->execution_duration;
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
void QueryInfo_UpdateWaitingTime
(
	QueryInfo *qi
) {
    ASSERT(qi != NULL);

    qi->wait_duration = QueryInfo_GetCountedMilliseconds(qi);
}

// reads the stage timer and updates the execution time with it.
void QueryInfo_UpdateExecutionTime
(
	QueryInfo *qi
) {
    ASSERT(qi);

    info->execution_duration = QueryInfo_GetCountedMilliseconds(info);
}

// reads the stage timer and updates the reporting time with it
void QueryInfo_UpdateReportingTime
(
	QueryInfo *qi
) {
    ASSERT(qi != NULL);

    qi->report_duration = QueryInfo_GetCountedMilliseconds(qi);
}

// returns the number of milliseconds the timer has counted
// this function does not reset the timer
millis_t QueryInfo_GetCountedMilliseconds
(
	QueryInfo *qi
) {
    ASSERT(qi != NULL);

    return (millis_t)TIMER_GET_ELAPSED_MILLISECONDS(qi->stage_timer);
}

// resets the stage timer and returns the milliseconds it had recorded before
// having been reset
millis_t QueryInfo_ResetStageTimer
(
	QueryInfo *qi
) {
    ASSERT(qi != NULL);

    const millis_t milliseconds_spent
        = (millis_t)TIMER_GET_ELAPSED_MILLISECONDS(qi->stage_timer);

    TIMER_RESTART(qi->stage_timer);

    return milliseconds_spent;
}

