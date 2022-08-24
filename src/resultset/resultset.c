/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
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

static void _ResultSet_ReplyWithPreamble
(
	ResultSet *set
) {
	if(set->column_count > 0) {
		// prepare a response containing a header, records, and statistics
		RedisModule_ReplyWithArray(set->ctx, 3);
		// emit the table header using the appropriate formatter
		set->formatter->EmitHeader(set->ctx, set->columns);
	} else {
		// prepare a response containing only statistics
		RedisModule_ReplyWithArray(set->ctx, 1);
	}
}

static void _ResultSet_SetColumns
(
	ResultSet *set
) {
	ASSERT(set->columns == NULL);

	AST *ast = QueryCtx_GetAST();
	const cypher_astnode_type_t root_type = cypher_astnode_type(ast->root);

	if(root_type == CYPHER_AST_QUERY) {
		uint clause_count = cypher_ast_query_nclauses(ast->root);

		const cypher_astnode_t *last_clause =
			cypher_ast_query_get_clause(ast->root, clause_count - 1);

		cypher_astnode_type_t last_clause_type =
			cypher_astnode_type(last_clause);

		if(last_clause_type == CYPHER_AST_RETURN) {
			set->columns = AST_BuildReturnColumnNames(last_clause);
		} else if(last_clause_type == CYPHER_AST_CALL) {
			set->columns = AST_BuildCallColumnNames(last_clause);
		}

		set->column_count = array_len(set->columns);
	}
}

// create a new result set
ResultSet *NewResultSet
(
	RedisModuleCtx *ctx,
	ResultSetFormatterType format  // resultset format
) {
	ResultSet *set = rm_malloc(sizeof(ResultSet));

	set->gc                  =  QueryCtx_GetGraphCtx();
	set->ctx                 =  ctx;
	set->format              =  format;
	set->columns             =  NULL;
	set->formatter           =  ResultSetFormatter_GetFormatter(format);
	set->column_count        =  0;
	set->formatter_pdata     =  set->formatter->CreatePData();
	set->columns_record_map  =  NULL;

	// init resultset statistics
	ResultSetStat_init(&set->stats);

	// create resultset columns
	if(set->format != FORMATTER_NOP) {
		_ResultSet_SetColumns(set);
	}

	// allocate space for resultset entries only if data is expected
	if(set->column_count > 0) {
		// none empty result-set
		// allocate formatter's private data
		set->formatter_pdata = set->formatter->CreatePData();
	}

	return set;
}

// map each column to a record index
// such that when resolving resultset row i column j we'll extract
// data from record at position columns_record_map[j]
void ResultSet_MapProjection
(
	ResultSet *set,  // resultset to init mappings for
	rax *mapping     // mapping
) {
	ASSERT(set     != NULL);
	ASSERT(mapping != NULL);

	if(set->columns_record_map == NULL) {
		set->columns_record_map = rm_malloc(sizeof(uint) * set->column_count);
	}

	for(uint i = 0; i < set->column_count; i++) {
		const char *column = set->columns[i];
		void *idx = raxFind(mapping, (unsigned char *)column, strlen(column));
		ASSERT(idx != raxNotFound);
		set->columns_record_map[i] = (intptr_t)idx;
	}
}

// add a new row to resultset
int ResultSet_AddRecord
(
	ResultSet *set,  // resultset to extend
	Record r         // record containing projected data
) {
	ASSERT(r   != NULL);
	ASSERT(set != NULL);

	// TODO: arrange for the input record to contain
	// projected values in indices 0..column_count
	// this way we won't have to use a columns_record_map
	set->formatter->ProcessRow(r, set->columns_record_map, set->column_count,
			set->formatter_pdata);

	return RESULTSET_OK;
}

// update resultset index creation statistics
void ResultSet_IndexCreated
(
	ResultSet *set,  // resultset to update
	int status_code  // index creation status code
) {
	ASSERT(set != NULL);
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

// update resultset index deleted statistics
void ResultSet_IndexDeleted
(
	ResultSet *set,  // resultset to update
	int status_code  // index deletion status code
) {
	ASSERT(set != NULL);

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

// update resultset cache execution statistics
void ResultSet_CachedExecution
(
	ResultSet *set  // resultset to update
) {
	ASSERT(set != NULL);
	set->stats.cached = true;
}

// flush resultset to network
void ResultSet_Reply
(
	ResultSet *set  // resultset to reply with
) {
	ASSERT(set != NULL);

	// check to see if we've encountered a run-time error
	// if so, emit it as the only response
	if(ErrorCtx_EncounteredError()) {
		ErrorCtx_EmitException();
		return;
	}

	// set up the results array and emit the header if the query requires one
	_ResultSet_ReplyWithPreamble(set);

	// emit resultset
	if(set->column_count > 0) {
		set->formatter->EmitData(set->ctx, set->gc, set->column_count,
			set->formatter_pdata);
	}

	// response with statistics
	ResultSetStat_emit(set->ctx, &set->stats);
}

void ResultSet_Clear(ResultSet *set) {
	ASSERT(set != NULL);
	ResultSetStat_Clear(&set->stats);
}

// free resultset
void ResultSet_Free
(
	ResultSet *set  // resultset to free
) {
	if(set == NULL) return;

	if(set->columns) {
		array_free(set->columns);
	}

	if(set->columns_record_map) {
		rm_free(set->columns_record_map);
	}

	// free formatter's private data
	set->formatter->FreePData(set->formatter_pdata);

	rm_free(set);
}

