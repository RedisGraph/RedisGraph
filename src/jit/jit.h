/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../arithmetic/arithmetic_expression.h"
#include "pthread.h"
#include "llvm-c/Core.h"
#include "llvm-c/Error.h"
#include "llvm-c/Initialization.h"
#include "llvm-c/Orc.h"
#include "llvm-c/OrcEE.h"
#include "llvm-c/LLJIT.h"
#include "llvm-c/Support.h"
#include "llvm-c/Target.h"
#include "llvm-c/Analysis.h"
#include <llvm-c/Transforms/PassManagerBuilder.h>
#include "llvm-c/Transforms/Scalar.h"

typedef struct {
	LLVMBasicBlockRef loop_cond;
	LLVMBasicBlockRef loop;
	LLVMBasicBlockRef loop_end;
} LLVMLoop;

typedef struct {
	LLVMModuleRef module;
	LLVMBuilderRef builder;
	LLVMOrcThreadSafeContextRef TSCtx;
	LLVMContextRef Ctx;
	LLVMOrcThreadSafeModuleRef TSM;
	LLVMValueRef query;
	LLVMTypeRef voidPtr;
	LLVMTypeRef boolPtr;
	LLVMTypeRef i64ptr;
	LLVMTypeRef i1;
	LLVMTypeRef i8;
	LLVMTypeRef i32;
	LLVMTypeRef i64;
	LLVMTypeRef si_type;
	LLVMTypeRef node_type;
	LLVMValueRef r;
	LLVMLoop *loop;
} EmitCtx;

typedef void *(*SymbolResolve)(const char *);

void EmitCtx_Init();
EmitCtx *EmitCtx_Get();

void JIT_Init(void *g);
void JIT_End();
void JIT_Run(SymbolResolve fn);
void JIT_CreateRecord(void *opBase);
void JIT_Result(void *rsVal);
void JIT_Project(void *opBase, AR_ExpNode **exps, uint exp_count, uint *record_offsets);
void JIT_StartLabelScan(void *iter, int nodeIdx);
void JIT_EndLabelScan();