/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "config.h"
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include "util/redis_version.h"
#include "../deps/GraphBLAS/Include/GraphBLAS.h"

//-----------------------------------------------------------------------------
// Configuration parameters
//-----------------------------------------------------------------------------

#define TIMEOUT "TIMEOUT"  // Config param, the timeout for each query in milliseconds.
#define CACHE_SIZE "CACHE_SIZE"  // Config param, the size of each thread cache size, per graph.
#define ASYNC_DELETE "ASYNC_DELETE" // whether graphs should be deleted asynchronously
#define THREAD_COUNT "THREAD_COUNT" // Config param, number of threads in thread pool
#define RESULTSET_SIZE "RESULTSET_SIZE" // resultset size limit
#define OMP_THREAD_COUNT "OMP_THREAD_COUNT" // Config param, max number of OpenMP threads
#define VKEY_MAX_ENTITY_COUNT "VKEY_MAX_ENTITY_COUNT" // Config param, max number of entities in each virtual key
#define MAINTAIN_TRANSPOSED_MATRICES "MAINTAIN_TRANSPOSED_MATRICES" // Whether the module should maintain transposed relationship matrices

//------------------------------------------------------------------------------
// Configuration defaults
//------------------------------------------------------------------------------

#define CACHE_SIZE_DEFAULT 25
#define VKEY_MAX_ENTITY_COUNT_DEFAULT 100000

extern RG_Config config; // global module configuration

//------------------------------------------------------------------------------
// config value parsing
//------------------------------------------------------------------------------

// parse integer
// return true if string represents an integer
static inline bool _Config_ParseInteger(RedisModuleString *integer_str, long long *value) {
	int res = RedisModule_StringToLongLong(integer_str, value);
	// Return an error code if integer parsing fails or value is not positive.
	return (res == REDISMODULE_OK);
}

// parse positive integer
// return true if string represents a positive integer > 0
static inline bool _Config_ParsePositiveInteger(RedisModuleString *integer_str, long long *value) {
	bool res = _Config_ParseInteger(integer_str, value);
	// Return an error code if integer parsing fails or value is not positive.
	return (res == true && *value > 0);
}

// return true if 'rm_str' is either "yes" or "no" otherwise returns false
// sets 'value' to true if 'rm_str' is "yes"
// sets 'value to false if 'rm_str' is "no"
static inline bool _Config_ParseYesNo(RedisModuleString *rm_str, bool *value) {
	bool res = false;
	const char *str = RedisModule_StringPtrLen(rm_str, NULL);

	if(!strcasecmp(str, "yes")) {
		res = true;
		*value = true;
	}
	else if(!strcasecmp(str, "no")) {
		res = true;
		*value = false;
	}

	return res;
}

//==============================================================================
// Config access functions
//==============================================================================

//------------------------------------------------------------------------------
// timeout
//------------------------------------------------------------------------------

void Config_timeout_set(uint64_t timeout) {
	config.timeout = timeout;
}

uint Config_timeout_get(void) {
	return config.timeout;
}

//------------------------------------------------------------------------------
// thread count
//------------------------------------------------------------------------------

void Config_thread_pool_size_set(uint nthreads) {
	config.thread_pool_size = nthreads;
}

uint Config_thread_pool_size_get(void) {
	return config.thread_pool_size;
}

//------------------------------------------------------------------------------
// OpenMP thread count
//------------------------------------------------------------------------------

void Config_OMP_thread_count_set(uint nthreads) {
	config.omp_thread_count = nthreads;
}

uint Config_OMP_thread_count_get(void) {
	return config.omp_thread_count;
}

//------------------------------------------------------------------------------
// virtual key entity count
//------------------------------------------------------------------------------

void Config_virtual_key_entity_count_set(uint64_t entity_count) {
	config.vkey_entity_count = entity_count;
}

uint64_t Config_virtual_key_entity_count_get(void) {
	return config.vkey_entity_count;
}

//------------------------------------------------------------------------------
// maintain transpose
//------------------------------------------------------------------------------

void Config_maintain_transpose_set(bool maintain) {
	config.maintain_transposed_matrices = maintain;
}

bool Config_maintain_transpose_get(void) {
	return config.maintain_transposed_matrices;
}

//------------------------------------------------------------------------------
// cache size
//------------------------------------------------------------------------------

void Config_cache_size_set(uint64_t cache_size) {
	config.cache_size = cache_size;
}

uint64_t Config_cache_size_get(void) {
	return config.cache_size;
}

//------------------------------------------------------------------------------
// async delete
//------------------------------------------------------------------------------

void Config_async_delete_set(bool async_delete) {
	config.async_delete = async_delete;
}

bool Config_async_delete_get(void) {
	return config.async_delete;
}

//------------------------------------------------------------------------------
// result-set max size
//------------------------------------------------------------------------------

void Config_resultset_max_size_set(int64_t max_size) {
	if(max_size < 0) config.resultset_size = RESULTSET_SIZE_UNLIMITED;
	else config.resultset_size = max_size;
}

