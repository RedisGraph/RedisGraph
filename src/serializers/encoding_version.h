/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

/*
 * Date: March 6th 2020.
 * As of Redis 6, Redis Server supports server events which allow creating and deleting keys upon RDB load and save, start and finish events.
 * As a result we generate multiple virtual keys during RDB saving start event and delete them one the loading is done. This done to reduce memory
 * overhead during Replica-Of procedure.
 * As a result of such encodeing we are left with two encoding systems. One for Redis 5 which saves a single RDB file, and one for Redis 6 and above which
 * splits the RDB file.
 * From now until further notice, all ODD encoding versions with value >= 7 are for Redis 6 and above. All EVEN encoding encoding versions with value >=7 are for Redis 5.
 *
 * Formally, with encoding version as ENCVER:
 *
 * ENCVER % 2 == 0 && ENCVER >= 6 // Redis 5 encoding - server events not supported.
 * ENCVER % 2 == 1 && ENCVER >= 7 // Redis 6 encoding - server events supported.
 *
 * Every modification will need to increase both versions by 2.
 * */

#define GRAPHCONTEXT_TYPE_ENCODING_VERSION_LATEST 7 // Latest RDB encoding version.

#define GRAPHCONTEXT_TYPE_ENCODING_VERSION_WITH_SERVER_EVENTS 7 // Current RDB encoding version for redis server with server events
#define GRAPHCONTEXT_TYPE_ENCODING_VERSION_WITHOUT_SERVER_EVENTS 6 // Current RDB encoding version for redis server without server events

#define PREV_DECODER_SUPPORT_MAX_V 5 // Highst version that has backwards-compatibility decoding routines.
#define PREV_DECODER_SUPPORT_MIN_V 4 // Lowest version that has backwards-compatibility decoding routines.
