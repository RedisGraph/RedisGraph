/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "resultset.h"
#include "../RG.h"
#include "../value.h"
#include "../errors.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../grouping/group_cache.h"

static void _ResultSet_ReplayStats(RedisModuleCtx *ctx, ResultSet *set) {
	char buff[512] = {0};
	size_t resultset_size = 2; // execution time, cached
	int buflen;

	if(set->stats.labels_added > 0) resultset_size++;
	if(set->stats.nodes_created > 0) resultset_size++;
	if(set->stats.properties_set > 0) resultset_size++;
	if(set->stats.relationships_created > 0) resultset_size++;
	if(set->stats.nodes_deleted > 0) resultset_size++;
	if(set->stats.relationships_deleted > 0) resultset_size++;
	if(set->stats.indices_created != STAT_NOT_SET) resultset_size++;
	if(set->stats.indices_deleted != STAT_NOT_SET) resultset_size++;

	RedisModule_ReplyWithArray(ctx, resultset_size);

	if(set->stats.labels_added > 0) {
		buflen = sprintf(buff, "Labels added: %d", set->stats.labels_added);
		RedisModule_ReplyWithStringBuffer(ctx, (const char *)buff, buflen);
	}

	if(set->stats.nodes_created > 0) {
		buflen = sprintf(buff, "Nodes created: %d", set->stats.nodes_created);
		RedisModule_ReplyWithStringBuffer(ctx, (const char *)buff, buflen);
	}

	if(set->stats.properties_set > 0) {
		buflen = sprintf(buff, "Properties set: %d", set->stats.properties_set);
		RedisModule_ReplyWithStringBuffer(ctx, (const char *)buff, buflen);
	}

	if(set->stats.relationships_created > 0) {
		buflen = sprintf(buff, "Relationships created: %d", set->stats.relationships_created);
		RedisModule_ReplyWithStringBuffer(ctx, (const char *)buff, buflen);
	}

	if(set->stats.nodes_deleted > 0) {
		buflen = sprintf(buff, "Nodes deleted: %d", set->stats.nodes_deleted);
		RedisModule_ReplyWithStringBuffer(ctx, (const char *)buff, buflen);
	}

	if(set->stats.relationships_deleted > 0) {
		buflen = sprintf(buff, "Relationships deleted: %d", set->stats.relationships_deleted);
		RedisModule_ReplyWithStringBuffer(ctx, (const char *)buff, buflen);
	}

	if(set->stats.indices_created != STAT_NOT_SET) {
		buflen = sprintf(buff, "Indices created: %d", set->stats.indices_created);
		RedisModule_ReplyWithStringBuffer(ctx, (const char *)buff, buflen);
	}

	if(set->stats.indices_deleted != STAT_NOT_SET) {
		buflen = sprintf(buff, "Indices deleted: %d", set->stats.indices_deleted);
		RedisModule_ReplyWithStringBuffer(ctx, (const char *)buff, buflen);
	}

	buflen = sprintf(buff, "Cached execution: %d", set->stats.cached ? 1 : 0);
	RedisModule_ReplyWithStringBuffer(ctx, (const char *)buff, buflen);

	// Emit query execution time.
	ResultSet_ReportQueryRuntime(ctx);
}

/* Map each column to a record index
 * such that when resolving resultset row i column j we'll extract
 * data from record at position columns_record_map[j]. */
void ResultSet_MapProjection(ResultSet *set, const Record r) {
	if(!set->columns_record_map) set->columns_record_map = rm_malloc(sizeof(uint) * set->column_count);

	for(uint i = 0; i < set->column_count; i++) {
		const char *column = set->columns[i];
		uint idx = Record_GetEntryIdx(r, column);
		ASSERT(idx != INVALID_INDEX);
		set->columns_record_map[i] = idx;
	}
}

