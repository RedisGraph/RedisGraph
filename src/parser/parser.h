/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../deps/libcypher-parser/lib/src/cypher-parser.h"

cypher_parse_result_t *parse(const char *query);
