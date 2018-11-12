/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef REDISGRAPH_MODULE_VERSION

#define REDISGRAPH_VERSION_MAJOR 1
#define REDISGRAPH_VERSION_MINOR 0
#define REDISGRAPH_VERSION_PATCH 0

#define REDISGRAPH_SEMANTIC_VERSION(major, minor, patch) \
  (major * 10000 + minor * 100 + patch)

#define REDISGRAPH_MODULE_VERSION REDISGRAPH_SEMANTIC_VERSION(REDISGRAPH_VERSION_MAJOR, REDISGRAPH_VERSION_MINOR, REDISGRAPH_VERSION_PATCH)

#endif
