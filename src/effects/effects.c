/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "effects.h"
#include "../query_ctx.h"

// determine block available space 
#define BLOCK_AVAILABLE_SPACE(b) (b->cap - BLOCK_USED_SPACE(b))

// determine how many bytes been written to buffer
#define BLOCK_USED_SPACE(b) (b->offset - b->buffer)

// linked list of EffectsBufferblocks
struct EffectsBufferBlock {
	size_t cap;                       // block capacity
	unsigned char *offset;            // buffer offset
	struct EffectsBufferBlock *next;  // pointer to next buffer
	unsigned char buffer[];           // buffer
};

// effects buffer is a linked-list of buffers
struct _EffectsBuffer {
	size_t block_size;                   // block size
	struct EffectsBufferBlock *head;     // first block
	struct EffectsBufferBlock *current;  // current block
	uint64_t n;                          // number of effects in buffer
};

// forward declarations
static void EffectsBuffer_WriteSIArray
(
	const SIValue *arr,  // array
	EffectsBuffer *buff  // effect buffer
);

// create a new effects-buffer block
static struct EffectsBufferBlock *EffectsBufferBlock_New
(
	size_t n  // size of block
) {
	size_t _n = sizeof(struct EffectsBufferBlock) + n;
	struct EffectsBufferBlock *b = rm_malloc(_n);

	b->cap    = n;
	b->next   = NULL;
	b->offset = b->buffer;

	return b;
}

// add a new block to effects-buffer
static void EffectsBuffer_AddBlock
(
	EffectsBuffer *eb  // effects-buffer
) {
	// create a new block and link
	struct EffectsBufferBlock *b = EffectsBufferBlock_New(eb->block_size);
	eb->current->next = b;
	eb->current       = b;
}

// write n bytes from ptr into block
// returns actual number of bytes written
// if buffer isn't large enough only a portion of the bytes will be written
static size_t EffectsBufferBlock_WriteBytes
(
	const void *ptr,              // data to write
	size_t n,                     // number of bytes to write
	struct EffectsBufferBlock *b  // block to write to
) {
	// validations
	ASSERT(n   > 0);
	ASSERT(b   != NULL);
	ASSERT(ptr != NULL);

	// determine number of bytes we can write
	n = MIN(n, BLOCK_AVAILABLE_SPACE(b));

	// write n bytes to buffer
	memcpy(b->offset, ptr, n);

	// update offset
	b->offset += n;

	return n;
}

// write n bytes from ptr into effects-buffer
static void EffectsBuffer_WriteBytes
(
	const void *ptr,   // data to write
	size_t n,          // number of bytes to write
	EffectsBuffer *eb  // effects-buffer
) {
	ASSERT(n   > 0);
	ASSERT(eb  != NULL);
	ASSERT(ptr != NULL);

	while(n > 0) {
		struct EffectsBufferBlock *b = eb->current;
		size_t written = EffectsBufferBlock_WriteBytes(ptr, n, b);
		if(written == 0) {
			// no bytes written block is full, create a new block
			EffectsBuffer_AddBlock(eb);
		}

		// update remaining bytes to write
		n -= written;
	}
}

static void EffectsBuffer_WriteString
(
	const char *str,
	EffectsBuffer *eb
) {
	ASSERT(eb  != NULL);
	ASSERT(str != NULL);

	size_t l = strlen(str) + 1;
	EffectsBuffer_WriteBytes(&l, sizeof(size_t), eb);
	EffectsBuffer_WriteBytes(str, l, eb);
}

