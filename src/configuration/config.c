/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "config.h"
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include "util/redis_version.h"
#include "../deps/GraphBLAS/Include/GraphBLAS.h"

//-----------------------------------------------------------------------------
// Configuration parameters
//-----------------------------------------------------------------------------
// config param, the timeout for each query in milliseconds
#define TIMEOUT "TIMEOUT"

// default timeout for read and write queries
#define TIMEOUT_DEFAULT "TIMEOUT_DEFAULT"

// max timeout that can be enforced
#define TIMEOUT_MAX "TIMEOUT_MAX"

// config param, the size of each thread cache size, per graph
#define CACHE_SIZE "CACHE_SIZE"

// whether graphs should be deleted asynchronously
#define ASYNC_DELETE "ASYNC_DELETE"

// config param, number of threads in thread pool
#define THREAD_COUNT "THREAD_COUNT"

// resultset size limit
#define RESULTSET_SIZE "RESULTSET_SIZE"

// config param, max number of OpenMP threads
#define OMP_THREAD_COUNT "OMP_THREAD_COUNT"

// config param, max number of entities in each virtual key
#define VKEY_MAX_ENTITY_COUNT "VKEY_MAX_ENTITY_COUNT"

// config param, max number of queued queries
#define MAX_QUEUED_QUERIES "MAX_QUEUED_QUERIES"

// Max mem(bytes) that query/thread can utilize at any given time
#define QUERY_MEM_CAPACITY "QUERY_MEM_CAPACITY"

// number of pending changed befor RG_Matrix flushed
#define DELTA_MAX_PENDING_CHANGES "DELTA_MAX_PENDING_CHANGES"

// size of node creation buffer
#define NODE_CREATION_BUFFER "NODE_CREATION_BUFFER"

//------------------------------------------------------------------------------
// Configuration defaults
//------------------------------------------------------------------------------

#define CACHE_SIZE_DEFAULT            25
#define QUEUED_QUERIES_UNLIMITED      UINT64_MAX
#define VKEY_MAX_ENTITY_COUNT_DEFAULT 100000

// configuration object
typedef struct {
	uint64_t timeout;                  // The timeout for each query in milliseconds.
	uint64_t timeout_max;              // max timeout that can be enforced
	uint64_t timeout_default;          // default timeout for read and write queries
	bool async_delete;                 // If true, graph deletion is done asynchronously.
	uint64_t cache_size;               // The cache size for each thread, per graph.
	uint thread_pool_size;             // Thread count for thread pool.
	uint omp_thread_count;             // Maximum number of OpenMP threads.
	uint64_t resultset_size;           // resultset maximum size, UINT64_MAX unlimited
	uint64_t vkey_entity_count;        // The limit of number of entities encoded at once for each RDB key.
	uint64_t max_queued_queries;       // max number of queued queries
	int64_t query_mem_capacity;        // Max mem(bytes) that query/thread can utilize at any given time
	uint64_t node_creation_buffer;     // Number of extra node creations to buffer as margin in matrices
	int64_t delta_max_pending_changes; // number of pending changed befor RG_Matrix flushed
	Config_on_change cb;               // callback function which being called when config param changed
} RG_Config;

RG_Config config; // global module configuration

//------------------------------------------------------------------------------
// config value parsing
//------------------------------------------------------------------------------

// parse integer
// return true if string represents an integer
static inline bool _Config_ParseInteger
(
	const char *integer_str,
	long long *value
) {
	char *endptr;
	errno = 0;    // To distinguish success/failure after call
	*value = strtoll(integer_str, &endptr, 10);

	// Return an error code if integer parsing fails.
	return (errno == 0 && endptr != integer_str && *endptr == '\0');
}

// parse positive integer
// return true if string represents a positive integer > 0
static inline bool _Config_ParsePositiveInteger
(
	const char *integer_str,
	long long *value
) {
	bool res = _Config_ParseInteger(integer_str, value);
	// Return an error code if integer parsing fails or value is not positive.
	return (res == true && *value > 0);
}

// parse non-negative integer
// return true if string represents an integer >= 0
static inline bool _Config_ParseNonNegativeInteger
(
	const char *integer_str,
	long long *value
) {
	bool res = _Config_ParseInteger(integer_str, value);
	// Return an error code if integer parsing fails or value is negative.
	return (res == true && *value >= 0);
}

