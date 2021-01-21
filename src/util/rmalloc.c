#include "rmalloc.h"

static size_t max_memory = 0;
static size_t used_memory = 0;

static void* (*Real_RedisModule_Alloc)(size_t bytes);
static void* (*Real_RedisModule_Realloc)(void *ptr, size_t bytes);
static void (*Real_RedisModule_Free)(void *ptr);
static void* (*Real_RedisModule_Calloc)(size_t nmemb, size_t size);
static char* (*Real_RedisModule_Strdup)(const char *str);

static void (*OnMaxMemoryReached)();

/* Redefine the allocator functions to use the malloc family.
 * Only to be used when running module code from a non-Redis
 * context, such as unit tests. */
void Alloc_Reset() {
  RedisModule_Alloc = malloc;
  RedisModule_Realloc = realloc;
  RedisModule_Calloc = calloc;
  RedisModule_Free = free;
  RedisModule_Strdup = strdup;
}

static void max_memory_reached() {
    RedisModule_Log(NULL, "warning", "max memory reached, crashing, bye bye :)");
    *((int*)NULL) = 0; //crashing!!!
}

static void *protected_rm_malloc(size_t n) {
    void* ptr = Real_RedisModule_Alloc(n);
    size_t s = RedisModule_MallocSize(ptr);
    size_t curr_max_memory = __atomic_load_n(&max_memory, __ATOMIC_RELAXED);
    if(__atomic_add_fetch(&used_memory, s, __ATOMIC_RELAXED) > curr_max_memory && curr_max_memory){
        OnMaxMemoryReached();
    }
    return ptr;
}

static void *protected_rm_calloc(size_t nelem, size_t elemsz) {
    void* ptr = Real_RedisModule_Calloc(nelem, elemsz);
    size_t s = RedisModule_MallocSize(ptr);
    size_t curr_max_memory = __atomic_load_n(&max_memory, __ATOMIC_RELAXED);
    if(__atomic_add_fetch(&used_memory, s, __ATOMIC_RELAXED) > curr_max_memory && curr_max_memory){
        OnMaxMemoryReached();
    }
    return ptr;
}

static void *protected_rm_realloc(void *p, size_t n) {
    size_t s = RedisModule_MallocSize(p);
    __atomic_sub_fetch(&used_memory, s, __ATOMIC_RELAXED);

    void* ptr = Real_RedisModule_Realloc(p, n);

    s = RedisModule_MallocSize(ptr);
    size_t curr_max_memory = __atomic_load_n(&max_memory, __ATOMIC_RELAXED);
    if(__atomic_add_fetch(&used_memory, s, __ATOMIC_RELAXED) > curr_max_memory && curr_max_memory){
        OnMaxMemoryReached();
    }

    return ptr;
}

static void protected_rm_free(void *p) {
    size_t s = RedisModule_MallocSize(p);
    __atomic_sub_fetch(&used_memory, s, __ATOMIC_RELAXED);
    Real_RedisModule_Free(p);
}

static char *protected_rm_strdup(const char *s) {
    size_t len = strlen(s);
    char *ret = (char *)rm_malloc(len + 1);
    memcpy(ret, s, len);
    ret[len] = '\0';
    return ret;
}

static int memory_report(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    size_t curr_max_memory = __atomic_load_n(&max_memory, __ATOMIC_RELAXED);
    size_t curr_used_memory = __atomic_load_n(&used_memory, __ATOMIC_RELAXED);
    RedisModule_ReplyWithArray(ctx, 4);
    RedisModule_ReplyWithCString(ctx, "used_memory");
    RedisModule_ReplyWithLongLong(ctx, curr_used_memory);
    RedisModule_ReplyWithCString(ctx, "max_memory");
    RedisModule_ReplyWithLongLong(ctx, curr_max_memory);
    return REDISMODULE_OK;
}

static int update_max_memory(RedisModuleCtx* ctx){
    int out_err = REDISMODULE_OK;
    RedisModuleServerInfoData *infoCtx = RedisModule_GetServerInfo(ctx, "memory");
    size_t curr_max_memory = 2 * RedisModule_ServerInfoGetFieldUnsigned(infoCtx, "maxmemory", &out_err);
    __atomic_store_n(&max_memory, curr_max_memory, __ATOMIC_RELAXED);
    RedisModule_FreeServerInfo(ctx, infoCtx);
    return out_err;
}

static void update_max_memory_cron_job(RedisModuleCtx *ctx, RedisModuleEvent eid, uint64_t subevent, void *data){
    update_max_memory(ctx);
}

int rm_alloc_initialize(RedisModuleCtx* ctx, int useMemoryProtection, void (*OnMaxMemoryReached)()) {
    Real_RedisModule_Alloc = RedisModule_Alloc;
    Real_RedisModule_Realloc = RedisModule_Realloc;
    Real_RedisModule_Free = RedisModule_Free;
    Real_RedisModule_Calloc = RedisModule_Calloc;
    Real_RedisModule_Strdup = RedisModule_Strdup;

    if(useMemoryProtection){
        RedisModule_Alloc = protected_rm_malloc;
        RedisModule_Realloc = protected_rm_realloc;
        RedisModule_Calloc = protected_rm_calloc;
        RedisModule_Free = protected_rm_free;
        RedisModule_Strdup = protected_rm_strdup;
        if(update_max_memory(ctx) != REDISMODULE_OK){
            RedisModule_Log(ctx, "warning", "Failed getting max memory from server info");
            return REDISMODULE_ERR;
        }

        if(RedisModule_SubscribeToServerEvent(ctx, RedisModuleEvent_CronLoop, update_max_memory_cron_job) != REDISMODULE_OK){
            RedisModule_Log(ctx, "warning", "Failed register to server cron job for memory protection");
            return REDISMODULE_ERR;
        }
    }

    if(!OnMaxMemoryReached){
        OnMaxMemoryReached = max_memory_reached;
    }

    if(RedisModule_CreateCommand(ctx, "graph.memoryreport", memory_report, "readonly", 0, 0,
                                 0) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }


    return REDISMODULE_OK;
}
