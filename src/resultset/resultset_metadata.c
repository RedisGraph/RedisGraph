/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "resultset.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../datatypes/map.h"
#include "../datatypes/array.h"

/* Prepare metadata of the form:
 * {
 *   "version", VERSION,
 *   "labels", [[VALUE_STRING, "label_1"] ... ],
 *   "relationship types ", [[VALUE_STRING, "reltype_1"] ... ],
 *   "property keys", [[VALUE_STRING, "prop_1"] ... ]
 * }
 */
SIValue ResultSet_PrepareMetadata(GraphContext *gc) {
	// Instantiate a new map.
	SIValue map = Map_New(4);

	SIValue version_key = SI_ConstStringVal("version");
	SIValue version = SI_LongVal(GraphContext_GetVersion(gc));
	Map_Add(&map, version_key, version);

	SIValue label_key = SI_ConstStringVal("labels");
	uint label_count = array_len(gc->node_schemas);
	SIValue labels = SIArray_New(label_count);
	for(uint i = 0; i < label_count; i ++) {
		char *label = gc->node_schemas[i]->name;
		SIArray_Append(&labels, SI_ConstStringVal(label));
	}
	Map_Add(&map, label_key, labels);

	SIValue reltype_key = SI_ConstStringVal("relationship types");
	uint reltype_count = array_len(gc->relation_schemas);
	SIValue reltypes = SIArray_New(reltype_count);
	for(uint i = 0; i < reltype_count; i ++) {
		char *reltype = gc->relation_schemas[i]->name;
		SIArray_Append(&reltypes, SI_ConstStringVal(reltype));
	}
	Map_Add(&map, reltype_key, reltypes);

	SIValue prop_key = SI_ConstStringVal("property keys");
	uint prop_count = array_len(gc->string_mapping);
	SIValue props = SIArray_New(prop_count);
	for(uint i = 0; i < prop_count; i ++) {
		char *prop = gc->string_mapping[i];
		SIArray_Append(&props, SI_ConstStringVal(prop));
	}
	Map_Add(&map, prop_key, props);

	return map;
}