// return true if 'str' is either "yes" or "no" otherwise returns false
// sets 'value' to true if 'str' is "yes"
// sets 'value to false if 'str' is "no"
static inline bool _Config_ParseYesNo
(
	const char *str,
	bool *value
) {
	bool res = false;

	if(!strcasecmp(str, "yes")) {
		res = true;
		*value = true;
	} else if(!strcasecmp(str, "no")) {
		res = true;
		*value = false;
	}

	return res;
}

//==============================================================================
// Config access functions
//==============================================================================

//------------------------------------------------------------------------------
// max queued queries
//------------------------------------------------------------------------------

static void Config_max_queued_queries_set
(
	uint64_t max_queued_queries
) {
	config.max_queued_queries = max_queued_queries;
}

static uint Config_max_queued_queries_get(void) {
	return config.max_queued_queries;
}

//------------------------------------------------------------------------------
// timeout
//------------------------------------------------------------------------------

static void Config_timeout_set
(
	uint64_t timeout
) {
	config.timeout = timeout;
}

// check if new(TIMEOUT_DEFAULT or TIMEOUT_MAX) are used
// log a deprecation message
static bool _Config_check_if_new_timeout_used() {
	bool new_timeout_set  = config.timeout_default != CONFIG_TIMEOUT_NO_TIMEOUT;
	new_timeout_set      |= config.timeout_max     != CONFIG_TIMEOUT_NO_TIMEOUT;

	if(new_timeout_set) {
		RedisModule_Log(NULL, "warning", "The TIMEOUT configuration parameter is deprecated. Please set TIMEOUT_MAX and TIMEOUT_DEFAULT instead");
		return false;
	}

	return true;
}

static bool Config_enforce_timeout_max
(
	uint64_t timeout_default,
	uint64_t timeout_max
) {
	if(timeout_max != CONFIG_TIMEOUT_NO_TIMEOUT &&
	   timeout_default > timeout_max) {
#ifdef __aarch64__
		RedisModule_Log(NULL, "warning", "The TIMEOUT_DEFAULT(%lld) configuration parameter value is higher than TIMEOUT_MAX(%lld).", timeout_default, timeout_max);
#else
		RedisModule_Log(NULL, "warning", "The TIMEOUT_DEFAULT(%ld) configuration parameter value is higher than TIMEOUT_MAX(%ld).", timeout_default, timeout_max);
#endif
		return false;
	}
	
	config.timeout_max     = timeout_max;
	config.timeout_default = timeout_default;
	return true;
}

static bool Config_timeout_default_set
(
	uint64_t timeout_default
) {
	return Config_enforce_timeout_max(timeout_default, config.timeout_max);
}

static bool Config_timeout_max_set
(
	uint64_t timeout_max
) {
	return Config_enforce_timeout_max(config.timeout_default, timeout_max);
}

static uint Config_timeout_get(void) {
	return config.timeout;
}

static uint Config_timeout_default_get(void) {
	return config.timeout_default;
}

static uint Config_timeout_max_get(void) {
	return config.timeout_max;
}

//------------------------------------------------------------------------------
// thread count
//------------------------------------------------------------------------------

static void Config_thread_pool_size_set
(
	uint nthreads
) {
	config.thread_pool_size = nthreads;
}

static uint Config_thread_pool_size_get(void) {
	return config.thread_pool_size;
}

//------------------------------------------------------------------------------
// OpenMP thread count
//------------------------------------------------------------------------------

static void Config_OMP_thread_count_set(uint nthreads) {
	config.omp_thread_count = nthreads;
}

static uint Config_OMP_thread_count_get(void) {
	return config.omp_thread_count;
}

//------------------------------------------------------------------------------
// virtual key entity count
//------------------------------------------------------------------------------

static void Config_virtual_key_entity_count_set
(
	uint64_t entity_count
) {
	config.vkey_entity_count = entity_count;
}

static uint64_t Config_virtual_key_entity_count_get(void) {
	return config.vkey_entity_count;
}

//------------------------------------------------------------------------------
// cache size
//------------------------------------------------------------------------------

static void Config_cache_size_set
(
	uint64_t cache_size
) {
	config.cache_size = cache_size;
}

static uint64_t Config_cache_size_get(void) {
	return config.cache_size;
}

//------------------------------------------------------------------------------
// async delete
//------------------------------------------------------------------------------

