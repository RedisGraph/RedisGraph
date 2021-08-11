/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "resultset.h"
#include "RG.h"
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

static void _ResultSet_ReplyWithPreamble(ResultSet *set) {
	if(set->column_count > 0) {
		// prepare a response containing a header, records, and statistics
		RedisModule_ReplyWithArray(set->ctx, 3);
		// emit the table header using the appropriate formatter
		set->formatter->EmitHeader(set->ctx, set->columns, set->columns_record_map);
	} else {
		// prepare a response containing only statistics
		RedisModule_ReplyWithArray(set->ctx, 1);
	}
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

ResultSet *NewResultSet(RedisModuleCtx *ctx, ResultSetFormatterType format) {
	ResultSet *set = rm_malloc(sizeof(ResultSet));
	set->ctx = ctx;
	set->gc = QueryCtx_GetGraphCtx();
	set->format = format;
	set->formatter = ResultSetFormatter_GetFormatter(format);
	set->columns = NULL;
	set->column_count = 0;
	set->columns_record_map = NULL;
	set->cells = DataBlock_New(32, sizeof(SIValue), NULL);

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

uint64_t ResultSet_RowCount(const ResultSet *set) {
	ASSERT(set != NULL);
	if(set->column_count == 0) return 0;
	return DataBlock_ItemCount(set->cells) / set->column_count;
}

void _ResultSet_ConsumeRecord(ResultSet *set, Record r) {
	for(int i = 0; i < set->column_count; i++) {
		int idx = set->columns_record_map[i];
		SIValue *cell = DataBlock_AllocateItem(set->cells, NULL);
		*cell = Record_Get(r, idx);
		SIValue_Persist(cell);
	}

	// remove entry from record in a second pass
	// this will ensure duplicated projections are not removed
	// too early, consider: MATCH (a) RETURN max(a.val), max(a.val)
	for(int i = 0; i < set->column_count; i++) {
		int idx = set->columns_record_map[i];
		Record_Remove(r, idx);
	}
}

int ResultSet_AddRecord(ResultSet *set, Record r) {
	// if result-set format is NOP, don't process record
	if(set->format == FORMATTER_NOP) return RESULTSET_OK;

	// if this is the first Record encountered, map columns to record indices
	if(DataBlock_ItemCount(set->cells) == 0) ResultSet_MapProjection(set, r);

	_ResultSet_ConsumeRecord(set, r);

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

inline void ResultSet_CachedExecution(ResultSet *set, bool cached) {
	set->stats.cached = cached;
}

void ResultSet_Reply(ResultSet *set) {
	if(set == NULL) return;

	uint64_t row_count = ResultSet_RowCount(set);
	/* Check to see if we've encountered a run-time error.
	 * If so, emit it as the only response. */
	if(ErrorCtx_EncounteredError()) {
		ErrorCtx_EmitException();
		return;
	}

	// Set up the results array and emit the header if the query requires one.
	_ResultSet_ReplyWithPreamble(set);

	// Emit the records cached in the result set.
	if(set->column_count > 0) {
		RedisModule_ReplyWithArray(set->ctx, row_count);
		SIValue *row[set->column_count];
		uint64_t cells = DataBlock_ItemCount(set->cells);
		for(uint64_t i = 0; i < cells; i += set->column_count) {
			for(uint j = 0; j < set->column_count; j++) {
				row[j] = DataBlock_GetItem(set->cells, i + j);
			}

			set->formatter->EmitRow(set->ctx, set->gc, row, set->column_count);

			for(uint j = 0; j < set->column_count; j++) SIValue_Free(*row[j]);
		}
	}

	_ResultSet_ReplayStats(set->ctx, set); // The last response is query statistics.
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
	if(set->cells) DataBlock_Free(set->cells);

	rm_free(set);
}