// writes a binary representation of v into Effect-Buffer
static void EffectsBuffer_WriteSIValue
(
	const SIValue *v,
	EffectsBuffer *buff
) {
	ASSERT(v != NULL);
	ASSERT(buff != NULL);

	// format:
	//    type
	//    value
	bool b;
	size_t len = 0;

	SIType t = v->type;

	// write type
	EffectsBuffer_WriteBytes(&t, sizeof(SIType), buff);

	// write value
	switch(t) {
		case T_POINT:
			// write value to stream
			EffectsBuffer_WriteBytes(&v->point, sizeof(Point), buff);
			break;
		case T_ARRAY:
			// write array to stream
			EffectsBuffer_WriteSIArray(v, buff);
			break;
		case T_STRING:
			EffectsBuffer_WriteString(v->stringval, buff);
			break;
		case T_BOOL:
			// write bool to stream
			b = SIValue_IsTrue(*v);
			EffectsBuffer_WriteBytes(&b, sizeof(bool), buff);
			break;
		case T_INT64:
			// write int to stream
			EffectsBuffer_WriteBytes(&v->longval, sizeof(v->longval), buff);
			break;
		case T_DOUBLE:
			// write double to stream
			EffectsBuffer_WriteBytes(&v->doubleval, sizeof(v->doubleval), buff);
			break;
		case T_NULL:
			// no additional data is required to represent NULL
			break;
		default:
			assert(false && "unknown SIValue type");
	}
}

// writes a binary representation of arr into Effect-Buffer
static void EffectsBuffer_WriteSIArray
(
	const SIValue *arr,  // array
	EffectsBuffer *buff  // effect buffer
) {
	// format:
	// number of elements
	// elements

	SIValue *elements = arr->array;
	uint32_t len = array_len(elements);

	// write number of elements
	EffectsBuffer_WriteBytes(&len, sizeof(uint32_t), buff);

	// write each element
	for (uint32_t i = 0; i < len; i++) {
		EffectsBuffer_WriteSIValue(elements + i, buff);
	}
}

// dump attributes to stream
static void EffectsBuffer_WriteAttributeSet
(
	const AttributeSet attrs,  // attribute set to write to stream
	EffectsBuffer *buff
) {
	//--------------------------------------------------------------------------
	// write attribute count
	//--------------------------------------------------------------------------

	ushort attr_count = ATTRIBUTE_SET_COUNT(attrs);
	EffectsBuffer_WriteBytes(&attr_count, sizeof(attr_count), buff);

	//--------------------------------------------------------------------------
	// write attributes
	//--------------------------------------------------------------------------

	for(ushort i = 0; i < attr_count; i++) {
		// get current attribute name and value
		Attribute_ID attr_id;
		SIValue attr = AttributeSet_GetIdx(attrs, i, &attr_id);

		// write attribute ID
		EffectsBuffer_WriteBytes(&attr_id, sizeof(Attribute_ID), buff);

		// write attribute value
		EffectsBuffer_WriteSIValue(&attr, buff);
	}
}

static inline void EffectsBuffer_IncEffectCount
(
	EffectsBuffer *buff
) {
	ASSERT(buff != NULL);
	
	buff->n++;
}

// create a new effects-buffer
EffectsBuffer *EffectsBuffer_New
(
	void
) {
	size_t n = 62500;  // initial size of buffer
	EffectsBuffer *eb = rm_malloc(sizeof(EffectsBuffer));

	struct EffectsBufferBlock *b = EffectsBufferBlock_New(n);

	eb->n          = 0;
	eb->head       = b;
	eb->current    = b;
	eb->block_size = n;

	// write effects version to newly created buffer
	uint8_t v = EFFECTS_VERSION;
	EffectsBuffer_WriteBytes(&v, sizeof(v), eb);

	return eb;
}

// returns number of effects in buffer
uint64_t EffectsBuffer_Length
(
	const EffectsBuffer *buff  // effects-buffer
) {
	ASSERT(buff != NULL);
	
	return buff->n;
}

// get a copy of effectspbuffer internal buffer
unsigned char *EffectsBuffer_Buffer
(
	const EffectsBuffer *eb,  // effects-buffer
	size_t *n                 // size of returned buffer
) {
	ASSERT(eb != NULL);

	//--------------------------------------------------------------------------
	// determine required buffer size
	//--------------------------------------------------------------------------

	size_t l = 0;  // required buffer size
	struct EffectsBufferBlock *b = eb->head;
	while(b != NULL) {
		l += BLOCK_USED_SPACE(b);
		b = b->next;
	}

	//--------------------------------------------------------------------------
	// allocate buffer and populate
	//--------------------------------------------------------------------------

	unsigned char *buffer = rm_malloc(sizeof(unsigned char) * l);
	unsigned char *offset = buffer;

	b = eb->head;
	while(b != NULL) {
		// write block's data to buffer
		size_t _n = BLOCK_USED_SPACE(b);
		memcpy(offset, b->buffer, _n);
		offset += _n;

		// advance to next block
		b = b->next;
	}

	*n = l;
	return buffer;
}

