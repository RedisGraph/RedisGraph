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

#define CACHE_SIZE "CACHE_SIZE"             // number of entries in cache
#define THREAD_COUNT "THREAD_COUNT"         // number of threads in thread pool
#define RESULTSET_SIZE "RESULTSET_SIZE"     // resultset size limit
#define OMP_THREAD_COUNT "OMP_THREAD_COUNT" // max number of OpenMP threads
#define VKEY_MAX_ENTITY_COUNT "VKEY_MAX_ENTITY_COUNT" // max number of entities in each virtual key
#define MAINTAIN_TRANSPOSED_MATRICES "MAINTAIN_TRANSPOSED_MATRICES" // whether the module should maintain transposed relationship matrices

//------------------------------------------------------------------------------
// Configuration defaults
//------------------------------------------------------------------------------

#define CACHE_SIZE_DEFAULT 25
#define VKEY_MAX_ENTITY_COUNT_DEFAULT 100000

extern RG_Config config; // global module configuration

//------------------------------------------------------------------------------
// config value parsing
//------------------------------------------------------------------------------

// parse positive integer
// return true if string represents a positive integer > 0
static inline bool _Config_ParsePositiveInteger(RedisModuleString *integer_str, long long *value) {
	int res = RedisModule_StringToLongLong(integer_str, value);
	// Return an error code if integer parsing fails or value is not positive.
	return (res == REDISMODULE_OK && *value > 0);
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

void Config_resultset_max_size_set(uint64_t max_size) {
	config.resultset_size = max_size;
}

uint64_t Config_resultset_max_size_get(void) {
	return config.resultset_size;
}

bool Config_Contains_field(const char *field_str, Config_Option_Field *field) {
	ASSERT(field != NULL);
	ASSERT(field_str != NULL);

	if(!strcasecmp(field_str, THREAD_COUNT)) {
		*field = Config_THREAD_POOL_SIZE;
	} else if(!strcasecmp(field_str, OMP_THREAD_COUNT)) {
		*field = Config_OPENMP_NTHREAD;
	} else if(!strcasecmp(field_str, VKEY_MAX_ENTITY_COUNT)) {
		*field = Config_VKEY_MAX_ENTITY_COUNT;
	} else if(!strcasecmp(field_str, MAINTAIN_TRANSPOSED_MATRICES)) {
		*field = Config_MAINTAIN_TRANSPOSE;
	} else if(!(strcasecmp(field_str, CACHE_SIZE))) {
		*field = Config_CACHE_SIZE;
	} else if(!(strcasecmp(field_str, RESULTSET_SIZE))) {
		*field = Config_RESULTSET_MAX_SIZE;
	} else {
		return false;
	}

	return true;
}

// Initialize every module-level configuration to its default value.
void _Config_SetToDefaults(RedisModuleCtx *ctx) {
	// The thread pool's default size is equal to the system's number of cores.
	int CPUCount = sysconf(_SC_NPROCESSORS_ONLN);
	config.thread_pool_size = (CPUCount != -1) ? CPUCount : 1;

	// Use the GraphBLAS-defined number of OpenMP threads by default.
	GxB_get(GxB_NTHREADS, &config.omp_thread_count);

	if(Redis_Version_GreaterOrEqual(6, 0, 0)) {
		// The default entity count of virtual keys for server versions >= 6 is set by macro.
		config.vkey_entity_count = VKEY_MAX_ENTITY_COUNT_DEFAULT;
	} else {
		// For redis-server versions below 6.0.0, we will not split the graph into virtual keys.
		config.vkey_entity_count = VKEY_ENTITY_COUNT_UNLIMITED;
	}

	// MEMCHECK compile flag;
	#ifdef MEMCHECK
		// Disable async delete during memcheck.
		config.async_delete = false;
		RedisModule_Log(ctx, "notice", "Graph deletion will be done synchronously.");
	#else
		// Always perform async delete when no checking for memory issues.
		config.async_delete = true;
		RedisModule_Log(ctx, "notice", "Graph deletion will be done asynchronously.");
	#endif

	config.cache_size = CACHE_SIZE_DEFAULT;

	// Always build transposed matrices by default.
	config.maintain_transposed_matrices = true;

	// No limit on result-set size
	config.resultset_size = RESULTSET_SIZE_UNLIMITED;
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

	long long v;                    // config value
	int res = REDISMODULE_OK;       // return value
	const char *invalid_arg = NULL; // error message

	for(int cur = 0; cur < argc; cur += 2) {
		// Each configuration is a key-value pair. (K, V)

		//----------------------------------------------------------------------
		// get field
		//----------------------------------------------------------------------

		Config_Option_Field field;
		const char *field_str = RedisModule_StringPtrLen(argv[cur], NULL);
		RedisModuleString *val = argv[cur + 1];

		// exit if configuration is not aware of field
		if(!Config_Contains_field(field_str, &field)) {
			RedisModule_Log(ctx, "warning",
					"Encountered unknown configuration field '%s'", field_str);
			return REDISMODULE_ERR;
		}

		switch(field) {

			//------------------------------------------------------------------
			// cache size
			//------------------------------------------------------------------

			case Config_CACHE_SIZE:
				if(!_Config_ParsePositiveInteger(val, &v)) {
					invalid_arg = RedisModule_StringPtrLen(val, NULL);

					RedisModule_Log(ctx, "warning",
							"Could not parse cache size argument '%s' as an integer",
							invalid_arg);

					return REDISMODULE_ERR;
				}

				res = Config_Option_set(field, v);
				ASSERT(res == 1);

				// log the cache size
				RedisModule_Log(ctx, "notice", "Cache size is set to %lld.", v);
				break;

			//------------------------------------------------------------------
			// OpenMP thread count
			//------------------------------------------------------------------
			case Config_OPENMP_NTHREAD:
				{
					if(!_Config_ParsePositiveInteger(val, &v)) {
						invalid_arg = RedisModule_StringPtrLen(val, NULL);

						RedisModule_Log(ctx, "warning",
								"Specified invalid maximum '%s' for OpenMP thread count",
								invalid_arg);

						return REDISMODULE_ERR;
					}

					res = Config_Option_set(field, v);
					ASSERT(res == 1);

					// log the OpenMP thread count
					RedisModule_Log(ctx, "notice",
							"OpenMP thread count set to %lld.", v);

					break;
				}

			//------------------------------------------------------------------
			// thread pool size
			//------------------------------------------------------------------

			case Config_THREAD_POOL_SIZE:
				{
					if(!_Config_ParsePositiveInteger(val, &v)) {
						invalid_arg = RedisModule_StringPtrLen(val, NULL);

						RedisModule_Log(ctx, "warning",
								"Received invalid value '%s' as thread count argument",
								invalid_arg);

						return REDISMODULE_ERR;
					}

					if(v > INT_MAX) {
						invalid_arg = RedisModule_StringPtrLen(val, NULL);

						RedisModule_Log(ctx, "warning",
								"Received invalid value '%s' as thread count argument",
								invalid_arg);

						return REDISMODULE_ERR;
					}
					// Emit notice but do not fail if the specified thread count is greater than the system's
					// number of cores (which is already set as the default value).
					if(v > config.thread_pool_size) {
						RedisModule_Log(ctx, "notice",
								"thread pool size: %lld greater then number of cores: %d.",
								v, config.thread_pool_size);
					}

					res = Config_Option_set(field, v);
					ASSERT(res == 1);

					// log thread pool size
					RedisModule_Log(ctx, "notice",
							"thread pool size is set to %lld.", v);
				}

				break;

			//------------------------------------------------------------------
			// result-set max size
			//------------------------------------------------------------------

			case Config_RESULTSET_MAX_SIZE:
				{
					if(!_Config_ParsePositiveInteger(val, &v)) {
						invalid_arg = RedisModule_StringPtrLen(val, NULL);

						RedisModule_Log(ctx, "warning",
								"Could not parse resultset size argument '%s' as an integer",
								invalid_arg);

						return REDISMODULE_ERR;
					}

					res = Config_Option_set(field, v);
					ASSERT(res == 1);

					// log the resultset size
					RedisModule_Log(ctx, "notice",
							"Resultset size is set to %lld.", v);
				}

				break;

			//------------------------------------------------------------------
			// maintain transpose matrices
			//------------------------------------------------------------------

			case Config_MAINTAIN_TRANSPOSE:
				{
					bool maintain_transpose;

					// Exit with error if argument was not "yes" or "no".
					if(!_Config_ParseYesNo(val, &maintain_transpose)) {
						invalid_arg = RedisModule_StringPtrLen(val, NULL);

						RedisModule_Log(ctx, "warning",
								"Invalid argument '%s' for maintain_transposed_matrices, expected 'yes' or 'no'",
								invalid_arg);

						return REDISMODULE_ERR;
					}

					res = Config_Option_set(field, v);
					ASSERT(res == 1);

					if(maintain_transpose)
						RedisModule_Log(ctx, "notice",
								"Maintaining transposed copies of relationship matrices.");
					else 
						RedisModule_Log(ctx, "notice",
								"Not maintaining transposed copies of relationship matrices.");
				}
				break;

			//------------------------------------------------------------------
			// virtual key element count
			//------------------------------------------------------------------

			case Config_VKEY_MAX_ENTITY_COUNT:
				{
					if(!_Config_ParsePositiveInteger(val, &v)) {
						invalid_arg = RedisModule_StringPtrLen(val, NULL);

						RedisModule_Log(ctx, "warning",
								"Could not parse virtual key size argument '%s' as an integer",
								invalid_arg);

						return REDISMODULE_ERR;
					}

					res = Config_Option_set(field, v);
					ASSERT(res == 1);

					// log the new entity threshold
					RedisModule_Log(ctx, "notice",
							"Max number of entities per graph meta key set to %lld.",
							v);
				}
				break;

			default:
				ASSERT("Unknown configuration field" && false);
				return REDISMODULE_ERR;
		}
	}

	return res;
}

int Config_Option_set(Config_Option_Field field, ...) {
	//--------------------------------------------------------------------------
	// set the option
	//--------------------------------------------------------------------------

	va_list ap;

	switch (field)
	{
		//----------------------------------------------------------------------
		// cache size
		//----------------------------------------------------------------------

		case Config_CACHE_SIZE:
			{
				va_start(ap, field);
				uint64_t cache_size = va_arg(ap, uint64_t);
				va_end(ap);

				// TODO: validate cache_size
				Config_cache_size_set(cache_size);
			}
			break;

			//----------------------------------------------------------------------
			// OpenMP thread count
			//----------------------------------------------------------------------

		case Config_OPENMP_NTHREAD:
			{
				va_start(ap, field);
				uint omp_nthreads = va_arg(ap, uint);
				va_end(ap);

				Config_OMP_thread_count_set(omp_nthreads);
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

				if(pool_nthreads == NULL) return 0;
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

				if(resultset_max_size == NULL) return 0;
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

				if(maintain_transpose == NULL) return 0;
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

				if(vkey_max_entity_count == NULL) return 0;
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

				if(async_delete == NULL) return 0;
				(*async_delete) = Config_async_delete_get();
			}
			break;

        //----------------------------------------------------------------------
        // invalid option
        //----------------------------------------------------------------------

        default : 
			ASSERT("invalid option field" && false);
            return 0;
    }

	return 1;
}

int Config_Option_get(Config_Option_Field field, ...) {

	//--------------------------------------------------------------------------
	// get the option
	//--------------------------------------------------------------------------

	va_list ap;

	switch (field)
	{
		//----------------------------------------------------------------------
		// cache size
		//----------------------------------------------------------------------

		case Config_CACHE_SIZE:
			{
				va_start(ap, field);
				uint64_t *cache_size = va_arg(ap, uint64_t*);
				va_end(ap);

				if(cache_size == NULL) return 0;
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

				if(omp_nthreads == NULL) return 0;
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

				if(pool_nthreads == NULL) return 0;
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

				if(resultset_max_size == NULL) return 0;
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

				if(maintain_transpose == NULL) return 0;
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

				if(vkey_max_entity_count == NULL) return 0;
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

				if(async_delete == NULL) return 0;
				(*async_delete) = Config_async_delete_get();
			}
			break;

        //----------------------------------------------------------------------
        // invalid option
        //----------------------------------------------------------------------

        default : 
			ASSERT("invalid option field" && false);
            return 0;
    }

	return 1;
}

