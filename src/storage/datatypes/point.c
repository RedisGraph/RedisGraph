/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "point.h"

float Point_lat(SIValue point) {
	ASSERT(SI_TYPE(point) == T_POINT);

	return point.point.latitude;
}

float Point_lon(SIValue point) {
	ASSERT(SI_TYPE(point) == T_POINT);

	return point.point.longitude;
}

