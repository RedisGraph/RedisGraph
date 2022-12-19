/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

// initialize the mapping between ast-node-types and 
// visiting functions for the ast-visitor
bool AST_ValidationsMappingInit();

// free a mapping
void AST_ValidationsMappingFree();
