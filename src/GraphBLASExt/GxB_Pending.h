/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Apache License, Version 2.0,
 * modified with the Commons Clause restriction.
 */

#ifndef __GXB_PENDING_H__
#define __GXB_PENDING_H__

#include <stdbool.h>
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"

/* Returns true if M has pending updates that would make
 * operations like matrix multiplication not thread-safe. */
bool GxB_Matrix_Pending
(
    GrB_Matrix M
) ;

#endif
