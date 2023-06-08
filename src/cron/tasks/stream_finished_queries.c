/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "globals.h"
#include "cron/cron.h"
#include "redismodule.h"
#include "graph/graphcontext.h"
#include "configuration/config.h"
#include "util/circular_buffer.h"
#include "stream_finished_queries.h"

// event fields count
#define FLD_COUNT 7

// field names
#define FLD_NAME_QUERY               "Query"
#define FLD_NAME_UTILIZED_CACHE      "Utilized cache"
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
					FLD_NAME_QUERY,
					strlen(FLD_NAME_QUERY)
				 );

	_event[4]  = RedisModule_CreateString(
					ctx,
					FLD_NAME_TOTAL_DURATION,
					strlen(FLD_NAME_TOTAL_DURATION)
				 );

	_event[6] = RedisModule_CreateString(
					ctx,
					FLD_NAME_WAIT_DURATION,
					strlen(FLD_NAME_WAIT_DURATION)
				 );

	_event[8] = RedisModule_CreateString(
					ctx,
					FLD_NAME_EXECUTION_DURATION,
					strlen(FLD_NAME_EXECUTION_DURATION)
				 );

	_event[10] = RedisModule_CreateString(
					ctx,
					FLD_NAME_REPORT_DURATION,
					strlen(FLD_NAME_REPORT_DURATION)
				 );

	_event[12] = RedisModule_CreateString(
					ctx,
					FLD_NAME_UTILIZED_CACHE,
					strlen(FLD_NAME_UTILIZED_CACHE)
				 );
}

// populate event
// sets event values
static void _populateEvent
(
	RedisModuleCtx *ctx,  // redis module context
	const LoggedQuery *q  // query information
) {
	int l = 0;
	char buff[512] = {0};

	const double total_duration = q->wait_duration        +
									q->execution_duration +
									q->report_duration;

	// FLD_NAME_RECEIVED_TIMESTAMP
	_event[1] = RedisModule_CreateStringFromLongLong(ctx, q->received);

	// FLD_NAME_QUERY
	_event[3] = RedisModule_CreateString(ctx, q->query, strlen(q->query));

	// FLD_NAME_TOTAL_DURATION
	l = sprintf(buff, "%.6f", total_duration);
	_event[5] = RedisModule_CreateString(ctx, buff, l);

	// FLD_NAME_WAIT_DURATION
	l = sprintf(buff, "%.6f", q->wait_duration);
	_event[7] = RedisModule_CreateString(ctx, buff, l);

	// FLD_NAME_EXECUTION_DURATION
	l = sprintf(buff, "%.6f", q->execution_duration);
	_event[9] = RedisModule_CreateString(ctx, buff, l);

	// FLD_NAME_REPORT_DURATION
	l = sprintf(buff, "%.6f", q->report_duration);
	_event[11] = RedisModule_CreateString(ctx, buff, l);

	// FLD_NAME_UTILIZED_CACHE
	_event[13] = RedisModule_CreateStringFromLongLong(ctx, q->utilized_cache);
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
	CircularBuffer queries    // queries to stream
) {
	LoggedQuery *q = NULL;

	// reset reader
	CircularBuffer_ResetReader(queries, CircularBuffer_ItemCount(queries));

	while((q = CircularBuffer_Read(queries, NULL)) != NULL) {
		_populateEvent(ctx, q);

		RedisModule_StreamAdd(key, REDISMODULE_STREAM_ADD_AUTOID, NULL,
				_event, FLD_COUNT);

		// clean up
		_clearEvent(ctx);
	}
}

// cron task
// stream finished queries for each graph in the keyspace
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

	// start stopwatch
	double deadline = 3;  // 3ms
	simple_tic(ctx->stopwatch);

	uint32_t max_query_count = 0;  // determine max number of queries to collect
	Config_Option_get(Config_CMD_INFO_MAX_QUERY_COUNT, &max_query_count);

	GraphIterator it;
	Globals_ScanGraphs(&it);

	// pick up from where we've left
	GraphIterator_Seek(&it, ctx->graph_idx);

	GraphContext *gc = NULL;

	// as long as we've got processing time
	while(TIMER_GET_ELAPSED_MILLISECONDS(ctx->stopwatch) < deadline) {
		ctx->graph_idx++;  // prepare next iteration

		// pull iterator
		gc = GraphIterator_Next(&it);

		// iterator depleted
		if((gc) == NULL) {
			break;
		}

		// get graph's queries log
		QueriesLog queries_log = gc->queries_log;

		// process logged queries
		if(QueriesLog_GetQueriesCount(queries_log) > 0) {

			//------------------------------------------------------------------
			// try to acquire GIL
			//------------------------------------------------------------------

			uint8_t attempts  = 8;  // max number of attempts to acquire the GIL
			bool gil_acquired = false;

			while(attempts > 0 && !gil_acquired) {
				attempts--;
				gil_acquired =
					RedisModule_ThreadSafeContextTryLock(rm_ctx) == REDISMODULE_OK;
			}

			if(gil_acquired == true) {
				CircularBuffer queries = QueriesLog_ResetQueries(queries_log);

				//--------------------------------------------------------------
				// stream queries
				//--------------------------------------------------------------

				RedisModuleString *keyname =
					(RedisModuleString*) GraphContext_GetTelematicsStreamName(gc);

				RedisModuleKey *key = RedisModule_OpenKey(rm_ctx, keyname,
						REDISMODULE_WRITE);

				// make sure key is of type stream
				int key_type = RedisModule_KeyType(key);
				if(key_type == REDISMODULE_KEYTYPE_STREAM ||
				   key_type == REDISMODULE_KEYTYPE_EMPTY) {
					// add queries to stream
					_stream_queries(rm_ctx, key, queries);

					// cap stream
					RedisModule_StreamTrimByLength(key,
							REDISMODULE_STREAM_TRIM_APPROX, max_query_count);
				} else {
					// TODO: decide how to handle this...
				}

				// clean up
				RedisModule_CloseKey(key);

				// release GIL
				RedisModule_ThreadSafeContextUnlock(rm_ctx);
			}
		}

		GraphContext_DecreaseRefCount(gc);
	}

	RedisModule_FreeThreadSafeContext(rm_ctx);

	//--------------------------------------------------------------------------
	// determine next invocation
	//--------------------------------------------------------------------------

	// create private data for next invocation
	StreamFinishedQueryCtx *_pdata = rm_malloc(sizeof(StreamFinishedQueryCtx));

	// set next iteration graph index
	_pdata->graph_idx =	(gc == NULL) ? 0 : ctx->graph_idx;

	bool speed_up = (gc != NULL);
	if(speed_up) {
		// reduce delay, lower limit: 250ms
		_pdata->when = (250 + ctx->when) / 2;
	} else {
		// increase delay, upper limit: 1sec
		_pdata->when = (1000 + ctx->when) / 2;
	}

	// re-add task to CRON
	Cron_AddTask(ctx->when, CronTask_streamFinishedQueries, rm_free, _pdata);
}

