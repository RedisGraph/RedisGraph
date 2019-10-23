/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "list_funcs/list_funcs.h"
#include "time_funcs/time_funcs.h"
#include "entity_funcs/entity_funcs.h"
#include "string_funcs/string_funcs.h"
#include "boolean_funcs/boolean_funcs.h"
#include "numeric_funcs/numeric_funcs.h"
#include "conditional_funcs/conditional_funcs.h"
#include "path_funcs/path_funcs.h"

/* Registers all arithmetic functions. */
void AR_RegisterFuncs();

