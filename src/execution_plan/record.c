/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./record.h"
#include "../util/rmalloc.h"
#include "xxhash.h"
#include <assert.h>

#define RECORD_HEADER(r) (r-1)
#define RECORD_HEADER_ENTRY(r) *(RECORD_HEADER((r)))

static inline void _Record_ShareEntry(Record a, const Entry e, uint idx) {
	a[idx] = e;
	// If the entry is a scalar, make sure both Records don't believe they own the allocation.
	if(e.type == REC_TYPE_SCALAR) SIValue_MakeVolatile(&a[idx].value.s);
}

Record Record_New(int entries) {
	Record r = rm_calloc((entries + 1), sizeof(Entry));

	// First entry holds records length.
	r[0].type = REC_TYPE_HEADER;
	r[0].value.s = SI_LongVal(entries);

	// Skip header entry.
	return r + 1;
}

void Record_Extend(Record *r, int len) {
	int original_len = Record_length(*r);
	if(original_len >= len) return;

	Record header = RECORD_HEADER(*r);
	header->value.s.longval = len;
	header = rm_realloc(header, sizeof(Entry) * (len + 1));

	// Initialize the added space to 0 (as Record_Merge will access it directly)
	int added_count = len - original_len;
	memset(header + original_len + 1, 0, added_count * sizeof(Entry));

	// Reposition the Record's data pointer in case it was moved by the realloc
	*r = header + 1;
}

void Record_Truncate(Record r, uint count) {
	uint original_len = Record_length(r);
	if(count >= original_len) return;

	for(uint i = count + 1; i < original_len; i++) {
		if(r[i].type == REC_TYPE_SCALAR) {
			SIValue_Free(&r[i].value.s);
		}
	}

	Record header = RECORD_HEADER(r);
	header->value.s.longval = original_len - count;
}

unsigned int Record_length(const Record r) {
	Entry header = RECORD_HEADER_ENTRY(r);
	int recordLength = header.value.s.longval;
	return recordLength;
}

Record Record_Clone(const Record r) {
	int recordLength = Record_length(r);
	Record clone = Record_New(recordLength);
	for(uint i = 0; i < recordLength; i++) {
		_Record_ShareEntry(clone, r[i], i);
	}
	return clone;
}

void Record_Merge(Record *a, const Record b) {
	int aLength = Record_length(*a);
	int bLength = Record_length(b);
	if(aLength < bLength) Record_Extend(a, bLength);

	for(int i = 0; i < bLength; i++) {
		if(b[i].type != REC_TYPE_UNKNOWN) {
			_Record_ShareEntry(*a, b[i], i);
		}
	}
}

RecordEntryType Record_GetType(const Record r, int idx) {
	return r[idx].type;
}

SIValue Record_GetScalar(Record r,  int idx) {
	r[idx].type = REC_TYPE_SCALAR;
	return r[idx].value.s;
}

Node *Record_GetNode(const Record r,  int idx) {
	r[idx].type = REC_TYPE_NODE;
	return &(r[idx].value.n);
}

Edge *Record_GetEdge(const Record r,  int idx) {
	r[idx].type = REC_TYPE_EDGE;
	return &(r[idx].value.e);
}

SIValue Record_Get(Record r, int idx) {
	Entry e = r[idx];
	switch(e.type) {
	case REC_TYPE_NODE:
		return SI_Node(Record_GetNode(r, idx));
	case REC_TYPE_EDGE:
		return SI_Edge(Record_GetEdge(r, idx));
	case REC_TYPE_SCALAR:
		return Record_GetScalar(r, idx);
	default:
		assert(false);
	}
}

GraphEntity *Record_GetGraphEntity(const Record r, int idx) {
	Entry e = r[idx];
	switch(e.type) {
	case REC_TYPE_NODE:
		return (GraphEntity *)Record_GetNode(r, idx);
	case REC_TYPE_EDGE:
		return (GraphEntity *)Record_GetEdge(r, idx);
	case REC_TYPE_SCALAR:
		return (GraphEntity *)(Record_GetScalar(r, idx).ptrval);
	default:
		assert(false);
	}
	return NULL;
}

void Record_Add(Record r, int idx, SIValue v) {
	switch(SI_TYPE(v)) {
	case T_NODE:
		Record_AddNode(r, idx, *(Node *)v.ptrval);
		break;
	case T_EDGE:
		Record_AddEdge(r, idx, *(Edge *)v.ptrval);
		break;
	default:
		Record_AddScalar(r, idx, v);
		break;
	}
}

void Record_AddScalar(Record r, int idx, SIValue v) {
	r[idx].value.s = v;
	r[idx].type = REC_TYPE_SCALAR;
}

void Record_AddNode(Record r, int idx, Node node) {
	r[idx].value.n = node;
	r[idx].type = REC_TYPE_NODE;
}

void Record_AddEdge(Record r, int idx, Edge edge) {
	r[idx].value.e = edge;
	r[idx].type = REC_TYPE_EDGE;
}

size_t Record_ToString(const Record r, char **buf, size_t *buf_cap) {
	uint rLen = Record_length(r);
	SIValue values[rLen];
	for(int i = 0; i < rLen; i++) {
		if(Record_GetType(r, i) == REC_TYPE_UNKNOWN) {
			values[i] = SI_ConstStringVal("UNKNOWN");
		} else {
			values[i] = Record_Get(r, i);
		}
	}

	size_t required_len = SIValue_StringConcatLen(values, rLen);

	if(*buf_cap < required_len) {
		*buf = rm_realloc(*buf, sizeof(char) * required_len);
		*buf_cap = required_len;
	}

	return SIValue_StringConcat(values, rLen, *buf, *buf_cap);
}

unsigned long long Record_Hash64(const Record r) {
	uint rec_len = Record_length(r);
	void *data;
	size_t len;
	static long long _null = 0;
	EntityID id;
	SIValue si;

	XXH_errorcode res;
	XXH64_state_t state;

	res = XXH64_reset(&state, 0);
	assert(res != XXH_ERROR);

	for(int i = 0; i < rec_len; ++i) {
		Entry e = r[i];
		switch(e.type) {
		case REC_TYPE_NODE:
		case REC_TYPE_EDGE:
			// Since nodes and edges cannot occupy the same index within
			// a record, we do not need to differentiate on type
			id = ENTITY_GET_ID(Record_GetGraphEntity(r, i));
			data = &id;
			len = sizeof(id);
			break;
		case REC_TYPE_SCALAR:
			si = Record_GetScalar(r, i);
			switch(si.type) {
			case T_NULL:
				data = &_null;
				len = sizeof(_null);
				break;

			case T_STRING:
				data = si.stringval;
				len = strlen(si.stringval);
				break;

			case T_INT64:
			case T_BOOL:
				data = &si.longval;
				len = sizeof(si.longval);
				break;

			case T_PTR:
				data = &si.ptrval;
				len = sizeof(si.ptrval);
				break;

			case T_DOUBLE:
				data = &si.doubleval;
				len = sizeof(si.doubleval);
				break;

			default:
				assert(false);
			}
			break;

		case REC_TYPE_UNKNOWN:
			assert(false);

		default:
			assert(false);
		}

		res = XXH64_update(&state, data, len);
		assert(res != XXH_ERROR);
	}

	unsigned long long const hash = XXH64_digest(&state);
	return hash;
}

void Record_Free(Record r) {
	unsigned int length = Record_length(r);
	for(unsigned int i = 0; i < length; i++) {
		if(r[i].type == REC_TYPE_SCALAR) {
			SIValue_Free(&r[i].value.s);
		}
	}
	rm_free((r - 1));
}

