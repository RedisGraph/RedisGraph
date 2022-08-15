/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../value.h"

// returns latitude of given point
float Point_lat(SIValue point);

// returns longitude of given point
float Point_lon(SIValue point);

// returns a coordinate (latitude or longitude) of a given point
SIValue Point_GetCoordinate(SIValue point, SIValue key);

