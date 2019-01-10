/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef NEW_AST_H
#define NEW_AST_H

#include "../../deps/libcypher-parser/lib/src/cypher-parser.h"

// Checks if AST represent a read only query.
bool NEWAST_ReadOnly(const cypher_parse_result_t *ast);

#endif
