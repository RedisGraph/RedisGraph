/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_distinct.h"
#include "../../../deps/xxhash/xxhash.h"
#include "../../util/arr.h"

/* Forward declarations */
static void Free(OpBase *ctx);
static OpResult Reset(OpBase *ctx);
static Record Consume(OpBase *opBase);

OpBase *NewDistinctOp(const ExecutionPlan *plan) {
	OpDistinct *op = malloc(sizeof(OpDistinct));
	op->trie = NewTrieMap();

	OpBase_Init((OpBase *)op, OPType_DISTINCT, "Distinct", NULL, Consume, Reset, NULL, Free, plan);

	return (OpBase *)op;
}

static Record Consume(OpBase *opBase) {
	OpDistinct *self = (OpDistinct *)opBase;
	OpBase *child = self->op.children[0];

	while(true) {
		Record r = OpBase_Consume(child);
		if(!r) return NULL;

		unsigned long long const hash = Record_Hash64(r);
		int is_new = TrieMap_Add(self->trie, (char *) &hash, sizeof(hash), NULL, TrieMap_DONT_CARE_REPLACE);
		if(is_new)
			return r;
		Record_Free(r);
	}
}

static OpResult Reset(OpBase *ctx) {
	return OP_OK;
}

static void Free(OpBase *ctx) {
	OpDistinct *op = (OpDistinct *)ctx;
	if(op->trie) {
		TrieMap_Free(op->trie, TrieMap_NOP_CB);
		op->trie = NULL;
	}
}
