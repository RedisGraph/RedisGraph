/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "agg_funcs.h"
#include "../../logic/functions/map_funcs/map_funcs.h"
#include "../../logic/functions/list_funcs/list_funcs.h"
#include "../../logic/functions/time_funcs/time_funcs.h"
#include "../../logic/functions/point_funcs/point_funcs.h"
#include "../../logic/functions/entity_funcs/entity_funcs.h"
#include "../../logic/functions/string_funcs/string_funcs.h"
#include "../../logic/functions/boolean_funcs/boolean_funcs.h"
#include "../../logic/functions/numeric_funcs/numeric_funcs.h"
#include "../../logic/functions/conditional_funcs/conditional_funcs.h"
#include "../../logic/functions/comprehension_funcs/comprehension_funcs.h"
#include "../../logic/functions/path_funcs/path_funcs.h"
#include "../../logic/functions/placeholder_funcs/placeholder_funcs.h"

/* Registers all arithmetic functions. */
void AR_RegisterFuncs();

