/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "reconf_handler.h"
#include "util/thpool/pools.h"
#include "util/rmalloc.h"

// handler function which being called when config param changed
void reconf_handler(Config_Option_Field type) {
	switch (type)
	{
		//----------------------------------------------------------------------
		// max queued queries
		//----------------------------------------------------------------------

		case Config_MAX_QUEUED_QUERIES:
			{
				uint64_t max_queued_queries;
				ASSERT(Config_Option_get(type, &max_queued_queries));
				ThreadPools_Set_max_pending_work(max_queued_queries);
			}
			break;
		
		//----------------------------------------------------------------------
		// query mem capacity
		//----------------------------------------------------------------------

		case Config_QUERY_MEM_CAPACITY:
			{
				int64_t query_mem_capacity;
				ASSERT(Config_Option_get(type, &query_mem_capacity));
				rm_set_mem_capacity(query_mem_capacity);
			}
			break;

        //----------------------------------------------------------------------
        // all other options
        //----------------------------------------------------------------------
        default : 
			return;
    }
}
