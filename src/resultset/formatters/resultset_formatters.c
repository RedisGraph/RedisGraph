/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "resultset_formatters.h"

ResultSetFormatter *ResultSetFormatter_GetFormatter(ResultSetFormatterType t) {
	ResultSetFormatter *formatter = NULL;
	switch(t) {
	case FORMATTER_NOP:
		formatter = &ResultSetNOP;
		break;
	case FORMATTER_VERBOSE:
		formatter = &ResultSetFormatterVerbose;
		break;
	case FORMATTER_COMPACT:
		formatter = &ResultSetFormatterCompact;
		break;
	case FORMATTER_BINARY:
		formatter = &ResultSetFormatterBinary;
		break;
	default:
		RedisModule_Assert(false && "Unknown formatter");
	}

	return formatter;
}

