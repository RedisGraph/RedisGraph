/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "map_funcs.h"
#include "RG.h"
#include "../func_desc.h"
#include "../../errors.h"
#include "../../util/arr.h"
#include "../../datatypes/map.h"
#include "../../datatypes/array.h"
#include "../../graph/entities/graph_entity.h"

SIValue AR_TOMAP(SIValue *argv, int argc, void *private_data) {
	/* create a new SIMap object
	 * expecting an even number of arguments
	 * argv[even] = key
	 * argv[odd] = value */

	// validate number of arguments
	if(argc % 2 != 0) {
		ErrorCtx_RaiseRuntimeException("map expects even number of elements");
	}

	SIValue map = SI_Map(argc / 2);

	for(int i = 0; i < argc; i += 2) {
		SIValue key = argv[i];
		SIValue val = argv[i + 1];

		// make sure key is a string
		if(!(SI_TYPE(key) & T_STRING)) {
			Error_SITypeMismatch(key, T_STRING);
			break;
		}

		Map_Add(&map, key, val);
	}

	return map;
}

SIValue AR_KEYS(SIValue *argv, int argc, void *private_data) {
	ASSERT(argc == 1);
	switch(SI_TYPE(argv[0])) {
		case T_NULL:
			return SI_NullVal();
		case T_NODE:
		case T_EDGE:
			return GraphEntity_Keys(argv[0].ptrval);
		case T_MAP:
			return Map_Keys(argv[0]);
		default:
			ASSERT(false);
	}
	return SI_NullVal();
}

SIValue AR_PROPERTIES(SIValue *argv, int argc, void *private_data) {
	ASSERT(argc == 1);
	switch(SI_TYPE(argv[0])) {
		case T_NULL:
			return SI_NullVal();
		case T_NODE:
		case T_EDGE:
			return GraphEntity_Properties(argv[0].ptrval);
		case T_MAP:
			return argv[0];
		default:
			ASSERT(false);
	}
	return SI_NullVal();
}

// Receives two maps and returns a new map adding them in a single map
SIValue AR_ADDMAP(SIValue *argv, int argc, void *private_data) {
	ASSERT(argc == 2);

	SIValue map0 = argv[0];
	SIValue map1 = argv[1];

	if ((SI_TYPE(map0) & T_NULL) && (SI_TYPE(map1) & T_NULL)) {
		return SI_NullVal();
	} else if (SI_TYPE(map0) & T_NULL) {
		return map1;
	} else if (SI_TYPE(map1) & T_NULL) {
		return map0;
	} else {
		uint keyCount0 = Map_KeyCount(map0);
		uint keyCount1 = Map_KeyCount(map1);

		if(keyCount0 > keyCount1) {
			for(int i=0; i < keyCount1; i++) {
				Pair p = map1.map[i];
				Map_Add(&map0, p.key, p.val);
			}
			return map0;
		} else {
			for(int i=0; i < keyCount0; i++) {
				Pair p = map0.map[i];
				Map_Add(&map1, p.key, p.val);
			}
			return map1;
		}
	}
}

void Register_MapFuncs() {
	SIType *types;
	SIType ret_type;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 1);
	array_append(types, SI_ALL);
	ret_type = T_MAP;
	func_desc = AR_FuncDescNew("tomap", AR_TOMAP, 0, VAR_ARG_LEN, types, ret_type, true, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, T_NULL | T_MAP | T_NODE | T_EDGE);
	ret_type = T_NULL | T_ARRAY;
	func_desc = AR_FuncDescNew("keys", AR_KEYS, 1, 1, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, T_NULL | T_MAP | T_NODE | T_EDGE);
	ret_type = T_NULL | T_MAP;
	func_desc = AR_FuncDescNew("properties", AR_PROPERTIES, 1, 1, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	array_append(types, T_NULL | T_MAP);
	array_append(types, T_NULL | T_MAP);
	ret_type = T_NULL | T_MAP;
	func_desc = AR_FuncDescNew("addmap", AR_ADDMAP, 2, 2, types, ret_type, true, true);
	AR_RegFunc(func_desc);
}

