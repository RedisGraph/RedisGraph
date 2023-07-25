/*
 * Copyright FalkorDB Ltd. 2023 - present
 * Licensed under the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "../graph/entities/attribute_set.h"

#define INDEX_FIELD_NONE_INDEXED "NONE_INDEXABLE_FIELDS"
#define INDEX_FIELD_DEFAULT_WEIGHT 1.0
#define INDEX_FIELD_DEFAULT_NOSTEM false
#define INDEX_FIELD_DEFAULT_PHONETIC "no"

// type of index field
// multiple types can be combined via bitwise OR
typedef enum {
	INDEX_FLD_FULLTEXT = 0x01,  // full text field
	INDEX_FLD_NUMERIC  = 0x02,  // numeric field
	INDEX_FLD_GEO      = 0x04,  // geo field
	INDEX_FLD_STR      = 0x08,  // string field
	INDEX_FLD_VECTOR   = 0x10,  // vector field
} IndexFieldType;

typedef struct {
	char *name;              // field name
	Attribute_ID id;         // field id
	IndexFieldType type;     // field type(s)
	struct {
		double weight;       // the importance of text
		bool nostem;         // disable stemming of the text
		char *phonetic;      // phonetic search of text
		uint32_t dimension;  // vector dimension
	} options;
} IndexField;

//------------------------------------------------------------------------------
// index field creation
//------------------------------------------------------------------------------

// create a new exact match index field
void IndexField_NewExactMatchField
(
	IndexField *field,   // field to initialize
	const char *name,    // field name
	Attribute_ID id      // field id
);

// create a new full text index field
void IndexField_NewFullTextField
(
	IndexField *field,   // field to initialize
	const char *name,    // field name
	Attribute_ID id      // field id
);

// create a new vector index field
void IndexField_NewVectorField
(
	IndexField *field,   // field to initialize
	const char *name,    // field name
	Attribute_ID id,     // field id
	uint32_t dimension   // vector dimension
);

// clone index field
void IndexField_Clone
(
	const IndexField *src,  // field to clone
	IndexField *dest        // cloned field
);

//------------------------------------------------------------------------------
// index field options
//------------------------------------------------------------------------------

// set index field weight
void IndexField_SetWeight
(
	IndexField *field,  // field to update
	double weight       // new weight
);

// set index field stemming
void IndexField_SetStemming
(
	IndexField *field,  // field to update
	bool nostem         // enable/disable stemming
);

// set index field phonetic
void IndexField_SetPhonetic
(
	IndexField *field,    // field to update
	const char *phonetic  // phonetic
);

// set index field vector dimension
void IndexField_SetDimension
(
	IndexField *field,  // field to update
	uint32_t dimension  // vector dimension
);

// free index field
void IndexField_Free
(
	IndexField *field  // index field to be freed
);
