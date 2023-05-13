/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "cron/cron.h"
#include "redismodule.h"
#include "graph/graphcontext.h"
#include "configuration/config.h"
#include "util/circular_buffer.h"
#include "stream_finished_queries.h"

// TODO: thread-safe access to 'graphs_in_keyspace'
extern GraphContext **graphs_in_keyspace;

// event fields count
#define FLD_COUNT 9

// field names
#define FLD_NAME_STAGE               "Stage"
#define FLD_NAME_QUERY               "Query"
#define FLD_NAME_UTILIZED_CACHE      "Utilized cache"
#define FLD_NAME_GRAPH_NAME          "Graph name"
#define FLD_NAME_WAIT_DURATION       "Wait duration"
#define FLD_NAME_TOTAL_DURATION      "Total duration"
#define FLD_NAME_RECEIVED_TIMESTAMP  "Received at"
#define FLD_NAME_REPORT_DURATION     "Report duration"
#define FLD_NAME_EXECUTION_DURATION  "Execution duration"

// event field:value pairs
static RedisModuleString *_event[FLD_COUNT * 2] = {0};

// initialize '_event' template
// this function should be called only once
static void _initEventTemplate
(
	RedisModuleCtx *ctx  // redis module context
) {
	ASSERT(ctx != NULL);
	ASSERT(_event[0] == NULL);

	//--------------------------------------------------------------------------
	// create field names
	//--------------------------------------------------------------------------

	_event[0] = RedisModule_CreateString(
					ctx,
					FLD_NAME_RECEIVED_TIMESTAMP,
					strlen(FLD_NAME_RECEIVED_TIMESTAMP)
				);

	_event[2]  = RedisModule_CreateString(
					ctx,
					FLD_NAME_STAGE,
					strlen(FLD_NAME_STAGE)
				 );

	_event[4]  = RedisModule_CreateString(
					ctx,
					FLD_NAME_GRAPH_NAME,
					strlen(FLD_NAME_GRAPH_NAME)
				 );

	_event[6]  = RedisModule_CreateString(
					ctx,
					FLD_NAME_QUERY,
					strlen(FLD_NAME_QUERY)
				 );

	_event[8]  = RedisModule_CreateString(
					ctx,
					FLD_NAME_TOTAL_DURATION,
					strlen(FLD_NAME_TOTAL_DURATION)
				 );

	_event[10] = RedisModule_CreateString(
					ctx,
					FLD_NAME_WAIT_DURATION,
					strlen(FLD_NAME_WAIT_DURATION)
				 );

	_event[12] = RedisModule_CreateString(
					ctx,
					FLD_NAME_EXECUTION_DURATION,
					strlen(FLD_NAME_EXECUTION_DURATION)
				 );

	_event[14] = RedisModule_CreateString(
					ctx,
					FLD_NAME_REPORT_DURATION,
					strlen(FLD_NAME_REPORT_DURATION)
				 );

	_event[16] = RedisModule_CreateString(
					ctx,
					FLD_NAME_UTILIZED_CACHE,
					strlen(FLD_NAME_UTILIZED_CACHE)
				 );
}

// populate event
// sets event values
static void _populateEvent
(
	RedisModuleCtx *ctx,     // redis module context
	const QueryInfo *qi ,    // query information
	const char *graph_name,  // graph name
	size_t graph_name_len    // length of graph name
) {
	const millis_t total_spent_time = qi->wait_duration      +
									  qi->execution_duration +
									  qi->report_duration;

	// FLD_NAME_RECEIVED_TIMESTAMP
	_event[1]  = RedisModule_CreateStringFromLongLong(
					ctx,
					qi->received_ts
				 );

	// FLD_NAME_STAGE
	_event[3]  = RedisModule_CreateStringFromLongLong(
					ctx,
					qi->stage
				 );

	// FLD_NAME_GRAPH_NAME
	_event[5]  = RedisModule_CreateString(
					ctx,
					graph_name,
					graph_name_len
				 );

	// FLD_NAME_QUERY
	_event[7]  = RedisModule_CreateString(
					ctx,
					qi->query_string,
					strlen(qi->query_string)
				 );

	// FLD_NAME_TOTAL_DURATION
	_event[9]  = RedisModule_CreateStringFromLongLong(
					ctx,
					total_spent_time
				 );

	// FLD_NAME_WAIT_DURATION
	_event[11] = RedisModule_CreateStringFromLongLong(
					ctx,
					qi->wait_duration
				 );

	// FLD_NAME_EXECUTION_DURATION
	_event[13] = RedisModule_CreateStringFromLongLong(
					ctx,
					qi->execution_duration
				 );

	// FLD_NAME_REPORT_DURATION
	_event[15] = RedisModule_CreateStringFromLongLong(
					ctx,
					qi->report_duration
				 );

	// FLD_NAME_UTILIZED_CACHE
	_event[17] = RedisModule_CreateStringFromLongLong(
					ctx,
					qi->utilized_cache
				 );
}