uint64_t Config_resultset_max_size_get(void) {
	return config.resultset_size;
}

bool Config_Contains_field(const char *field_str, Config_Option_Field *field) {
	ASSERT(field_str != NULL);

	Config_Option_Field f;

	if(!strcasecmp(field_str, THREAD_COUNT)) {
		f = Config_THREAD_POOL_SIZE;
	} else if(!strcasecmp(field_str, TIMEOUT)) {
		f = Config_TIMEOUT;
	} else if(!strcasecmp(field_str, OMP_THREAD_COUNT)) {
		f = Config_OPENMP_NTHREAD;
	} else if(!strcasecmp(field_str, VKEY_MAX_ENTITY_COUNT)) {
		f = Config_VKEY_MAX_ENTITY_COUNT;
	} else if(!strcasecmp(field_str, MAINTAIN_TRANSPOSED_MATRICES)) {
		f = Config_MAINTAIN_TRANSPOSE;
	} else if(!(strcasecmp(field_str, CACHE_SIZE))) {
		f = Config_CACHE_SIZE;
	} else if(!(strcasecmp(field_str, RESULTSET_SIZE))) {
		f = Config_RESULTSET_MAX_SIZE;
	} else {
		return false;
	}

	if(field) *field = f;
	return true;
}

const char *Config_Field_name(Config_Option_Field field) {
	const char *name = NULL;
	switch (field)
	{
		case Config_TIMEOUT:
			name = TIMEOUT;
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

		case Config_MAINTAIN_TRANSPOSE:
			name = MAINTAIN_TRANSPOSED_MATRICES;
			break;

		case Config_VKEY_MAX_ENTITY_COUNT:
			name = VKEY_MAX_ENTITY_COUNT;
			break;

		case Config_ASYNC_DELETE:
			name = ASYNC_DELETE;
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
void _Config_SetToDefaults(RedisModuleCtx *ctx) {
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
		RedisModule_Log(ctx, "notice", "Graph deletion will be done synchronously.");
	#else
		// always perform async delete when no checking for memory issues
		config.async_delete = true;
		RedisModule_Log(ctx, "notice", "Graph deletion will be done asynchronously.");
	#endif

	config.cache_size = CACHE_SIZE_DEFAULT;

	// always build transposed matrices by default
	config.maintain_transposed_matrices = true;

	// no limit on result-set size
	config.resultset_size = RESULTSET_SIZE_UNLIMITED;

	// No query timeout by default.
	config.timeout = 0;
}

int Config_Init(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	// Initialize the configuration to its default values.
	_Config_SetToDefaults(ctx);

	if(argc % 2) {
		// emit an error if we received an odd number of arguments,
		// as this indicates an invalid configuration.
		RedisModule_Log(ctx, "warning",
						"RedisGraph received %d arguments, all configurations should be key-value pairs", argc);
		return REDISMODULE_ERR;
	}

	for(int i = 0; i < argc; i += 2) {
		// Each configuration is a key-value pair. (K, V)

		//----------------------------------------------------------------------
		// get field
		//----------------------------------------------------------------------

		Config_Option_Field field;
		RedisModuleString *val = argv[i + 1];
		const char *field_str = RedisModule_StringPtrLen(argv[i], NULL);

		// exit if configuration is not aware of field
		if(!Config_Contains_field(field_str, &field)) {
			RedisModule_Log(ctx, "warning",
					"Encountered unknown configuration field '%s'", field_str);
			return REDISMODULE_ERR;
		}

		// exit if encountered an error when setting configuration
		if(!Config_Option_set(field, val)) {
			RedisModule_Log(ctx, "warning",
					"Failed setting field '%s'", field_str);
			return REDISMODULE_ERR;
		}
	}

	return REDISMODULE_OK;
}

bool Config_Option_set(Config_Option_Field field, RedisModuleString *val) {
	//--------------------------------------------------------------------------
	// set the option
	//--------------------------------------------------------------------------

	switch (field)
	{
		//----------------------------------------------------------------------
		// timeout
		//----------------------------------------------------------------------

		case Config_TIMEOUT:
			{
				long long timeout;
				if(!_Config_ParsePositiveInteger(val, &timeout)) return false;
				Config_timeout_set(timeout);
			}
			break;

		//----------------------------------------------------------------------
		// cache size
		//----------------------------------------------------------------------

		case Config_CACHE_SIZE:
			{
				long long cache_size;
				if(!_Config_ParsePositiveInteger(val, &cache_size)) return false;
				Config_cache_size_set(cache_size);
			}
			break;

		//----------------------------------------------------------------------
		// OpenMP thread count
		//----------------------------------------------------------------------

		case Config_OPENMP_NTHREAD:
			{
				long long omp_nthreads;
				if(!_Config_ParsePositiveInteger(val, &omp_nthreads)) return false;

				Config_OMP_thread_count_set(omp_nthreads);
			}
			break;

		//----------------------------------------------------------------------
		// thread-pool size
		//----------------------------------------------------------------------

		case Config_THREAD_POOL_SIZE:
			{
				long long pool_nthreads;
				if(!_Config_ParsePositiveInteger(val, &pool_nthreads)) return false;

				Config_thread_pool_size_set(pool_nthreads);
			}
			break;

		//----------------------------------------------------------------------
		// result-set size
		//----------------------------------------------------------------------

		case Config_RESULTSET_MAX_SIZE:
			{
				long long resultset_max_size;
				if(!_Config_ParseInteger(val, &resultset_max_size)) return false;

				Config_resultset_max_size_set(resultset_max_size);
			}
			break;

		//----------------------------------------------------------------------
		// maintain transpose
		//----------------------------------------------------------------------

		case Config_MAINTAIN_TRANSPOSE:
			{
				bool maintain_transpose;
				if(!_Config_ParseYesNo(val, &maintain_transpose)) return false;

				Config_maintain_transpose_set(maintain_transpose);
			}
			break;

		//----------------------------------------------------------------------
		// virtual key entity count
		//----------------------------------------------------------------------

		case Config_VKEY_MAX_ENTITY_COUNT:
			{
				long long vkey_max_entity_count;
				if(!_Config_ParsePositiveInteger(val, &vkey_max_entity_count)) return false;

				Config_virtual_key_entity_count_set(vkey_max_entity_count);
			}
			break;

		//----------------------------------------------------------------------
		// async deleteion
		//----------------------------------------------------------------------

		case Config_ASYNC_DELETE:
			{
				bool async_delete;
				if(!_Config_ParseYesNo(val, &async_delete)) return false;

				Config_async_delete_set(async_delete);
			}
			break;

	    //----------------------------------------------------------------------
	    // invalid option
	    //----------------------------------------------------------------------

        default : 
			return false;
    }

	return true;
}

bool Config_Option_get(Config_Option_Field field, ...) {

	//--------------------------------------------------------------------------
	// get the option
	//--------------------------------------------------------------------------

	va_list ap;

	switch (field)
	{
		//----------------------------------------------------------------------
		// timeout
		//----------------------------------------------------------------------

		case Config_TIMEOUT:
			{
				va_start(ap, field);
				uint64_t *timeout = va_arg(ap, uint64_t*);
				va_end(ap);

				ASSERT(timeout != NULL);
				(*timeout) = Config_timeout_get();
			}
			break;

		//----------------------------------------------------------------------
		// cache size
		//----------------------------------------------------------------------

		case Config_CACHE_SIZE:
			{
				va_start(ap, field);
				uint64_t *cache_size = va_arg(ap, uint64_t*);
				va_end(ap);

				ASSERT(cache_size != NULL);
				(*cache_size) = Config_cache_size_get();
			}
			break;

		//----------------------------------------------------------------------
		// OpenMP thread count
		//----------------------------------------------------------------------

		case Config_OPENMP_NTHREAD:
			{
				va_start(ap, field);
				uint *omp_nthreads = va_arg(ap, uint*);
				va_end(ap);

				ASSERT(omp_nthreads != NULL);
				(*omp_nthreads) = Config_OMP_thread_count_get();
			}
			break;

		//----------------------------------------------------------------------
		// thread-pool size
		//----------------------------------------------------------------------

		case Config_THREAD_POOL_SIZE:
			{
				va_start(ap, field);
				uint *pool_nthreads = va_arg(ap, uint*);
				va_end(ap);

				ASSERT(pool_nthreads != NULL);
				(*pool_nthreads) = Config_thread_pool_size_get();
			}
			break;

		//----------------------------------------------------------------------
		// result-set size
		//----------------------------------------------------------------------

		case Config_RESULTSET_MAX_SIZE:
			{
				va_start(ap, field);
				uint64_t *resultset_max_size = va_arg(ap, uint64_t*);
				va_end(ap);

				ASSERT(resultset_max_size != NULL);
				(*resultset_max_size) = Config_resultset_max_size_get();
			}
			break;

		//----------------------------------------------------------------------
		// maintain transpose
		//----------------------------------------------------------------------

		case Config_MAINTAIN_TRANSPOSE:
			{
				va_start(ap, field);
				bool *maintain_transpose = va_arg(ap, bool*);
				va_end(ap);

				ASSERT(maintain_transpose != NULL);
				(*maintain_transpose) = Config_maintain_transpose_get();
			}
			break;

		//----------------------------------------------------------------------
		// virtual key entity count
		//----------------------------------------------------------------------

		case Config_VKEY_MAX_ENTITY_COUNT:
			{
				va_start(ap, field);
				uint64_t *vkey_max_entity_count = va_arg(ap, uint64_t*);
				va_end(ap);

				ASSERT(vkey_max_entity_count != NULL);
				(*vkey_max_entity_count) = Config_virtual_key_entity_count_get();
			}
			break;

		//----------------------------------------------------------------------
		// async deleteion
		//----------------------------------------------------------------------

		case Config_ASYNC_DELETE:
			{
				va_start(ap, field);
				bool *async_delete = va_arg(ap, bool*);
				va_end(ap);

				ASSERT(async_delete != NULL);
				(*async_delete) = Config_async_delete_get();
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