static void Config_async_delete_set
(
	bool async_delete
) {
	config.async_delete = async_delete;
}

static bool Config_async_delete_get(void) {
	return config.async_delete;
}

//------------------------------------------------------------------------------
// result-set max size
//------------------------------------------------------------------------------

static void Config_resultset_max_size_set
(
	int64_t max_size
) {
	if(max_size < 0) config.resultset_size = RESULTSET_SIZE_UNLIMITED;
	else config.resultset_size = max_size;
}

static uint64_t Config_resultset_max_size_get(void) {
	return config.resultset_size;
}

//------------------------------------------------------------------------------
// query mem capacity
//------------------------------------------------------------------------------

static void Config_query_mem_capacity_set
(
	int64_t capacity
) {
	if(capacity <= 0)
		config.query_mem_capacity = QUERY_MEM_CAPACITY_UNLIMITED;
	else
		config.query_mem_capacity = capacity;
}

static uint64_t Config_query_mem_capacity_get(void) {
	return config.query_mem_capacity;
}

//------------------------------------------------------------------------------
// delta max pending changes
//------------------------------------------------------------------------------

static void Config_delta_max_pending_changes_set
(
	int64_t capacity
) {
	if(capacity == 0)
		config.delta_max_pending_changes = DELTA_MAX_PENDING_CHANGES_DEFAULT;
	else
		config.delta_max_pending_changes = capacity;
}

static uint64_t Config_delta_max_pending_changes_get(void) {
	return config.delta_max_pending_changes;
}

//------------------------------------------------------------------------------
// node creation buffer
//------------------------------------------------------------------------------

static void Config_node_creation_buffer_set
(
	uint64_t buf_size
) {
	config.node_creation_buffer = buf_size;
}

static uint64_t Config_node_creation_buffer_get(void) {
	return config.node_creation_buffer;
}

bool Config_Contains_field
(
	const char *field_str,
	Config_Option_Field *field
) {
	ASSERT(field_str != NULL);

	Config_Option_Field f;

	if(!strcasecmp(field_str, THREAD_COUNT)) {
		f = Config_THREAD_POOL_SIZE;
	} else if(!strcasecmp(field_str, TIMEOUT)) {
		f = Config_TIMEOUT;
	} else if(!(strcasecmp(field_str, TIMEOUT_DEFAULT))) {
		f = Config_TIMEOUT_DEFAULT;
	} else if(!(strcasecmp(field_str, TIMEOUT_MAX))) {
		f = Config_TIMEOUT_MAX;
	} else if(!strcasecmp(field_str, OMP_THREAD_COUNT)) {
		f = Config_OPENMP_NTHREAD;
	} else if(!strcasecmp(field_str, VKEY_MAX_ENTITY_COUNT)) {
		f = Config_VKEY_MAX_ENTITY_COUNT;
	} else if(!(strcasecmp(field_str, CACHE_SIZE))) {
		f = Config_CACHE_SIZE;
	} else if(!(strcasecmp(field_str, RESULTSET_SIZE))) {
		f = Config_RESULTSET_MAX_SIZE;
	} else if(!(strcasecmp(field_str, MAX_QUEUED_QUERIES))) {
		f = Config_MAX_QUEUED_QUERIES;
	} else if(!(strcasecmp(field_str, QUERY_MEM_CAPACITY))) {
		f = Config_QUERY_MEM_CAPACITY;
	} else if(!(strcasecmp(field_str, DELTA_MAX_PENDING_CHANGES))) {
		f = Config_DELTA_MAX_PENDING_CHANGES;
	} else if(!(strcasecmp(field_str, NODE_CREATION_BUFFER))) {
		f = Config_NODE_CREATION_BUFFER;
	} else {
		return false;
	}

	if(field) *field = f;
	return true;
}

