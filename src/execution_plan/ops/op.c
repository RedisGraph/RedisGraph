/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op.h"

void OpBase_Free(OpBase *op) {
    // Free internal operation
    op->free(op);
    if(op->children) free(op->children);
    free(op);
}
