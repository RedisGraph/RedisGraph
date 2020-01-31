/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <stdio.h>
#include <assert.h>
#include <unistd.h>

#include "./slow_log.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../util/thpool/thpool.h"

static SlowLog *_slowlog = NULL;

static int get_thread_id() {
	extern threadpool _thpool;  // Declared in module.c

	/* thpool_get_thread_id returns -1 if pthread_self isn't in the thread pool
	 * most likely Redis main thread */
	int thread_id = thpool_get_thread_id(_thpool, pthread_self());
	thread_id += 1; // +1 to compensate for Redis main thread.
	return thread_id;
}

static SlowLogItem *_SlowLogItem_New(const char *cmd, const char *graph_id, const char *query,
									 double latency) {
	SlowLogItem *item = rm_malloc(sizeof(SlowLogItem));
	item->cmd = rm_strdup(cmd);
	item->query = rm_strdup(query);
	item->graph_id = rm_strdup(graph_id);
	item->latency = latency;
	time(&item->time);
	return item;
}

static void _SlowLog_Item_Free(SlowLogItem *item) {
	assert(item);
	rm_free(item->cmd);
	rm_free(item->query);
	rm_free(item->graph_id);
	rm_free(item);
}

// Compares two heap record nodes.
static int _slowlog_elem_compare(const void *A, const void *B, const void *udata) {
	SlowLogItem *a = (SlowLogItem *)A;
	SlowLogItem *b = (SlowLogItem *)B;
	return b->latency - a->latency;
}

static inline size_t _compute_key(char **s, const char *cmd, const char *graph_id,
								  const char *query) {
	return asprintf(s, "%s %s %s", cmd, graph_id, query);
}

static size_t _SlowLogItem_ToString(const SlowLogItem *item, char **s) {
	assert(item);
	return _compute_key(s, item->cmd, item->graph_id, item->query);
}

static SlowLog *_SlowLog_Create(int thread_count) {
	SlowLog *slowlog = rm_malloc(sizeof(SlowLog));
	slowlog->count = thread_count;
	slowlog->lookup = rm_malloc(sizeof(rax *) * thread_count);
	slowlog->min_heap = rm_malloc(sizeof(heap_t *) * thread_count);
	slowlog->locks = rm_malloc(sizeof(pthread_mutex_t) * thread_count);

	for(int i = 0; i < thread_count; i++) {
		slowlog->lookup[i] = raxNew();
		slowlog->min_heap[i] = heap_new(_slowlog_elem_compare, NULL);
		assert(pthread_mutex_init(slowlog->locks + i, NULL) == 0);
	}

	return slowlog;
}

static void _SlowLog_Free(SlowLog *slowlog) {
	assert(slowlog);

	for(int i = 0; i < slowlog->count; i++) {
		rax *lookup = slowlog->lookup[i];
		heap_t *heap = slowlog->min_heap[i];

		/* Free each slowlog item, these are shared between
		 * the rax and the heap objects. */
		while(heap_count(heap)) {
			SlowLogItem *item = heap_poll(heap);
			_SlowLog_Item_Free(item);
		}

		heap_free(heap);
		raxFree(lookup);
		assert(pthread_mutex_destroy(slowlog->locks + i) == 0);
	}

	// Free arrays.
	rm_free(slowlog->locks);
	rm_free(slowlog->lookup);
	rm_free(slowlog->min_heap);
	rm_free(slowlog);
	slowlog = NULL;
}

static bool _SlowLog_Contains(const SlowLog *slowlog, int t_id, const char *cmd,
							  const char *graph_id, const char *query, char **key, SlowLogItem **item) {
	size_t len = _compute_key(key, cmd, graph_id, query);
	rax *lookup = slowlog->lookup[t_id];
	*item = (SlowLogItem *)raxFind(lookup, (unsigned char *)key, len);
	if(*item == raxNotFound) *item = NULL;
	return (*item != NULL);
}