const char *Config_Field_name
(
	Config_Option_Field field
) {
	const char *name = NULL;
	switch(field) {
		case Config_TIMEOUT:
			name = TIMEOUT;
			break;

		case Config_TIMEOUT_DEFAULT:
			name = TIMEOUT_DEFAULT;
			break;

		case Config_TIMEOUT_MAX:
			name = TIMEOUT_MAX;
			break;

		case Config_CACHE_SIZE:
			name = CACHE_SIZE;
			break;

		case Config_OPENMP_NTHREAD:
			name = OMP_THREAD_COUNT;
			break;

		case Config_THREAD_POOL_SIZE:
			name = THREAD_COUNT;
			break;

		case Config_RESULTSET_MAX_SIZE:
			name = RESULTSET_SIZE;
			break;

		case Config_VKEY_MAX_ENTITY_COUNT:
			name = VKEY_MAX_ENTITY_COUNT;
			break;

		case Config_ASYNC_DELETE:
			name = ASYNC_DELETE;
			break;

		case Config_MAX_QUEUED_QUERIES:
			name = MAX_QUEUED_QUERIES;
			break;

		case Config_QUERY_MEM_CAPACITY:
			name = QUERY_MEM_CAPACITY;
			break;

		case Config_DELTA_MAX_PENDING_CHANGES:
			name = DELTA_MAX_PENDING_CHANGES;
			break;

		case Config_NODE_CREATION_BUFFER:
			name = NODE_CREATION_BUFFER;
			break;

		//----------------------------------------------------------------------
		// invalid option
		//----------------------------------------------------------------------

		default :
			ASSERT("invalid option field" && false);
			break;
	}

	return name;
}

// initialize every module-level configuration to its default value
static void _Config_SetToDefaults(void) {
	// the thread pool's default size is equal to the system's number of cores
	int CPUCount = sysconf(_SC_NPROCESSORS_ONLN);
	config.thread_pool_size = (CPUCount != -1) ? CPUCount : 1;

	// use the GraphBLAS-defined number of OpenMP threads by default
	GxB_get(GxB_NTHREADS, &config.omp_thread_count);

	// the default entity count of virtual keys
	config.vkey_entity_count = VKEY_MAX_ENTITY_COUNT_DEFAULT;

	// MEMCHECK compile flag;
	#ifdef MEMCHECK
		// disable async delete during memcheck
		config.async_delete = false;
	#else
		// always perform async delete when no checking for memory issues
		config.async_delete = true;
	#endif

	config.cache_size = CACHE_SIZE_DEFAULT;

	// no limit on result-set size
	config.resultset_size = RESULTSET_SIZE_UNLIMITED;

	// no query timeout by default
	config.timeout = CONFIG_TIMEOUT_NO_TIMEOUT;

	// no max timeout by default
	config.timeout_max = CONFIG_TIMEOUT_NO_TIMEOUT;

	// no query timeout by default
	config.timeout_default = CONFIG_TIMEOUT_NO_TIMEOUT;

	// no limit on number of queued queries by default
	config.max_queued_queries = QUEUED_QUERIES_UNLIMITED;

	// no limit on query memory capacity
	config.query_mem_capacity = QUERY_MEM_CAPACITY_UNLIMITED;

	// number of pending changed befor RG_Matrix flushed
	config.delta_max_pending_changes = DELTA_MAX_PENDING_CHANGES_DEFAULT;

	// the amount of empty space to reserve for node creations in matrices
	config.node_creation_buffer = NODE_CREATION_BUFFER_DEFAULT;
}

int Config_Init
(
	RedisModuleCtx *ctx,
	RedisModuleString **argv,
	int argc
) {
	// make sure reconfiguration callback is already registered
	ASSERT(config.cb != NULL);

	// initialize the configuration to its default values
	_Config_SetToDefaults();

	if(argc % 2) {
		// emit an error if we received an odd number of arguments,
		// as this indicates an invalid configuration
		RedisModule_Log(ctx, "warning",
						"RedisGraph received %d arguments, all configurations should be key-value pairs", argc);
		return REDISMODULE_ERR;
	}

	bool old_timeout_specified = false;
	bool new_timeout_specified = false;

	for(int i = 0; i < argc; i += 2) {
		// each configuration is a key-value pair. (K, V)

		//----------------------------------------------------------------------
		// get field
		//----------------------------------------------------------------------

		Config_Option_Field field;
		RedisModuleString *val = argv[i + 1];
		const char *field_str = RedisModule_StringPtrLen(argv[i], NULL);
		const char *val_str = RedisModule_StringPtrLen(val, NULL);

		// exit if configuration is not aware of field
		if(!Config_Contains_field(field_str, &field)) {
			RedisModule_Log(ctx, "error",
							"Encountered unknown configuration field '%s'", field_str);
			return REDISMODULE_ERR;
		}

		if(field == Config_TIMEOUT_DEFAULT || field == Config_TIMEOUT_MAX) {
			new_timeout_specified = true;
		}

		if(field == Config_TIMEOUT) {
			old_timeout_specified = true;
		}

		// exit if encountered an error when setting configuration
		char *error = NULL;
		if(!Config_Option_set(field, val_str, &error)) {
			if(error != NULL) {
				RedisModule_Log(ctx, "error",
							"Failed setting field '%s' with error: %s", field_str, error);
			} else {
				RedisModule_Log(ctx, "error",
							"Failed setting field '%s'", field_str);	
			}
			return REDISMODULE_ERR;
		}
	}

	if(old_timeout_specified && new_timeout_specified) {
		RedisModule_Log(ctx, "error",
						"The TIMEOUT configuration parameter should be removed when specifying TIMEOUT_DEFAULT and/or TIMEOUT_MAX");
		return REDISMODULE_ERR;
	}

	return REDISMODULE_OK;
}

