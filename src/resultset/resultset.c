/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "resultset.h"
#include "../value.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../grouping/group_cache.h"
#include "../arithmetic/aggregate.h"

static void _ResultSet_ReplayStats(RedisModuleCtx *ctx, ResultSet *set) {
	char buff[512] = {0};
	size_t resultset_size = 1; /* query execution time. */
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

	// Emit query execution time.
	ResultSet_ReportQueryRuntime(ctx);
}

/* Map each column to a record index
 * such that when resolving resultset row i column j we'll extract
 * data from record at position columns_record_map[j]. */
static void _ResultSet_SetColToRecMap(ResultSet *set, const Record r) {
	assert(set->columns_record_map == NULL);

	set->columns_record_map = rm_malloc(sizeof(uint) * set->column_count);

	for(uint i = 0; i < set->column_count; i++) {
		const char *column = set->columns[i];
		uint idx = Record_GetEntryIdx(r, column);
		assert(idx != INVALID_INDEX);
		set->columns_record_map[i] = idx;
	}
}

static void _ResultSet_ReplyWithPreamble(ResultSet *set, const Record r) {
	assert(set->recordCount == 0);

	// Prepare a response containing a header, records, and statistics
	RedisModule_ReplyWithArray(set->ctx, 3);

	// Emit the table header using the appropriate formatter
	set->formatter->EmitHeader(set->ctx, set->columns, r, set->columns_record_map);

	set->header_emitted = true;

	// We don't know at this point the number of records we're about to return.
	RedisModule_ReplyWithArray(set->ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
}

static void _ResultSet_SetColumns(ResultSet *set) {
	assert(!set->columns);

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
	set->columns_record_map = NULL;

	set->stats.labels_added = 0;
	set->stats.nodes_created = 0;
	set->stats.properties_set = 0;
	set->stats.relationships_created = 0;
	set->stats.nodes_deleted = 0;
	set->stats.relationships_deleted = 0;
	set->stats.indices_created = STAT_NOT_SET;
	set->stats.indices_deleted = STAT_NOT_SET;

	_ResultSet_SetColumns(set);

	return set;
}

int ResultSet_AddRecord(ResultSet *set, Record r) {
	// If result-set format is NOP, don't process record.
	if(set->format == FORMATTER_NOP) return RESULTSET_OK;

	// If this is the first Record encountered
	if(set->header_emitted == false) {
		// Map columns to record indices.
		_ResultSet_SetColToRecMap(set, r);
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

void ResultSet_Reply(ResultSet *set) {
	if(set->header_emitted) {
		// If we have emitted a header, set the number of elements in the preceding array.
		RedisModule_ReplySetArrayLength(set->ctx, set->recordCount);
	} else if(set->header_emitted == false && set->columns != NULL) {
		assert(set->recordCount == 0);
		// Handle the edge case in which the query was intended to return results, but none were created.
		_ResultSet_ReplyWithPreamble(set, NULL);
		RedisModule_ReplySetArrayLength(set->ctx, 0);
	} else {
		// Queries that don't emit data will only emit statistics
		RedisModule_ReplyWithArray(set->ctx, 1);
	}

	/* Check to see if we've encountered a run-time error.
	 * If so, emit it as the last top-level response. */
	if(QueryCtx_EncounteredError()) QueryCtx_EmitException();
	else _ResultSet_ReplayStats(set->ctx, set); // Otherwise, the last response is query statistics.
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