static void _ResultSet_ReplyWithPreamble(ResultSet *set, const Record r) {
	ASSERT(set->recordCount == 0);

	// Prepare a response containing a header, records, statistics, and possibly a metadata footer.
	RedisModule_ReplyWithArray(set->ctx, REDISMODULE_POSTPONED_ARRAY_LEN);

	// Emit the table header using the appropriate formatter
	set->formatter->EmitHeader(set->ctx, set->columns, r, set->columns_record_map);

	set->header_emitted = true;

	// We don't know at this point the number of records we're about to return.
	RedisModule_ReplyWithArray(set->ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
}

static void _ResultSet_SetColumns(ResultSet *set) {
	ASSERT(set->columns == NULL);

	AST *ast = QueryCtx_GetAST();
	const cypher_astnode_type_t root_type = cypher_astnode_type(ast->root);
	if(root_type == CYPHER_AST_QUERY) {
		uint clause_count = cypher_ast_query_nclauses(ast->root);
		const cypher_astnode_t *last_clause = cypher_ast_query_get_clause(ast->root, clause_count - 1);
		cypher_astnode_type_t last_clause_type = cypher_astnode_type(last_clause);
		bool query_has_return = (last_clause_type == CYPHER_AST_RETURN);
		if(query_has_return) {
			set->columns = AST_BuildReturnColumnNames(last_clause);
			set->column_count = array_len(set->columns);
		} else if(last_clause_type == CYPHER_AST_CALL) {
			set->columns = AST_BuildCallColumnNames(last_clause);
			set->column_count = array_len(set->columns);
		}
	}
}

/* If appropriate, update the ResultSet to emit a footer.
 * Currently, we never emit a footer unless we are also emitting results
 * and are using the compact formatter. */
inline void ResultSet_MaybeEmitFooter(ResultSet *set) {
	if(set->columns == NULL || set->format != FORMATTER_COMPACT) return;
	set->require_footer = true;
}

ResultSet *NewResultSet(RedisModuleCtx *ctx, ResultSetFormatterType format) {
	ResultSet *set = rm_malloc(sizeof(ResultSet));
	set->ctx = ctx;
	set->gc = QueryCtx_GetGraphCtx();
	set->format = format;
	set->formatter = ResultSetFormatter_GetFormatter(format);
	set->columns = NULL;
	set->recordCount = 0;
	set->column_count = 0;
	set->header_emitted = false;
	set->require_footer = false;
	set->columns_record_map = NULL;

	set->stats.labels_added = 0;
	set->stats.nodes_created = 0;
	set->stats.properties_set = 0;
	set->stats.relationships_created = 0;
	set->stats.nodes_deleted = 0;
	set->stats.relationships_deleted = 0;
	set->stats.indices_created = STAT_NOT_SET;
	set->stats.indices_deleted = STAT_NOT_SET;
	set->stats.cached = false;

	_ResultSet_SetColumns(set);

	return set;
}

uint64_t ResultSet_RecordCount(const ResultSet *set) {
	ASSERT(set != NULL);
	return set->recordCount;
}

int ResultSet_AddRecord(ResultSet *set, Record r) {
	// If result-set format is NOP, don't process record.
	if(set->format == FORMATTER_NOP) return RESULTSET_OK;

	// If this is the first Record encountered
	if(set->header_emitted == false) {
		// Map columns to record indices.
		ResultSet_MapProjection(set, r);
		// Prepare response arrays and emit the header.
		_ResultSet_ReplyWithPreamble(set, r);
	}

	set->recordCount++;

	// Output the current record using the defined formatter
	set->formatter->EmitRecord(set->ctx, set->gc, r, set->column_count, set->columns_record_map);

	return RESULTSET_OK;
}

void ResultSet_IndexCreated(ResultSet *set, int status_code) {
	if(status_code == INDEX_OK) {
		if(set->stats.indices_created == STAT_NOT_SET) {
			set->stats.indices_created = 1;
		} else {
			set->stats.indices_created += 1;
		}
	} else if(set->stats.indices_created == STAT_NOT_SET) {
		set->stats.indices_created = 0;
	}
}

void ResultSet_IndexDeleted(ResultSet *set, int status_code) {
	if(status_code == INDEX_OK) {
		if(set->stats.indices_deleted == STAT_NOT_SET) {
			set->stats.indices_deleted = 1;
		} else {
			set->stats.indices_deleted += 1;
		}
	} else if(set->stats.indices_deleted == STAT_NOT_SET) {
		set->stats.indices_deleted = 0;
	}
}

void ResultSet_CachedExecution(ResultSet *set) {
	set->stats.cached = true;
}

void ResultSet_Reply(ResultSet *set) {
	if(set->columns != NULL) {
		// The query emits data.
		if(set->header_emitted) {
			// If we have emitted a header, set the number of elements in the preceding array.
			RedisModule_ReplySetArrayLength(set->ctx, set->recordCount);
		} else {
			ASSERT(set->recordCount == 0);
			// Handle the edge case in which the query was intended to return results, but none were created.
			_ResultSet_ReplyWithPreamble(set, NULL);
			RedisModule_ReplySetArrayLength(set->ctx, 0);
		}

		// If the output requires a footer, accommodate it in the replied arrays.
		if(set->require_footer == false) RedisModule_ReplySetArrayLength(set->ctx, 3);
		else RedisModule_ReplySetArrayLength(set->ctx, 4);
	} else {
		// Queries that don't emit data will only emit statistics
		RedisModule_ReplyWithArray(set->ctx, 1);
	}

	/* Check to see if we've encountered a run-time error.
	 * If so, emit it as the last top-level response. */
	if(ErrorCtx_EncounteredError()) ErrorCtx_EmitException();
	else _ResultSet_ReplayStats(set->ctx, set); // Otherwise, the last response is query statistics.

	if(set->require_footer) ResultSet_ReplyWithMetadata(set);
}

/* Report execution timing. */
void ResultSet_ReportQueryRuntime(RedisModuleCtx *ctx) {
	char *strElapsed;
	double t = QueryCtx_GetExecutionTime();
	asprintf(&strElapsed, "Query internal execution time: %.6f milliseconds", t);
	RedisModule_ReplyWithStringBuffer(ctx, strElapsed, strlen(strElapsed));
	free(strElapsed);
}

void ResultSet_Free(ResultSet *set) {
	if(!set) return;

	if(set->columns) array_free(set->columns);
	if(set->columns_record_map) rm_free(set->columns_record_map);

	rm_free(set);
}