bool Config_Option_get
(
	Config_Option_Field field,
	...
) {
	//--------------------------------------------------------------------------
	// get the option
	//--------------------------------------------------------------------------

	va_list ap;

	switch(field) {
		case Config_MAX_QUEUED_QUERIES: {

			va_start(ap, field);
			uint64_t *max_queued_queries = va_arg(ap, uint64_t *);
			va_end(ap);

			ASSERT(max_queued_queries != NULL);
			(*max_queued_queries) = Config_max_queued_queries_get();
		}
		break;
		//----------------------------------------------------------------------
		// timeout
		//----------------------------------------------------------------------

		case Config_TIMEOUT: {
			va_start(ap, field);
			uint64_t *timeout = va_arg(ap, uint64_t *);
			va_end(ap);

			ASSERT(timeout != NULL);
			(*timeout) = Config_timeout_get();
		}
		break;

		//----------------------------------------------------------------------
		// timeout default
		//----------------------------------------------------------------------

		case Config_TIMEOUT_DEFAULT: {
			va_start(ap, field);
			uint64_t *timeout_default = va_arg(ap, uint64_t *);
			va_end(ap);

			ASSERT(timeout_default != NULL);
			(*timeout_default) = Config_timeout_default_get();
		}
		break;

		//----------------------------------------------------------------------
		// timeout max
		//----------------------------------------------------------------------

		case Config_TIMEOUT_MAX: {
			va_start(ap, field);
			uint64_t *timeout_max = va_arg(ap, uint64_t *);
			va_end(ap);

			ASSERT(timeout_max != NULL);
			(*timeout_max) = Config_timeout_max_get();
		}
		break;

		//----------------------------------------------------------------------
		// cache size
		//----------------------------------------------------------------------

		case Config_CACHE_SIZE: {
			va_start(ap, field);
			uint64_t *cache_size = va_arg(ap, uint64_t *);
			va_end(ap);

			ASSERT(cache_size != NULL);
			(*cache_size) = Config_cache_size_get();
		}
		break;

		//----------------------------------------------------------------------
		// OpenMP thread count
		//----------------------------------------------------------------------

		case Config_OPENMP_NTHREAD: {
			va_start(ap, field);
			uint *omp_nthreads = va_arg(ap, uint *);
			va_end(ap);

			ASSERT(omp_nthreads != NULL);
			(*omp_nthreads) = Config_OMP_thread_count_get();
		}
		break;

		//----------------------------------------------------------------------
		// thread-pool size
		//----------------------------------------------------------------------

		case Config_THREAD_POOL_SIZE: {
			va_start(ap, field);
			uint *pool_nthreads = va_arg(ap, uint *);
			va_end(ap);

			ASSERT(pool_nthreads != NULL);
			(*pool_nthreads) = Config_thread_pool_size_get();
		}
		break;

		//----------------------------------------------------------------------
		// result-set size
		//----------------------------------------------------------------------

		case Config_RESULTSET_MAX_SIZE: {
			va_start(ap, field);
			uint64_t *resultset_max_size = va_arg(ap, uint64_t *);
			va_end(ap);

			ASSERT(resultset_max_size != NULL);
			(*resultset_max_size) = Config_resultset_max_size_get();
		}
		break;

		//----------------------------------------------------------------------
		// virtual key entity count
		//----------------------------------------------------------------------

		case Config_VKEY_MAX_ENTITY_COUNT: {
			va_start(ap, field);
			uint64_t *vkey_max_entity_count = va_arg(ap, uint64_t *);
			va_end(ap);

			ASSERT(vkey_max_entity_count != NULL);
			(*vkey_max_entity_count) = Config_virtual_key_entity_count_get();
		}
		break;

		//----------------------------------------------------------------------
		// async deleteion
		//----------------------------------------------------------------------

		case Config_ASYNC_DELETE: {
			va_start(ap, field);
			bool *async_delete = va_arg(ap, bool *);
			va_end(ap);

			ASSERT(async_delete != NULL);
			(*async_delete) = Config_async_delete_get();
		}
		break;

		//----------------------------------------------------------------------
		// query mem capacity
		//----------------------------------------------------------------------

		case Config_QUERY_MEM_CAPACITY: {
			va_start(ap, field);
			int64_t *query_mem_capacity = va_arg(ap, int64_t *);
			va_end(ap);

			ASSERT(query_mem_capacity != NULL);
			(*query_mem_capacity) = Config_query_mem_capacity_get();
		}
		break;

		//----------------------------------------------------------------------
		// number of pending changed befor RG_Matrix flushed
		//----------------------------------------------------------------------

		case Config_DELTA_MAX_PENDING_CHANGES: {
			va_start(ap, field);
			int64_t *delta_max_pending_changes = va_arg(ap, int64_t *);
			va_end(ap);

			ASSERT(delta_max_pending_changes != NULL);
			(*delta_max_pending_changes) = Config_delta_max_pending_changes_get();
		}
		break;

		//----------------------------------------------------------------------
		// size of buffer to maintain as margin in matrices
		//----------------------------------------------------------------------

		case Config_NODE_CREATION_BUFFER: {
			va_start(ap, field);
			uint64_t *node_creation_buffer = va_arg(ap, uint64_t *);
			va_end(ap);

			ASSERT(node_creation_buffer != NULL);
			(*node_creation_buffer) = Config_node_creation_buffer_get();
		}
		break;

		//----------------------------------------------------------------------
		// invalid option
		//----------------------------------------------------------------------

		default :
			ASSERT("invalid option field" && false);
			return false;
	}

	return true;
}

