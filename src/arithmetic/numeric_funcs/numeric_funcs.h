/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../value.h"

void Register_NumericFuncs();

// tries to convert input to float
SIValue AR_TOFLOAT
(
    SIValue *argv,      // arguments
    int argc,           // number of arguments
    void *private_data  // private context
);

// tries to convert input to integer
SIValue AR_TOINTEGER
(
    SIValue *argv,      // arguments
    int argc,           // number of arguments
    void *private_data  // private context
);