//------------------------------------------------------------------------------
// effects creation API
//------------------------------------------------------------------------------

// add a node creation effect to buffer
void EffectsBuffer_AddCreateNodeEffect
(
	EffectsBuffer *buff,    // effect buffer
	const Node *n,          // node created
	const LabelID *labels,  // node labels
	ushort label_count      // number of labels
) {
	//--------------------------------------------------------------------------
	// effect format:
	// effect type
	// label count
	// labels
	// attribute count
	// attributes (id,value) pair
	//--------------------------------------------------------------------------
	
	ResultSetStatistics *stats = QueryCtx_GetResultSetStatistics();
	stats->nodes_created++;
	stats->properties_set += ATTRIBUTE_SET_COUNT(*n->attributes);

	EffectType t = EFFECT_CREATE_NODE;
	EffectsBuffer_WriteBytes(&t, sizeof(t), buff);

	//--------------------------------------------------------------------------
	// write label count
	//--------------------------------------------------------------------------

	EffectsBuffer_WriteBytes(&label_count, sizeof(label_count), buff);

	//--------------------------------------------------------------------------
	// write labels
	//--------------------------------------------------------------------------

	if(label_count > 0) {
		EffectsBuffer_WriteBytes(labels, sizeof(LabelID) * label_count, buff);
	}

	//--------------------------------------------------------------------------
	// write attribute set
	//--------------------------------------------------------------------------

	const AttributeSet attrs = GraphEntity_GetAttributes((const GraphEntity*)n);
	EffectsBuffer_WriteAttributeSet(attrs, buff);

	EffectsBuffer_IncEffectCount(buff);
}

// add a edge creation effect to buffer
void EffectsBuffer_AddCreateEdgeEffect
(
	EffectsBuffer *buff,  // effect buffer
	const Edge *edge      // edge created
) {
	//--------------------------------------------------------------------------
	// effect format:
	// effect type
	// relationship count
	// relationships
	// src node ID
	// dest node ID
	// attribute count
	// attributes (id,value) pair
	//--------------------------------------------------------------------------
	
	ResultSetStatistics *stats = QueryCtx_GetResultSetStatistics();
	stats->relationships_created++;
	stats->properties_set += ATTRIBUTE_SET_COUNT(*edge->attributes);

	EffectType t = EFFECT_CREATE_EDGE;
	EffectsBuffer_WriteBytes(&t, sizeof(t), buff);

	//--------------------------------------------------------------------------
	// write relationship type
	//--------------------------------------------------------------------------

	ushort rel_count = 1;
	EffectsBuffer_WriteBytes(&rel_count, sizeof(rel_count), buff);

	RelationID rel_id = Edge_GetRelationID(edge);
	EffectsBuffer_WriteBytes(&rel_id, sizeof(RelationID), buff);

	//--------------------------------------------------------------------------
	// write src node ID
	//--------------------------------------------------------------------------
	
	NodeID src_id = Edge_GetSrcNodeID(edge);
	EffectsBuffer_WriteBytes(&src_id, sizeof(NodeID), buff);

	//--------------------------------------------------------------------------
	// write dest node ID
	//--------------------------------------------------------------------------

	NodeID dest_id = Edge_GetDestNodeID(edge);
	EffectsBuffer_WriteBytes(&dest_id, sizeof(NodeID), buff);

	//--------------------------------------------------------------------------
	// write attribute set 
	//--------------------------------------------------------------------------

	const AttributeSet attrs = GraphEntity_GetAttributes((GraphEntity*)edge);
	EffectsBuffer_WriteAttributeSet(attrs, buff);

	EffectsBuffer_IncEffectCount(buff);
}

