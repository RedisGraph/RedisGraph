/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "RG.h"
#include "../func_desc.h"
#include "../../errors.h"
#include "../../util/arr.h"
#include "../../datatypes/map.h"

SIValue AR_TOPOINT(SIValue *argv, int argc) {
	SIValue map = argv[0];
	SIType t = SI_TYPE(map);

	if(t == T_NULL) return SI_NullVal();

	// expecting input to be a map
	// point({latitude: 32.0705767, longitude: 34.8185946})
	ASSERT(t == T_MAP);

	uint key_count = Map_KeyCount(map);
	if(key_count != 2) {
		// TODO: error
	}

	bool key_exists;
	SIValue latitude;
	SIValue longitude;

	if(!Map_Get(map, SI_ConstStringVal("latitude"), &latitude)) {
		// TODO: error
	}
	if(!Map_Get(map, SI_ConstStringVal("longitude"), &longitude)) {
		// TODO: error
	}

	if(SI_TYPE(latitude) != T_DOUBLE || SI_TYPE(longitude) != T_DOUBLE) {
		// TODO: error
	}

	float lat = latitude.doubleval;
	float lon = longitude.doubleval;
	return SI_Point(lat, lon);
}

void Register_PointFuncs() {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 1);
	types = array_append(types, T_NULL | T_MAP);
	func_desc = AR_FuncDescNew("point", AR_TOPOINT, 1, 1, types, true, false);
	AR_RegFunc(func_desc);
}

