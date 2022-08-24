/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "resultset_formatter.h"
#include "formatter_nop.h"
#include "formatter_binary.h"
#include "formatter_compact.h"
#include "formatter_verbose.h"

typedef enum {
	FORMATTER_NOP = 0,
	FORMATTER_VERBOSE = 1,
	FORMATTER_COMPACT = 2,
	FORMATTER_BINARY  = 3,
} ResultSetFormatterType;

// retrieves result-set formatter
// returns NULL for an unknown formatter type
ResultSetFormatter* ResultSetFormatter_GetFormatter
(
	ResultSetFormatterType t
);

// reply formater which does absolutely nothing
// used when profiling a query
static ResultSetFormatter ResultSetNOP __attribute__((used)) = {
	.CreatePData  = Formatter_NOP_CreatePData,
	.ProcessRow   = Formatter_NOP_ProcessRow,
	.EmitHeader   = Formatter_NOP_EmitHeader,
	.EmitData     = Formatter_NOP_EmitData,
	.FreePData    = Formatter_NOP_FreePData
};

// compact reply formatter, this is the default formatter
static ResultSetFormatter ResultSetFormatterCompact __attribute__((used)) = {
	.CreatePData  = Formatter_Compact_CreatePData,
	.ProcessRow   = Formatter_Compact_ProcessRow,
	.EmitHeader   = Formatter_Compact_EmitHeader,
	.EmitData     = Formatter_Compact_EmitData,
	.FreePData    = Formatter_Compact_FreePData
};

// verbose reply formatter, used when querying via CLI
static ResultSetFormatter ResultSetFormatterVerbose __attribute__((used)) = {
	.CreatePData  = Formatter_Verbose_CreatePData,
	.ProcessRow   = Formatter_Verbose_ProcessRow,
	.EmitHeader   = Formatter_Verbose_EmitHeader,
	.EmitData     = Formatter_Verbose_EmitData,
	.FreePData    = Formatter_Verbose_FreePData
};

// binary reply formatter, experimantal
static ResultSetFormatter ResultSetFormatterBinary __attribute__((used)) = {
	.CreatePData = Formatter_Binary_CreatePData,
	.ProcessRow  = Formatter_Binary_ProcessRow,
	.EmitHeader  = Formatter_Binary_EmitHeader,
	.EmitData    = Formatter_Binary_EmitData,
	.FreePData   = Formatter_Binary_FreePData
};

