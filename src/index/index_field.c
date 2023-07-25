/*
 * Copyright FalkorDB Ltd. 2023 - present
 * Licensed under the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "index_field.h"
#include "../util/rmalloc.h"

//------------------------------------------------------------------------------
// index field creation
//------------------------------------------------------------------------------

static void IndexField_New
(
	IndexField *field,   // field to initialize
	const char *name,    // field name
	Attribute_ID id,     // attribute ID
	IndexFieldType type  // field type
) {
	ASSERT(name     != NULL);
	ASSERT(field    != NULL);

	// clear field
	memset(field, 0, sizeof(IndexField));

	field->id   = id;
	field->name = rm_strdup(name);
	field->type = type;
}

// create a new exact match index field
void IndexField_NewExactMatchField
(
	IndexField *field,   // field to initialize
	const char *name,    // field name
	Attribute_ID id      // field id
) {
	IndexFieldType t = INDEX_FLD_NUMERIC | INDEX_FLD_STR | INDEX_FLD_GEO;
	IndexField_New(field, name, id, t);
}

// create a new full text index field
void IndexField_NewFullTextField
(
	IndexField *field,   // field to initialize
	const char *name,    // field name
	Attribute_ID id      // field id
) {
	IndexFieldType t = INDEX_FLD_FULLTEXT;
	IndexField_New(field, name, id, t);

	IndexField_SetWeight(field,   INDEX_FIELD_DEFAULT_WEIGHT);
	IndexField_SetStemming(field, INDEX_FIELD_DEFAULT_NOSTEM);
	IndexField_SetPhonetic(field, INDEX_FIELD_DEFAULT_PHONETIC);
}

// create a new vector index field
void IndexField_NewVectorField
(
	IndexField *field,   // field to initialize
	const char *name,    // field name
	Attribute_ID id,     // field id
	uint32_t dimension   // vector dimension
) {
	IndexField_New(field, name, id, INDEX_FLD_VECTOR);
	IndexField_SetDimension(field, dimension);
}

// clone index field
void IndexField_Clone
(
	const IndexField *src,  // field to clone
	IndexField *dest        // cloned field
) {
	ASSERT(src  != NULL);
	ASSERT(dest != NULL);

	memcpy(dest, src, sizeof(IndexField));

	dest->name = rm_strdup(src->name);

	if(src->options.phonetic != NULL) {
		dest->options.phonetic = rm_strdup(src->options.phonetic);
	}
}

//------------------------------------------------------------------------------
// index field options
//------------------------------------------------------------------------------

// set index field weight
void IndexField_SetWeight
(
	IndexField *field,  // field to update
	double weight       // new weight
) {
	ASSERT(field != NULL);
	field->options.weight = weight;
}

// set index field stemming
void IndexField_SetStemming
(
	IndexField *field,  // field to update
	bool nostem         // enable/disable stemming
) {
	ASSERT(field != NULL);
	field->options.nostem = nostem;
}

// set index field phonetic
void IndexField_SetPhonetic
(
	IndexField *field,    // field to update
	const char *phonetic  // phonetic
) {
	ASSERT(field    != NULL);
	ASSERT(phonetic != NULL);

	field->options.phonetic = rm_strdup(phonetic);
}

// set index field vector dimension
void IndexField_SetDimension
(
	IndexField *field,  // field to update
	uint32_t dimension  // vector dimension
) {
	ASSERT(field != NULL);
	field->options.dimension = dimension;
}

// free index field
void IndexField_Free
(
	IndexField *field
) {
	ASSERT(field != NULL);

	rm_free(field->name);

	if(field->options.phonetic != NULL) {
		rm_free(field->options.phonetic);
	}
}

