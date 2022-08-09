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

SIValue Point_GetCoordinate(SIValue point, SIValue key) {
	ASSERT(SI_TYPE(point) == T_POINT);
	ASSERT(SI_TYPE(key) == T_STRING);

	if(strcmp(key.stringval, "latitude")==0) {
		return SI_DoubleVal(Point_lat(point));
	} else if(strcmp(key.stringval, "longitude")==0) {
		return SI_DoubleVal(Point_lon(point));
	} else {
		return SI_NullVal();
	}

}

