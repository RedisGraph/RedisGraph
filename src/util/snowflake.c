/*
Copyright (c) 2014 Dwayn Matthies <dwayn dot matthies at gmail dot com>

Minor adjustments by Eran Sandler <eran at sandler dot co dot il>
to be used in redissnowflake project

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "stats.h"
#include "snowflake.h"
#include <sys/time.h>
#include <stdio.h>

long int snowflake_id() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int millisecs = tp.tv_sec * 1000 + tp.tv_usec / 1000 - SNOWFLAKE_EPOCH;
    long int id = 0L;

    // Catch NTP clock adjustment that rolls time backwards and sequence number overflow
    if ((snowflake_global_state.seq > snowflake_global_state.seq_max ) || snowflake_global_state.time > millisecs) {
        ++app_stats.waits;
        while (snowflake_global_state.time >= millisecs) {
            gettimeofday(&tp, NULL);
            millisecs = tp.tv_sec * 1000 + tp.tv_usec / 1000 - SNOWFLAKE_EPOCH;
        }
    }

    if (snowflake_global_state.time < millisecs) {
        snowflake_global_state.time = millisecs;
        snowflake_global_state.seq = 0L;
    }


    id = (millisecs << snowflake_global_state.time_shift_bits)
            | (snowflake_global_state.region_id << snowflake_global_state.region_shift_bits)
            | (snowflake_global_state.worker_id << snowflake_global_state.worker_shift_bits)
            | (snowflake_global_state.seq++);

    if (app_stats.seq_max < snowflake_global_state.seq)
        app_stats.seq_max = snowflake_global_state.seq;

    ++app_stats.ids;
    return id;
}

int snowflake_init(int region_id, int worker_id) {
    int max_region_id = (1 << SNOWFLAKE_REGIONID_BITS) - 1;
    if(region_id < 0 || region_id > max_region_id){
        //printf("Region ID must be in the range : 0-%d\n", max_region_id);
        return -1;
    }
    int max_worker_id = (1 << SNOWFLAKE_WORKERID_BITS) - 1;
    if(worker_id < 0 || worker_id > max_worker_id){
        //printf("Worker ID must be in the range: 0-%d\n", max_worker_id);
        return -1;
    }

    snowflake_global_state.time_shift_bits   = SNOWFLAKE_REGIONID_BITS + SNOWFLAKE_WORKERID_BITS + SNOWFLAKE_SEQUENCE_BITS;
    snowflake_global_state.region_shift_bits = SNOWFLAKE_WORKERID_BITS + SNOWFLAKE_SEQUENCE_BITS;
    snowflake_global_state.worker_shift_bits = SNOWFLAKE_SEQUENCE_BITS;

    snowflake_global_state.worker_id    = worker_id;
    snowflake_global_state.region_id    = region_id;
    snowflake_global_state.seq_max      = (1L << SNOWFLAKE_SEQUENCE_BITS) - 1;
    snowflake_global_state.seq          = 0L;
    snowflake_global_state.time         = 0L;
    app_stats.seq_cap                   = snowflake_global_state.seq_max;
    app_stats.waits                     = 0L;
    app_stats.seq_max                   = 0L;
    app_stats.ids                       = 0L;
    app_stats.region_id                 = region_id;
    app_stats.worker_id                 = worker_id;
    return 1;
}