/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../value.h"
/* This function register place holders as functions, for later execution plan modifications,
 * once a dedicated method encounters this place holder. */
void Register_PlaceholderFuncs();