bool Config_Option_set
(
	Config_Option_Field field,
	const char *val,
	char **err
) {
	//--------------------------------------------------------------------------
	// set the option
	//--------------------------------------------------------------------------

	switch(field) {
		//----------------------------------------------------------------------
		// max queued queries
		//----------------------------------------------------------------------

		case Config_MAX_QUEUED_QUERIES: {
			long long max_queued_queries;
			if(!_Config_ParsePositiveInteger(val, &max_queued_queries)) {
				return false;
			}
			Config_max_queued_queries_set(max_queued_queries);
		}
		break;

		//----------------------------------------------------------------------
		// timeout
		//----------------------------------------------------------------------

		case Config_TIMEOUT: {
			long long timeout;
			if(!_Config_ParseNonNegativeInteger(val, &timeout)) return false;
			if(!_Config_check_if_new_timeout_used()) {
				if(err) *err = "The TIMEOUT configuration parameter is deprecated. Please set TIMEOUT_MAX and TIMEOUT_DEFAULT instead";
				return false;
			}
			Config_timeout_set(timeout);
		}
		break;

		//----------------------------------------------------------------------
		// timeout default
		//----------------------------------------------------------------------

		case Config_TIMEOUT_DEFAULT: {
			long long timeout_default;
			if(!_Config_ParseNonNegativeInteger(val, &timeout_default)) return false;
			if(!Config_timeout_default_set(timeout_default)) {
				if(err) *err = "TIMEOUT_DEFAULT configuration parameter cannot be set to a value higher than TIMEOUT_MAX";
				return false;
			}
		}
		break;

		//----------------------------------------------------------------------
		// timeout max
		//----------------------------------------------------------------------

		case Config_TIMEOUT_MAX: {
			long long timeout_max;
			if(!_Config_ParseNonNegativeInteger(val, &timeout_max)) return false;
			if(!Config_timeout_max_set(timeout_max)) {
				if(err) *err = "TIMEOUT_MAX configuration parameter cannot be set to a value lower than TIMEOUT_DEFAULT";
				return false;
			}
		}
		break;

		//----------------------------------------------------------------------
		// cache size
		//----------------------------------------------------------------------

		case Config_CACHE_SIZE: {
			long long cache_size;
			if(!_Config_ParsePositiveInteger(val, &cache_size)) return false;
			Config_cache_size_set(cache_size);
		}
		break;

		//----------------------------------------------------------------------
		// OpenMP thread count
		//----------------------------------------------------------------------

		case Config_OPENMP_NTHREAD: {
			long long omp_nthreads;
			if(!_Config_ParsePositiveInteger(val, &omp_nthreads)) return false;

			Config_OMP_thread_count_set(omp_nthreads);
		}
		break;

		//----------------------------------------------------------------------
		// thread-pool size
		//----------------------------------------------------------------------

		case Config_THREAD_POOL_SIZE: {
			long long pool_nthreads;
			if(!_Config_ParsePositiveInteger(val, &pool_nthreads)) return false;

			Config_thread_pool_size_set(pool_nthreads);
		}
		break;

		//----------------------------------------------------------------------
		// result-set size
		//----------------------------------------------------------------------

		case Config_RESULTSET_MAX_SIZE: {
			long long resultset_max_size;
			if(!_Config_ParseInteger(val, &resultset_max_size)) return false;

			Config_resultset_max_size_set(resultset_max_size);
		}
		break;

		//----------------------------------------------------------------------
		// virtual key entity count
		//----------------------------------------------------------------------

		case Config_VKEY_MAX_ENTITY_COUNT: {
			long long vkey_max_entity_count;
			if(!_Config_ParseNonNegativeInteger(val, &vkey_max_entity_count)) return false;

			Config_virtual_key_entity_count_set(vkey_max_entity_count);
		}
		break;

		//----------------------------------------------------------------------
		// async deleteion
		//----------------------------------------------------------------------

		case Config_ASYNC_DELETE: {
			bool async_delete;
			if(!_Config_ParseYesNo(val, &async_delete)) return false;

			Config_async_delete_set(async_delete);
		}
		break;

		//----------------------------------------------------------------------
		// query mem capacity
		//----------------------------------------------------------------------

		case Config_QUERY_MEM_CAPACITY: {
			long long query_mem_capacity;
			if(!_Config_ParseNonNegativeInteger(val, &query_mem_capacity)) return false;

			Config_query_mem_capacity_set(query_mem_capacity);
		}
		break;

		//----------------------------------------------------------------------
		// number of pending changed befor RG_Matrix flushed
		//----------------------------------------------------------------------

		case Config_DELTA_MAX_PENDING_CHANGES: {
			long long delta_max_pending_changes;
			if(!_Config_ParseNonNegativeInteger(val, &delta_max_pending_changes)) return false;

			Config_delta_max_pending_changes_set(delta_max_pending_changes);
		}
		break;

		//----------------------------------------------------------------------
		// size of buffer to maintain as margin in matrices
		//----------------------------------------------------------------------

		case Config_NODE_CREATION_BUFFER: {
			long long node_creation_buffer;
			if(!_Config_ParseNonNegativeInteger(val, &node_creation_buffer)) return false;

			// node_creation_buffer should be at-least 128
			node_creation_buffer =
				(node_creation_buffer < 128) ? 128: node_creation_buffer;

			// retrieve the MSB of the value
			long long msb = (sizeof(long long) * 8) - __builtin_clzll(node_creation_buffer);
			long long set_msb = 1 << (msb - 1);

			// if the value is not a power of 2
			// (if any bits other than the MSB are 1),
			// raise it to the next power of 2
			if((~set_msb & node_creation_buffer) != 0) {
				node_creation_buffer = 1 << msb;
			}
			Config_node_creation_buffer_set(node_creation_buffer);
		}
		break;

		//----------------------------------------------------------------------
		// invalid option
		//----------------------------------------------------------------------

		default:
			return false;
	}

	if(config.cb) config.cb(field);

	return true;
}

// dry run configuration change
bool Config_Option_dryrun
(
	Config_Option_Field field,
	const char *val,
	char **err
) {
	// clone configuration
	RG_Config config_clone = config;

	// disable configuration notification
	config.cb = NULL;

	// NOTE: for a short period of time
	// whoever might query the configuration WILL see this modification
	bool valid = Config_Option_set(field, val, err);

	// restore original configuration
	config = config_clone;

	return valid;
}

void Config_Subscribe_Changes
(
	Config_on_change cb
) {
	ASSERT(cb != NULL);
	config.cb = cb;
}