// add a node deletion effect to buffer
void EffectsBuffer_AddDeleteNodeEffect
(
	EffectsBuffer *buff,  // effect buffer
	const Node *node      // node deleted
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    node ID
	//--------------------------------------------------------------------------

	QueryCtx_GetResultSetStatistics()->nodes_deleted++;

	EffectType t = EFFECT_DELETE_NODE;
	EffectsBuffer_WriteBytes(&t, sizeof(t), buff);

	// write node ID
	EffectsBuffer_WriteBytes(&ENTITY_GET_ID(node), sizeof(EntityID), buff);

	EffectsBuffer_IncEffectCount(buff);
}

// add a edge deletion effect to buffer
void EffectsBuffer_AddDeleteEdgeEffect
(
	EffectsBuffer *eb,  // effect buffer
	const Edge *edge    // edge deleted
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    edge ID
	//    relation ID
	//    src ID
	//    dest ID
	//--------------------------------------------------------------------------

	QueryCtx_GetResultSetStatistics()->relationships_deleted++;

	EffectType t = EFFECT_DELETE_EDGE;
	EffectsBuffer_WriteBytes(&t, sizeof(t), eb);

	EffectsBuffer_WriteBytes(&ENTITY_GET_ID(edge), sizeof(EntityID), eb);

	RelationID r_id = Edge_GetRelationID(edge);
	EffectsBuffer_WriteBytes(&r_id, sizeof(RelationID), eb);

	NodeID src_id = Edge_GetSrcNodeID(edge);
	EffectsBuffer_WriteBytes(&src_id, sizeof(EntityID), eb);

	NodeID dest_id = Edge_GetDestNodeID(edge);
	EffectsBuffer_WriteBytes(&dest_id, sizeof(EntityID), eb);

	EffectsBuffer_IncEffectCount(eb);
};

// add an entity update effect to buffer
static void EffectsBuffer_AddNodeUpdateEffect
(
	EffectsBuffer *buff,   // effect buffer
	Node *node,            // updated node
	Attribute_ID attr_id,  // updated attribute ID
 	SIValue value          // value
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    entity ID
	//    attribute id
	//    attribute value
	//--------------------------------------------------------------------------

	EffectType t = EFFECT_UPDATE_NODE;
	EffectsBuffer_WriteBytes(&t, sizeof(t), buff);

	//--------------------------------------------------------------------------
	// write entity ID
	//--------------------------------------------------------------------------

	EffectsBuffer_WriteBytes(&ENTITY_GET_ID(node), sizeof(EntityID), buff);

	//--------------------------------------------------------------------------
	// write attribute ID
	//--------------------------------------------------------------------------
	
	EffectsBuffer_WriteBytes(&attr_id, sizeof(Attribute_ID), buff);

	//--------------------------------------------------------------------------
	// write attribute value
	//--------------------------------------------------------------------------

	EffectsBuffer_WriteSIValue(&value, buff);

	EffectsBuffer_IncEffectCount(buff);
}

