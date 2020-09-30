/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef REDISGRAPH_MODULE_VERSION

#define REDISGRAPH_VERSION_MAJOR 2
#define REDISGRAPH_VERSION_MINOR 2
#define REDISGRAPH_VERSION_PATCH 6

#define REDISGRAPH_SEMANTIC_VERSION(major, minor, patch) \
  (major * 10000 + minor * 100 + patch)

#define REDISGRAPH_MODULE_VERSION REDISGRAPH_SEMANTIC_VERSION(REDISGRAPH_VERSION_MAJOR, REDISGRAPH_VERSION_MINOR, REDISGRAPH_VERSION_PATCH)

#endif
