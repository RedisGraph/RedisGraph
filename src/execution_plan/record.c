/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "xxhash.h"
#include "./record.h"
#include "../util/rmalloc.h"
#include <assert.h>

/* Migrate the entry at the given index in the source Record at the same index in the destination.
 * The source retains access to but not ownership of the entry if it is a heap allocation. */
static void _RecordPropagateEntry(Record dest, Record src, uint idx) {
	Entry e = src->entries[idx];
	dest->entries[idx] = e;
	// If the entry is a scalar, make sure both Records don't believe they own the allocation.
	if(e.type == REC_TYPE_SCALAR) SIValue_MakeVolatile(&src->entries[idx].value.s);
}

// This function is currently unused.
Record Record_New(rax *mapping) {
	assert(mapping);
	// Determine record size.
	uint entries_count = raxSize(mapping);
	uint rec_size = sizeof(_Record);
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

// Retrieve the offset into the Record of the given alias.
int Record_GetEntryIdx(Record r, const char *alias) {
	assert(r && alias);

	void *idx = raxFind(r->mapping, (unsigned char *)alias, strlen(alias));

	return idx != raxNotFound ? (intptr_t)idx : INVALID_INDEX;
}

void Record_Clone(const Record r, Record clone) {
	int entry_count = Record_length(r);
	size_t required_record_size = sizeof(Entry) * entry_count;

	memcpy(clone->entries, r->entries, required_record_size);

	/* Foreach scalar entry in cloned record, make sure it is not freed.
	 * it is the original record owner responsibility to free the record
	 * and its internal scalar as a result.
	 *
	 * TODO: I wish we wouldn't have to perform this loop as it is a major performance hit
	 * with the introduction of a garbage collection this should be removed. */
	for(int i = 0; i < entry_count; i++) {
		if(Record_GetType(clone, i) == REC_TYPE_SCALAR) {
			SIValue_MakeVolatile(&clone->entries[i].value.s);
		}
	}
}

void Record_Merge(Record *a, const Record b) {
	uint len = Record_length(b);
	for(uint i = 0; i < len; i++) {
		if(b->entries[i].type != REC_TYPE_UNKNOWN) {
			(*a)->entries[i] = b->entries[i];
		}
	}
}

void Record_TransferEntries(Record *to, Record from) {
	uint len = Record_length(from);
	for(uint i = 0; i < len; i++) {
		if(from->entries[i].type != REC_TYPE_UNKNOWN) {
			_RecordPropagateEntry(*to, from, i);
		}
	}
}

RecordEntryType Record_GetType(const Record r, int idx) {
	return r->entries[idx].type;
}

Node *Record_GetNode(const Record r, int idx) {
	switch(r->entries[idx].type) {
	case REC_TYPE_NODE:
		return &(r->entries[idx].value.n);
	case REC_TYPE_UNKNOWN:
		return NULL;
	case REC_TYPE_SCALAR:
		// Null scalar values are expected here; otherwise fall through.
		if(SIValue_IsNull(r->entries[idx].value.s)) return NULL;
	default:
		assert("encountered unexpected type in Record; expected Node" && false);
	}
}

Edge *Record_GetEdge(const Record r, int idx) {
	switch(r->entries[idx].type) {
	case REC_TYPE_EDGE:
		return &(r->entries[idx].value.e);
	case REC_TYPE_UNKNOWN:
		return NULL;
	case REC_TYPE_SCALAR:
		// Null scalar values are expected here; otherwise fall through.
		if(SIValue_IsNull(r->entries[idx].value.s)) return NULL;
	default:
		assert("encountered unexpected type in Record; expected Edge" && false);
	}
}

SIValue Record_Get(Record r, int idx) {
	Entry e = r->entries[idx];
	switch(e.type) {
	case REC_TYPE_NODE:
		return SI_Node(Record_GetNode(r, idx));
	case REC_TYPE_EDGE:
		return SI_Edge(Record_GetEdge(r, idx));
	case REC_TYPE_SCALAR:
		return r->entries[idx].value.s;
	case REC_TYPE_UNKNOWN:
		return SI_NullVal();
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
	default:
		assert(false && "encountered unexpected type when trying to retrieve graph entity");
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

SIValue *Record_AddScalar(Record r, int idx, SIValue v) {
	r->entries[idx].value.s = v;
	r->entries[idx].type = REC_TYPE_SCALAR;
	return &(r->entries[idx].value.s);
}

Node *Record_AddNode(Record r, int idx, Node node) {
	r->entries[idx].value.n = node;
	r->entries[idx].type = REC_TYPE_NODE;
	return &(r->entries[idx].value.n);
}

Edge *Record_AddEdge(Record r, int idx, Edge edge) {
	r->entries[idx].value.e = edge;
	r->entries[idx].type = REC_TYPE_EDGE;
	return &(r->entries[idx].value.e);
}

void Record_PersistScalars(Record r) {
	uint len = Record_length(r);
	for(uint i = 0; i < len; i++) {
		if(r->entries[i].type == REC_TYPE_SCALAR) SIValue_Persist(&r->entries[i].value.s);
	}
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
	// Initialize the hash state.
	XXH64_state_t state;
	XXH_errorcode res = XXH64_reset(&state, 0);
	assert(res != XXH_ERROR);

	uint rec_len = Record_length(r);
	for(uint idx = 0; idx < rec_len; idx++) {
		/* Retrieve the entry at 'idx' as an SIValue.
		 * If this entry is of type REC_TYPE_UNKNOWN, it will be returned as an SI_NullVal.
		 * As such, this hashing logic will not differentiate between implicit and explicit
		 * NULL values, but this is an acceptable design choice as the Cypher specification
		 * does not prescribe behavior for this scenario. */
		SIValue v = Record_Get(r, idx);
		// Update the hash state with the current value.
		SIValue_HashUpdate(v, &state);
	}

	// Finalize the hash.
	unsigned long long const hash = XXH64_digest(&state);
	return hash;
}

void Record_FreeEntries(Record r) {
	uint length = Record_length(r);
	for(uint i = 0; i < length; i++) {
		// Free any allocations held by this Record.
		if(r->entries[i].type == REC_TYPE_SCALAR) {
			SIValue_Free(r->entries[i].value.s);
		}
	}
}

// This function is currently unused.
void Record_Free(Record r) {
	Record_FreeEntries(r);
	rm_free(r);
}