// add an entity update effect to buffer
static void EffectsBuffer_AddEdgeUpdateEffect
(
	EffectsBuffer *buff,   // effect buffer
	Edge *edge,            // updated edge
	Attribute_ID attr_id,  // updated attribute ID
 	SIValue value          // value
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    edge ID
	//    relation ID
	//    src ID
	//    dest ID
	//    attribute count (=n)
	//    attributes (id,value) pair
	//--------------------------------------------------------------------------

	EffectType t = EFFECT_UPDATE_EDGE;
	EffectsBuffer_WriteBytes(&t, sizeof(t), buff);

	//--------------------------------------------------------------------------
	// write edge ID
	//--------------------------------------------------------------------------

	EffectsBuffer_WriteBytes(&ENTITY_GET_ID(edge), sizeof(EntityID), buff);

	//--------------------------------------------------------------------------
	// write relation ID
	//--------------------------------------------------------------------------

	Graph *g = QueryCtx_GetGraph();
	RelationID r = EDGE_GET_RELATION_ID(edge, g);
	EffectsBuffer_WriteBytes(&r, sizeof(RelationID), buff);

	//--------------------------------------------------------------------------
	// write src ID
	//--------------------------------------------------------------------------

	NodeID s = Edge_GetSrcNodeID(edge);
	EffectsBuffer_WriteBytes(&s, sizeof(NodeID), buff);

	//--------------------------------------------------------------------------
	// write dest ID
	//--------------------------------------------------------------------------

	NodeID d = Edge_GetDestNodeID(edge);
	EffectsBuffer_WriteBytes(&d, sizeof(NodeID), buff);

	//--------------------------------------------------------------------------
	// write attribute ID
	//--------------------------------------------------------------------------

	EffectsBuffer_WriteBytes(&attr_id, sizeof(Attribute_ID), buff);

	//--------------------------------------------------------------------------
	// write attribute value
	//--------------------------------------------------------------------------

	EffectsBuffer_WriteSIValue(&value, buff);

	EffectsBuffer_IncEffectCount(buff);
}

// add an entity attribute removal effect to buffer
void EffectsBuffer_AddEntityRemoveAttributeEffect
(
	EffectsBuffer *buff,         // effect buffer
	GraphEntity *entity,         // updated entity ID
	Attribute_ID attr_id,        // updated attribute ID
	GraphEntityType entity_type  // entity type
) {
	// attribute was deleted
	int n = (attr_id == ATTRIBUTE_ID_ALL)
		? ATTRIBUTE_SET_COUNT(*entity->attributes)
		: 1;

	ResultSetStatistics *stats = QueryCtx_GetResultSetStatistics();
	stats->properties_removed += n;

	SIValue v = SI_NullVal();
	if(entity_type == GETYPE_NODE) {
		EffectsBuffer_AddNodeUpdateEffect(buff, (Node*)entity, attr_id, v);
	} else {
		EffectsBuffer_AddEdgeUpdateEffect(buff, (Edge*)entity, attr_id, v);
	}
}

// add an entity add new attribute effect to buffer
void EffectsBuffer_AddEntityAddAttributeEffect
(
	EffectsBuffer *buff,         // effect buffer
	GraphEntity *entity,         // updated entity ID
	Attribute_ID attr_id,        // updated attribute ID
	SIValue value,               // value
	GraphEntityType entity_type  // entity type
) {
	// attribute was added
	QueryCtx_GetResultSetStatistics()->properties_set++;

	if(entity_type == GETYPE_NODE) {
		EffectsBuffer_AddNodeUpdateEffect(buff, (Node*)entity, attr_id, value);
	} else {
		EffectsBuffer_AddEdgeUpdateEffect(buff, (Edge*)entity, attr_id, value);
	}
}

// add an entity update attribute effect to buffer
void EffectsBuffer_AddEntityUpdateAttributeEffect
(
	EffectsBuffer *buff,         // effect buffer
	GraphEntity *entity,         // updated entity ID
	Attribute_ID attr_id,        // updated attribute ID
	SIValue value,               // value
	GraphEntityType entity_type  // entity type
) {
	ResultSetStatistics *stats = QueryCtx_GetResultSetStatistics();
	stats->properties_set++; // attribute was set
	stats->properties_removed++; // old attribute was deleted

	if(entity_type == GETYPE_NODE) {
		EffectsBuffer_AddNodeUpdateEffect(buff, (Node*)entity, attr_id, value);
	} else {
		EffectsBuffer_AddEdgeUpdateEffect(buff, (Edge*)entity, attr_id, value);
	}
}