// free event values
static void _clearEvent
(
	RedisModuleCtx *ctx  // redis module context
) {
	if(unlikely(_event[1] == NULL)) return;

	for(int i = 1; i < FLD_COUNT * 2; i += 2) {
		RedisModule_FreeString(ctx, _event[i]);
	}
}

// add queries to stream
static void _stream_queries
(
	RedisModuleCtx *ctx,      // redis module context
	RedisModuleKey *key,      // stream key
	const char *graph_name,   // graph name
	CircularBuffer queries    // queries to stream
) {
	size_t graph_name_len = strlen(graph_name);

	QueryInfo *q;
	while(CircularBuffer_Remove(queries, &q) == true) {
		_populateEvent(ctx, q, graph_name, graph_name_len);

		RedisModule_StreamAdd(key, REDISMODULE_STREAM_ADD_AUTOID, NULL,
				_event, FLD_COUNT);

		// clean up
		_clearEvent(ctx);
		QueryInfo_Free(q);
	}
}

// cron task
// stream finished queries for each graph in the keyspace
// task is alowed to run for 1ms before it terminates and reschedules
void CronTask_streamFinishedQueries
(
	void *pdata  // task context
) {
	StreamFinishedQueryCtx *ctx    = (StreamFinishedQueryCtx*)pdata;
	RedisModuleCtx         *rm_ctx = RedisModule_GetThreadSafeContext(NULL);

	// initialize stream event template
	if(unlikely(_event[0] == NULL)) {
		_initEventTemplate(rm_ctx);
	}

	// reset timer
	simple_tic(ctx->timer);

	// determine max number of queries to collect
	uint32_t max_query_count = 0;
	Config_Option_get(Config_CMD_INFO_MAX_QUERY_COUNT, &max_query_count);

	uint32_t n            = array_len(graphs_in_keyspace);  // #graphs
	double   window       = 1;                              // work window 1ms

	// pick up from where we've left
	// for each graph in the keyspace
	for(; ctx->graph_idx < n; ctx->graph_idx++) {
		// TODO: access to graphs_in_keyspace must be protected!
		GraphContext *gc = graphs_in_keyspace[ctx->graph_idx];

		// increase graph context reference count
		GraphContext_IncreaseRefCount(gc);

		Info *info = gc->info;

		// skip graph if there are no new finished queries
		if(Info_GetFinishedCount(info) == 0) {
			GraphContext_DecreaseRefCount(gc);
			continue;
		}

		//----------------------------------------------------------------------
		// collect queries
		//----------------------------------------------------------------------

		CircularBuffer queries = Info_ResetFinishedQueries(info);
		CircularBuffer_ResetReader(queries, CircularBuffer_ItemCount(queries));

		//----------------------------------------------------------------------
		// open stream
		//----------------------------------------------------------------------

		const char *graph_name = GraphContext_GetName(gc);
		uint graph_name_len = strlen(graph_name);
		RedisModuleString* keyname = RedisModule_CreateStringPrintf(rm_ctx,
				"telematics{%s}", graph_name);

		// acquire GIL
		RedisModule_ThreadSafeContextLock(rm_ctx);

		RedisModuleKey *key = RedisModule_OpenKey(rm_ctx, keyname,
				REDISMODULE_WRITE);

		// make sure key is of type stream
		if(RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_STREAM) {
			// TODO: decide how to handle this...	
		}

		// add queries to stream, free individual queries
		_stream_queries(rm_ctx, key, graph_name, queries);

		// cap stream
		RedisModule_StreamTrimByLength(key, REDISMODULE_STREAM_TRIM_APPROX,
				max_query_count);

		// clean up
		RedisModule_CloseKey(key);

		// release GIL
		RedisModule_ThreadSafeContextUnlock(rm_ctx);

		CircularBuffer_Free(queries);
		RedisModule_FreeString(rm_ctx, keyname);

		// decrease graph context reference count
		GraphContext_DecreaseRefCount(gc);

		// determine how much time we've spent
		// exit if we're out of time
		double elapsed = simple_toc(ctx->timer);
		if(elapsed > window) {
			ctx->graph_idx++;
			break;
		}
	}

	RedisModule_FreeThreadSafeContext(rm_ctx);

	//--------------------------------------------------------------------------
	// determine next invocation
	//--------------------------------------------------------------------------

	// task ran out of time and there's additional graph to process
	bool speedup = (ctx->graph_idx < n);

	if(speedup) {
		// reduce delay, hard limit 10ms
		ctx->when = MAX(10, ctx->when - 1);
	} else {
		// increase delay, hard limit 100ms
		ctx->when = MIN(100, ctx->when + 1);
	}

	// wrap around
	ctx->graph_idx = ctx->graph_idx % n;

	// re-add task to CRON
	Cron_AddTask(ctx->when, CronTask_streamFinishedQueries, ctx);
}

