#include "rmalloc.h"

static size_t used_memory  = 0;
static size_t memory_limit = 0;

static void* (*Real_RedisModule_Alloc)(size_t bytes);
static void* (*Real_RedisModule_Realloc)(void *ptr, size_t bytes);
static void (*Real_RedisModule_Free)(void *ptr);
static void* (*Real_RedisModule_Calloc)(size_t nmemb, size_t size);
static char* (*Real_RedisModule_Strdup)(const char *str);

// out of memory callback
static void (*OOM_CB)();

#define ADD_USED_MEMORY(s) __atomic_add_fetch(&used_memory, s, __ATOMIC_RELAXED) 

#define SUB_MEMORY_CONSUMPTION(p)                               \
	({                                                          \
		size_t s = RedisModule_MallocSize(p);                   \
		__atomic_sub_fetch(&used_memory, s, __ATOMIC_RELAXED);  \
	})

/* Redefine the allocator functions to use the malloc family.
 * Only to be used when running module code from a non-Redis
 * context, such as unit tests. */
void Alloc_Reset() {
	RedisModule_Alloc    = malloc;
	RedisModule_Realloc  = realloc;
	RedisModule_Calloc   = calloc;
	RedisModule_Free     = free;
	RedisModule_Strdup   = strdup;
}

static void enforce_memory_limit(void *ptr) {
	// check for real allocation size >= n
    size_t s = RedisModule_MallocSize(ptr);

	// get current memory limit
    size_t limit = __atomic_load_n(&memory_limit, __ATOMIC_RELAXED);

	// call OOM callback if memory limit been reached
    if(ADD_USED_MEMORY(s) > limit && limit > 0) OOM_CB();
}

static void *protected_rm_malloc(size_t n) {
	// allocate requested memory
    void* ptr = Real_RedisModule_Alloc(n);

	enforce_memory_limit(ptr);

    return ptr;
}

static void *protected_rm_calloc(size_t nelem, size_t elemsz) {
	// allocate requested memory
    void* ptr = Real_RedisModule_Calloc(nelem, elemsz);

	enforce_memory_limit(ptr);

    return ptr;
}


static void *protected_rm_realloc(void *p, size_t n) {
	// subtract buffer size from used memory
	SUB_MEMORY_CONSUMPTION(p);

    void *ptr = Real_RedisModule_Realloc(p, n);

	enforce_memory_limit(ptr);

    return ptr;
}

static void protected_rm_free(void *p) {
	SUB_MEMORY_CONSUMPTION(p);
    Real_RedisModule_Free(p);
}

static char *protected_rm_strdup(const char *s) {
    size_t len = strlen(s);
    char *ret = (char *)protected_rm_malloc(len + 1);

    memcpy(ret, s, len);
    ret[len] = '\0';

    return ret;
}

// determine redis memory limit
static int update_memory_limit() {
    int res = REDISMODULE_OK;
	// TODO: use a global RM_GetDetachedThreadSafeContext 
	RedisModuleCtx *ctx = RM_GetThreadSafeContext(NULL);

    RedisModuleServerInfoData *infoCtx = RedisModule_GetServerInfo(ctx,
			"memory");
    size_t maxmemory = 2 * RedisModule_ServerInfoGetFieldUnsigned(infoCtx,
			"maxmemory", &res);
    RedisModule_FreeServerInfo(ctx, infoCtx);

    __atomic_store_n(&maxmemory, memory_limit, __ATOMIC_RELAXED);

	FreeThreadSafeContext(ctx);

    return res;
}

int rm_alloc_init(RedisModuleCtx* ctx, bool enforce_memory_limit,
		void (*cbOOM)()) {
	// save original redis module functions
	OOM_CB                    = NULL;
	Real_RedisModule_Free     = RedisModule_Free;
	Real_RedisModule_Alloc    = RedisModule_Alloc;
	Real_RedisModule_Calloc   = RedisModule_Calloc;
	Real_RedisModule_Strdup   = RedisModule_Strdup;
	Real_RedisModule_Realloc  = RedisModule_Realloc;

    if(useMemoryProtection) {
		// override redis module functions with protected version
		RedisModule_Free     = protected_rm_free;
		RedisModule_Alloc    = protected_rm_malloc;
		RedisModule_Calloc   = protected_rm_calloc;
		RedisModule_Strdup   = protected_rm_strdup;
		RedisModule_Realloc  = protected_rm_realloc;

        if(update_memory_limit() != REDISMODULE_OK) {
            RedisModule_Log(ctx, "warning", "Failed getting server max memory");
            return REDISMODULE_ERR;
        }

		// sync with redis memory limit every 10 seconds
		Cron_AddTask(10 * 1000, update_memory_limit, NULL);

        if(RedisModule_SubscribeToServerEvent(ctx, RedisModuleEvent_CronLoop,
					update_memory_limit_cron_job) != REDISMODULE_OK) {
            RedisModule_Log(ctx, "warning", "Failed registering memory protection cron job");
            return REDISMODULE_ERR;
        }

		if(!cbOOM) OOM_CB = cbOOM;
    }

    return REDISMODULE_OK;
}

