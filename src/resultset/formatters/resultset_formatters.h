/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "resultset_formatter.h"
#include "resultset_replynop.h"
#include "resultset_replybinary.h"
#include "resultset_replycompact.h"
#include "resultset_replyverbose.h"

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
	.EmitRow     = ResultSet_EmitNOPRow,
	.EmitHeader  = ResultSet_EmitNOPHeader,
	.FreePData   = ResultSet_FreeNOPPData,
	.CreatePData = ResultSet_CreateNOPPData
};

// compact reply formatter, this is the default formatter
static ResultSetFormatter ResultSetFormatterCompact __attribute__((used)) = {
	.EmitRow      =  ResultSet_EmitCompactRow,
	.EmitHeader   =  ResultSet_ReplyWithCompactHeader,
	.FreePData    =  ResultSet_FreeCompactPData,
	.CreatePData  =  ResultSet_CreateCompactPData
};

// verbose reply formatter, used when querying via CLI
static ResultSetFormatter ResultSetFormatterVerbose __attribute__((used)) = {
	.EmitRow     = ResultSet_EmitVerboseRow,
	.EmitHeader  = ResultSet_ReplyWithVerboseHeader,
	.FreePData   = ResultSet_FreeVerbosePData,
	.CreatePData = ResultSet_CreateVerbosePData
};

// binary reply formatter, experimantal
static ResultSetFormatter ResultSetFormatterBinary __attribute__((used)) = {
	.EmitRow     = ResultSet_EmitBinaryRow,
	.EmitHeader  = ResultSet_ReplyWithBinaryHeader,
	.FreePData   = ResultSet_FreeBinaryPData,
	.CreatePData = ResultSet_CreateBinaryPData
};