// add a node add label effect to buffer
void EffectsBuffer_AddSetRemoveLabelsEffect
(
	EffectsBuffer *buff,     // effect buffer
	const Node *node,        // updated node
	const LabelID *lbl_ids,  // labels
	uint8_t lbl_count,       // number of labels
	EffectType t             // effect type
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    node ID
	//    labels count
	//    label IDs
	//--------------------------------------------------------------------------

	EffectsBuffer_WriteBytes(&t, sizeof(t), buff);

	// write node ID
	EffectsBuffer_WriteBytes(&ENTITY_GET_ID(node), sizeof(EntityID), buff); 
	
	// write labels count
	EffectsBuffer_WriteBytes(&lbl_count, sizeof(lbl_count), buff); 
	
	// write label IDs
	EffectsBuffer_WriteBytes(lbl_ids, sizeof(LabelID) * lbl_count, buff);

	EffectsBuffer_IncEffectCount(buff);
}

// add a node add labels effect to buffer
void EffectsBuffer_AddLabelsEffect
(
	EffectsBuffer *buff,     // effect buffer
	const Node *node,        // updated node
	const LabelID *lbl_ids,  // added labels
	size_t lbl_count         // number of removed labels
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    node ID
	//    labels count
	//    label IDs
	//--------------------------------------------------------------------------

	QueryCtx_GetResultSetStatistics()->labels_added += lbl_count;

	EffectType t = EFFECT_SET_LABELS;
	EffectsBuffer_AddSetRemoveLabelsEffect(buff, node, lbl_ids, lbl_count, t);

	EffectsBuffer_IncEffectCount(buff);
}

// add a node remove labels effect to buffer
void EffectsBuffer_AddRemoveLabelsEffect
(
	EffectsBuffer *buff,     // effect buffer
	const Node *node,        // updated node
	const LabelID *lbl_ids,  // removed labels
	size_t lbl_count         // number of removed labels
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    node ID
	//    labels count
	//    label IDs
	//--------------------------------------------------------------------------

	QueryCtx_GetResultSetStatistics()->labels_removed += lbl_count;

	EffectType t = EFFECT_REMOVE_LABELS;
	EffectsBuffer_AddSetRemoveLabelsEffect(buff, node, lbl_ids, lbl_count, t);

	EffectsBuffer_IncEffectCount(buff);
}

// add a schema addition effect to buffer
void EffectsBuffer_AddNewSchemaEffect
(
	EffectsBuffer *buff,      // effect buffer
	const char *schema_name,  // id of the schema
	SchemaType st             // type of the schema
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    schema type
	//    schema name
	//--------------------------------------------------------------------------

	EffectType t = EFFECT_ADD_SCHEMA;
	EffectsBuffer_WriteBytes(&t, sizeof(t), buff);

	//--------------------------------------------------------------------------
	// write schema type
	//--------------------------------------------------------------------------

	EffectsBuffer_WriteBytes(&st, sizeof(st), buff);

	//--------------------------------------------------------------------------
	// write schema name
	//--------------------------------------------------------------------------

	EffectsBuffer_WriteString(schema_name, buff);

	EffectsBuffer_IncEffectCount(buff);
}

// add an attribute addition effect to buffer
void EffectsBuffer_AddNewAttributeEffect
(
	EffectsBuffer *buff,  // effect buffer
	const char *attr      // attribute name
) {
	//--------------------------------------------------------------------------
	// effect format:
	// effect type
	// attribute name
	//--------------------------------------------------------------------------

	EffectType t = EFFECT_ADD_ATTRIBUTE;
	EffectsBuffer_WriteBytes(&t, sizeof(t), buff);

	//--------------------------------------------------------------------------
	// write attribute name
	//--------------------------------------------------------------------------

	EffectsBuffer_WriteString(attr, buff);

	EffectsBuffer_IncEffectCount(buff);
}

static inline void EffectsBufferBlock_Free
(
	struct EffectsBufferBlock *b
) {
	ASSERT(b != NULL);
	rm_free(b);
}

void EffectsBuffer_Free
(
	EffectsBuffer *eb
) {
	ASSERT(eb != NULL);

	// free blocks
	struct EffectsBufferBlock *b = eb->head;
	while(b != NULL) {
		struct EffectsBufferBlock *next = b->next;
		EffectsBufferBlock_Free(b);
		b = next;
	}

	rm_free(eb);
}

