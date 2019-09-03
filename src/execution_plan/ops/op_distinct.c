/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_distinct.h"
#include "xxhash.h"
#include "../../util/arr.h"

OpBase *NewDistinctOp(void) {
	OpDistinct *self = malloc(sizeof(OpDistinct));
	self->found = raxNew();

	OpBase_Init(&self->op);
	self->op.name = "Distinct";
	self->op.type = OPType_DISTINCT;
	self->op.consume = DistinctConsume;
	self->op.reset = DistinctReset;
	self->op.free = DistinctFree;

	return (OpBase *)self;
}

Record DistinctConsume(OpBase *opBase) {
	OpDistinct *self = (OpDistinct *)opBase;
	OpBase *child = self->op.children[0];

	while(true) {
		Record r = OpBase_Consume(child);
		if(!r) return NULL;

		unsigned long long const hash = Record_Hash64(r);
		int is_new = raxInsert(self->found, (unsigned char *) &hash, sizeof(hash), NULL, NULL);
		if(is_new) return r;
		Record_Free(r);
	}
}

OpResult DistinctReset(OpBase *ctx) {
	return OP_OK;
}

void DistinctFree(OpBase *ctx) {
	OpDistinct *op = (OpDistinct *)ctx;
	if(op->found) {
		raxFree(op->found);
		op->found = NULL;
	}
}