static void _SlowLog_Add(SlowLog *slowlog, int t_id, const char *cmd, const char *graph_id,
						 const char *query, double latency) {
	printf("_SlowLog_Add\n");

	assert(slowlog && cmd && graph_id && query && latency >= 0);

	rax *lookup = slowlog->lookup[t_id];
	heap_t *heap = slowlog->min_heap[t_id];
	pthread_mutex_lock(slowlog->locks + t_id);

	char *key;
	SlowLogItem *existing_item;
	bool exists = _SlowLog_Contains(slowlog, t_id, cmd, graph_id, query, &key, &existing_item);
	printf("item key: %s\n", key);

	if(exists) {
		printf("Similar item already exists.\n");
		/* A similar item is already part of the slowlog.
		 * see if we need to replace it. */
		if(existing_item->latency < latency) {
			printf("Existing item should be replaced\n");
			// Replace existing item with new item.
			SlowLogItem *item = _SlowLogItem_New(cmd, graph_id, query, latency);

			// Overwrite item in lookup.
			raxInsert(lookup, (unsigned char *)key, strlen(key), item, NULL);

			// Remove existing item from heap.
			assert(heap_remove_item(heap, existing_item) != NULL);
			_SlowLog_Item_Free(existing_item);

			// Introduce new item.
			heap_offer(slowlog->min_heap + t_id, item);
		}

		/* Done, if an item already existed we either replace it
		 * or leave it. */
		goto cleanup;
	}

	/* Similar item does not exist in the log.
	 * Check if there's enough room to store item. */
	printf("No similar item in log.\n");
	int introduce_item = 0;
	if(heap_count(heap) < SLOW_LOG_SIZE) {
		printf("Enough space for item\n");
		introduce_item = 1;
	} else {
		printf("Not enough space in log for item\n");
		// Not enough room, see if item should be tracked.
		SlowLogItem *top = heap_peek(heap);
		if(top->latency < latency) {
			printf("Item should be introduced to log\n");
			top = heap_poll(heap);

			char *top_str;
			size_t len = _SlowLogItem_ToString(top, &top_str);
			assert(raxRemove(lookup, (unsigned char *)top_str, len, NULL) == 1);
			free(top_str);

			_SlowLog_Item_Free(top);
			introduce_item = 1;
		}
	}

	if(introduce_item) {
		printf("Adding item to log\n");
		SlowLogItem *item = _SlowLogItem_New(cmd, graph_id, query, latency);
		assert(heap_offer(slowlog->min_heap + t_id, item) == 0);
		raxInsert(lookup, (unsigned char *)key, strlen(key), item, NULL);
	}

cleanup:
	pthread_mutex_unlock(slowlog->locks + t_id);
	free(key);
}

void SlowLog_Add(const char *cmd, const char *graph_id, const char *query,
				 double latency) {
	if(!_slowlog) {
		extern threadpool _thpool;  // Declared in module.c
		int thread_count = thpool_num_threads(_thpool);
		thread_count += 1;  // Redis main thread.
		_slowlog = _SlowLog_Create(thread_count);
	}

	_SlowLog_Add(_slowlog, get_thread_id(), cmd, graph_id, query, latency);
}

void SlowLog_Replay(RedisModuleCtx *ctx) {
	// No slowlog, reply with an empty array.
	if(!_slowlog) {
		RedisModule_ReplyWithArray(ctx, 0);
		return;
	}

	SlowLog *aggregated_slowlog = _SlowLog_Create(1);

	for(int t_id = 0; t_id < _slowlog->count; t_id++) {
		heap_t *heap = _slowlog->min_heap[t_id];
		rax *lookup = _slowlog->lookup[t_id];

		// Deplete heap.
		pthread_mutex_lock(_slowlog->locks + t_id);

		raxIterator iter;
		raxStart(&iter, lookup);
		raxSeek(&iter, "^", NULL, 0);
		while(raxNext(&iter)) {
			SlowLogItem *item = iter.data;
			_SlowLog_Add(aggregated_slowlog, 0, item->cmd, item->graph_id, item->query, item->latency);
		}
		raxStop(&iter);

		pthread_mutex_unlock(_slowlog->locks + t_id);
	}

	heap_t *heap = aggregated_slowlog->min_heap[0];
	RedisModule_ReplyWithArray(ctx, heap_count(heap));

	while(heap_count(heap)) {
		SlowLogItem *item = heap_poll(heap);
		RedisModule_ReplyWithArray(ctx, 5);
		RedisModule_ReplyWithDouble(ctx, item->time);
		RedisModule_ReplyWithStringBuffer(ctx, (const char *)item->cmd, strlen(item->cmd));
		RedisModule_ReplyWithStringBuffer(ctx, (const char *)item->graph_id, strlen(item->graph_id));
		RedisModule_ReplyWithStringBuffer(ctx, (const char *)item->query, strlen(item->query));
		RedisModule_ReplyWithDouble(ctx, item->latency);
	}

	_SlowLog_Free(aggregated_slowlog);
}
