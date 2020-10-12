/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "conditional_funcs.h"
#include "../func_desc.h"
#include "../../util/arr.h"

/* Case When
 * Case Value [When Option i Then Result i] Else Default end */
SIValue AR_CASEWHEN(SIValue *argv, int argc) {
	int alternatives = argc - 1;
	SIValue d = argv[argc - 1];

	if((argc % 2) == 0) {
		/* Simple form:
		 * argv[0] - Value
		 * argv[i] - Option i
		 * argv[i+1] - Result i
		 * argv[argc-1] - Default
		 *
		 * Evaluate alternatives in order, return first alternatives which
		 * is equals to Value. */
		SIValue v = argv[0];
		for(int i = 1; i < alternatives; i += 2) {
			SIValue a = argv[i];
			int disjointOrNull;
			if((SIValue_Compare(v, a, &disjointOrNull) == 0) && (disjointOrNull != COMPARED_NULL)) {
				// Return Result i.
				return argv[i + 1];
			}
		}
	} else {
		/* Generic form:
		 * argv[i] - Option i
		 * argv[i+1] - Result i
		 * arg[argc-1] - Default
		 *
		 * Evaluate alternatives in order, return first alternatives which
		 * is not NULL or false. */
		for(int i = 0; i < alternatives; i += 2) {
			SIValue a = argv[i];
			// Skip NULL and false options.
			if(SIValue_IsNull(a) || ((SI_TYPE(a) & T_BOOL) && SIValue_IsFalse(a))) continue;
			// The option was truthy, return the associated value.
			return argv[i + 1];
		}
	}

	//Did not match against any Option return default.
	return d;
}

// Coalesce - return the first value which is not null. Defaults to null.
SIValue AR_COALESCE(SIValue *argv, int argc) {
	for(int i = 0; i < argc; i++)
		if(!SIValue_IsNull(argv[i])) {
			/* Avoid double free, since the value is propagated and will be free twice:
			 * 1. Argument array free.
			 * 2. Record free. */
			SIValue copy = argv[i];
			SIValue_MakeVolatile(argv + i);
			return copy;
		}
	return SI_NullVal();
}

void Register_ConditionalFuncs() {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 1);
	types = array_append(types, SI_ALL);
	func_desc = AR_FuncDescNew("case", AR_CASEWHEN, 2, VAR_ARG_LEN, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, SI_ALL);
	func_desc = AR_FuncDescNew("coalesce", AR_COALESCE, 1, VAR_ARG_LEN, types, true);
	AR_RegFunc(func_desc);
}

