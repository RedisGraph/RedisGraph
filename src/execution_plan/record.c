/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "xxhash.h"
#include "./record.h"
#include "../util/rmalloc.h"
#include <assert.h>

static void _RecordPropagateEntry(Record to, Record from, uint idx) {
	Entry e = from->entries[idx];
	to->entries[idx] = e;
	// If the entry is a scalar, make sure both Records don't believe they own the allocation.
	if(e.type == REC_TYPE_SCALAR) SIValue_MakeVolatile(&from->entries[idx].value.s);
}

Record Record_New(rax *mapping) {
	assert(mapping);
	// Determine record size.
	uint entries_count = raxSize(mapping);
	uint rec_size = sizeof(Record);
	rec_size += sizeof(Entry) * entries_count;

	Record r = rm_calloc(1, rec_size);
	r->mapping = mapping;

	return r;
}

// Returns the number of entries held by record.
uint Record_length(const Record r) {
	assert(r);
	return raxSize(r->mapping);
}

// Make sure record is able to hold len entries.
void Record_Extend(Record *r, int len) {
	int original_len = Record_length(*r);
	if(original_len >= len) return;

	// Determin record size.
	size_t required_record_size = sizeof(Record);
	required_record_size += sizeof(Entry) * len ;

	*r = rm_realloc(*r, required_record_size);
}

// Retrieve the offset into the Record of the given alias.
int Record_GetEntryIdx(Record r, const char *alias) {
	assert(r && alias);

	void *idx = raxFind(r->mapping, (unsigned char *)alias, strlen(alias));
	assert(idx != raxNotFound && "ERR: tried to resolve unexpected alias");

	return (intptr_t)idx;
}

Record Record_Clone(const Record r) {
	Record clone = Record_New(r->mapping);

	int entry_count = Record_length(r);
	size_t required_record_size = sizeof(Entry) * entry_count;

	memcpy(clone->entries, r->entries, required_record_size);
	return clone;
}

void Record_Merge(Record *a, const Record b) {
	int aLength = Record_length(*a);
	int bLength = Record_length(b);
	if(aLength < bLength) Record_Extend(a, bLength);

	for(int i = 0; i < bLength; i++) {
		if(b->entries[i].type != REC_TYPE_UNKNOWN) {
			(*a)->entries[i] = b->entries[i];
		}
	}
}

void Record_TransferEntries(Record *to, Record from) {
	int aLength = Record_length(*to);
	int bLength = Record_length(from);
	if(aLength < bLength) Record_Extend(to, bLength);

	for(int i = 0; i < bLength; i++) {
		if(from->entries[i].type != REC_TYPE_UNKNOWN) {
			_RecordPropagateEntry(*to, from, i);
		}
	}
}

RecordEntryType Record_GetType(const Record r, int idx) {
	return r->entries[idx].type;
}

SIValue Record_GetScalar(Record r, int idx) {
	r->entries[idx].type = REC_TYPE_SCALAR;
	return r->entries[idx].value.s;
}

Node *Record_GetNode(const Record r, int idx) {
	r->entries[idx].type = REC_TYPE_NODE;
	return &(r->entries[idx].value.n);
}

Edge *Record_GetEdge(const Record r, int idx) {
	r->entries[idx].type = REC_TYPE_EDGE;
	return &(r->entries[idx].value.e);
}

SIValue Record_Get(Record r, int idx) {
	Entry e = r->entries[idx];
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
	Entry e = r->entries[idx];
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
	assert(idx < Record_length(r));
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
	r->entries[idx].value.s = v;
	r->entries[idx].type = REC_TYPE_SCALAR;
}

void Record_AddNode(Record r, int idx, Node node) {
	r->entries[idx].value.n = node;
	r->entries[idx].type = REC_TYPE_NODE;
}

void Record_AddEdge(Record r, int idx, Edge edge) {
	r->entries[idx].value.e = edge;
	r->entries[idx].type = REC_TYPE_EDGE;
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

	size_t required_len = SIValue_StringJoinLen(values, rLen, ",");

	if(*buf_cap < required_len) {
		*buf = rm_realloc(*buf, sizeof(char) * required_len);
		*buf_cap = required_len;
	}

	size_t bytesWritten = 0;
	SIValue_StringJoin(values, rLen, ",", buf, buf_cap, &bytesWritten);
	return bytesWritten;
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
		Entry e = r->entries[i];
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
		if(r->entries[i].type == REC_TYPE_SCALAR) {
			SIValue_Free(&r->entries[i].value.s);
		}
	}
	rm_free(r);
}

