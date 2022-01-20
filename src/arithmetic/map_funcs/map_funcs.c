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
#include "../../graph/entities/graph_entity.h"

SIValue AR_TOMAP(SIValue *argv, int argc) {
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

SIValue AR_KEYS(SIValue *argv, int argc) {
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

void Register_MapFuncs() {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 1);
	array_append(types, SI_ALL);
	func_desc = AR_FuncDescNew("tomap", AR_TOMAP, 0, VAR_ARG_LEN, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, T_NULL | T_MAP | T_NODE | T_EDGE);
	func_desc = AR_FuncDescNew("keys", AR_KEYS, 1, 1, types, true, false);
	AR_RegFunc(func_desc);
}

