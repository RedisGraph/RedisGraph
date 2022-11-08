/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../../value.h"

void Register_BooleanFuncs();

// tries to convert input to boolean
SIValue AR_TO_BOOLEAN
(
    SIValue *argv,      // arguments
    int argc,           // number of arguments
    void *private_data  // private context
);
