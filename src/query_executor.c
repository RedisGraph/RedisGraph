/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>
#include "query_executor.h"
#include "util/arr.h"
#include "graph/graph.h"
#include "util/vector.h"
#include "schema/schema.h"
#include "parser/grammar.h"
#include "arithmetic/agg_ctx.h"
#include "graph/entities/node.h"
#include "parser/parser_common.h"
#include "procedures/procedure.h"
#include "arithmetic/repository.h"

static void _inlineProperties(AST *ast) {
	/* Migrate inline filters to WHERE clause. */
	if(!ast->matchNode) return;
	// Vector *entities = ast->matchNode->graphEntities;
	Vector *entities = ast->matchNode->_mergedPatterns;

	char *alias;
	char *property;
	AST_ArithmeticExpressionNode *lhs;
	AST_ArithmeticExpressionNode *rhs;

	/* Foreach entity. */
	for(int i = 0; i < Vector_Size(entities); i++) {
		AST_GraphEntity *entity;
		Vector_Get(entities, i, &entity);

		alias = entity->alias;

		Vector *properties = entity->properties;
		if(properties == NULL) {
			continue;
		}

		/* Foreach property. */
		for(int j = 0; j < Vector_Size(properties); j += 2) {
			SIValue *key;
			SIValue *val;

			// Build the left-hand filter value from the node alias and property
			Vector_Get(properties, j, &key);
			property = key->stringval;
			lhs = New_AST_AR_EXP_VariableOperandNode(alias, property);

			// Build the right-hand filter value from the specified constant
			// TODO can update grammar so that this constant is already an ExpressionNode
			// instead of an SIValue
			Vector_Get(properties, j + 1, &val);
			rhs = New_AST_AR_EXP_ConstOperandNode(*val);

			AST_FilterNode *filterNode = New_AST_PredicateNode(lhs, EQ, rhs);

			/* Create WHERE clause if missing. */
			if(ast->whereNode == NULL) {
				ast->whereNode = New_AST_WhereNode(filterNode);
			} else {
				/* Introduce filter with AND operation. */
				AST_FilterNode *left = ast->whereNode->filters;
				AST_FilterNode *right = filterNode;
				ast->whereNode->filters = New_AST_ConditionNode(left, AND, right);
			}
		}
	}
}

/* Incase procedure call is missing its yield part
 * include procedure outputs. */
static void _inlineProcedureYield(AST_ProcedureCallNode *node) {
	if(node->yield) return;

	ProcedureCtx *proc = Proc_Get(node->procedure);
	if(!proc) return;

	unsigned int output_count = array_len(proc->output);

	node->yield = array_new(char *, output_count);
	for(int i = 0; i < output_count; i++) {
		node->yield = array_append(node->yield, strdup(proc->output[i]->name));
	}
}

/* Shares merge pattern with match clause. */
static void _replicateMergeClauseToMatchClause(AST *ast) {
	assert(ast->mergeNode && !ast->matchNode);

	/* Match node is expecting a vector of vectors,
	 * and so we have to wrap merge graph entities vector
	 * within another vector
	 * wrappedEntities will be freed by match clause. */
	Vector *wrappedEntities = NewVector(Vector *, 1);
	Vector_Push(wrappedEntities, ast->mergeNode->graphEntities);
	ast->matchNode = New_AST_MatchNode(wrappedEntities);
}

/* Create a RETURN clause from CALL clause. */
static void _replicateCallClauseToReturnClause(AST *ast) {
	assert(ast->callNode && !ast->returnNode);

	AST_ProcedureCallNode *call = ast->callNode;
	unsigned int output_len = array_len(call->yield);
	AST_ReturnElementNode **returnElements = array_new(AST_ReturnElementNode *,
													   output_len);

	for(unsigned int i = 0; i < output_len; i++) {
		char *alias = NULL;
		char *property = NULL;
		char *output = call->yield[i];
		AST_ArithmeticExpressionNode *exp = New_AST_AR_EXP_VariableOperandNode(output,
																			   property);
		AST_ReturnElementNode *returnElement = New_AST_ReturnElementNode(exp, alias);
		returnElements = array_append(returnElements, returnElement);
	}

	int distinct = 0;
	ast->returnNode = New_AST_ReturnNode(returnElements, distinct);
}

/* If we have a "RETURN *" clause, populate it with all aliased entities. */
static void _populateReturnAll(AST *ast) {
	// Do nothing if there is no RETURN or an array of return elements already exists.
	if(ast->returnNode == NULL || ast->returnNode->returnElements != NULL) return;
	// Collect all entities from clauses that can introduce entities.
	TrieMap *identifiers = NewTrieMap();
	MatchClause_DefinedEntities(ast->matchNode, identifiers);
	CreateClause_DefinedEntities(ast->createNode, identifiers);
	UnwindClause_DefinedEntities(ast->unwindNode, identifiers);
	WithClause_DefinedEntities(ast->withNode, identifiers);

	char buffer[256];
	// Allocate a new return element array to contain all user-provided aliases
	AST_ReturnElementNode **entities = array_new(AST_ReturnElementNode *,
												 identifiers->cardinality);
	char *ptr;
	tm_len_t len;
	void *value;
	TrieMapIterator *it = TrieMap_Iterate(identifiers, "", 0);
	while(TrieMapIterator_Next(it, &ptr, &len, &value)) {
		AST_GraphEntity *entity = value;
		if(entity->anonymous) continue;

		// Copy each alias string to the stack
		len = MIN(255, len);
		memcpy(buffer, ptr, len);
		buffer[len] = '\0';

		// Create and add a return entity for the alias
		AST_ArithmeticExpressionNode *arNode = New_AST_AR_EXP_VariableOperandNode(
												   buffer, NULL);
		AST_ReturnElementNode *returnEntity = New_AST_ReturnElementNode(arNode, NULL);
		entities = array_append(entities, returnEntity);
	}

	TrieMapIterator_Free(it);
	TrieMap_Free(identifiers, TrieMap_NOP_CB);

	ast->returnNode->returnElements = entities;
}

AST **ParseQuery(const char *query, size_t qLen, char **errMsg) {
	AST **asts = Query_Parse(query, qLen, errMsg);
	if(asts) {
		for(int i = 0; i < array_len(asts); i++) {
			/* Create match clause which will try to match against pattern specified within merge clause. */
			if(asts[i]->mergeNode) _replicateMergeClauseToMatchClause(asts[i]);
			if(asts[i]->callNode) {
				_inlineProcedureYield(asts[i]->callNode);
				// If there's no projection within current AST.
				if(!AST_Projects(asts[i])) _replicateCallClauseToReturnClause(asts[i]);
			}

			AST_NameAnonymousNodes(asts[i]);
			// Mark each alias with a unique ID.
			AST_WithNode *withClause = (i > 0) ? asts[i - 1]->withNode : NULL;
			AST_MapAliasToID(asts[i], withClause);
		}
	}
	return asts;
}

AST_Validation AST_PerformValidations(RedisModuleCtx *ctx, AST **ast) {
	char *reason;
	int ast_count = array_len(ast);
	for(int i = 0; i < ast_count; i++) {
		if(AST_Validate(ast[i], &reason) != AST_VALID) {
			RedisModule_ReplyWithError(ctx, reason);
			free(reason);
			return AST_INVALID;
		}
	}

	/* For the timebeing we do not allow re-definition of identifiers
	 * for example:
	 * MATCH (p) WITH max(p.v) AS maximum MATCH (p) RETURN p
	 * TODO: lift this restriction. */
	// char *redefined_identifier = NULL;
	// if(ast_count > 1) {
	//     TrieMap *global_identifiers = AST_Identifiers(ast[0]);
	//     for(int i = 1; i < ast_count; i++) {
	//         TrieMap *local_identifiers = AST_Identifiers(ast[i]);
	//         TrieMapIterator *it = TrieMap_Iterate(local_identifiers, "", 0);
	//         void *v;
	//         tm_len_t len;
	//         char *identifier;
	//         while(TrieMapIterator_Next(it, &identifier, &len, &v)) {
	//             if(!TrieMap_Add(global_identifiers, identifier, len, NULL, TrieMap_DONT_CARE_REPLACE)) {
	//                 redefined_identifier = rm_malloc(sizeof(char) * (len+1));
	//                 memcpy(redefined_identifier, identifier, len);
	//                 redefined_identifier[len] = '\0';
	//                 break;
	//             }
	//         }
	//         TrieMapIterator_Free(it);
	//         TrieMap_Free(local_identifiers, TrieMap_NOP_CB);
	//         if(redefined_identifier) break;
	//     }
	//     TrieMap_Free(global_identifiers, TrieMap_NOP_CB);

	//     if(redefined_identifier) {
	//         asprintf(&reason, "Identifier '%s' defined more than once", redefined_identifier);
	//         RedisModule_ReplyWithError(ctx, reason);
	//         rm_free(redefined_identifier);
	//         free(reason);
	//         return AST_INVALID;
	//     }
	// }

	return AST_VALID;
}

/* Counts the number of right to left edges,
 * if it's greater than half the number of edges in pattern
 * return true.*/
static bool _AST_should_reverse_pattern(Vector *pattern) {
	int transposed = 0; // Number of transposed edges
	int edge_count = 0; // Total number of edges.
	int pattern_length = Vector_Size(pattern);

	// Count how many edges are going right to left.
	for(int i = 0; i < pattern_length; i++) {
		AST_GraphEntity *match_element;
		Vector_Get(pattern, i, &match_element);

		if(match_element->t != N_LINK) continue;

		edge_count++;
		AST_LinkEntity *edge = (AST_LinkEntity *)match_element;
		if(edge->direction ==  N_RIGHT_TO_LEFT) transposed++;
	}

	// No edges.
	if(edge_count == 0) return false;
	return (transposed > edge_count / 2);
}

/* Construct a new MATCH clause by cloning the current one
 * and reversing traversal patterns to reduce matrix transpose
 * operation. */
static void _AST_reverse_match_patterns(AST *ast) {
	size_t pattern_count = Vector_Size(ast->matchNode->patterns);
	Vector *patterns = NewVector(Vector *, pattern_count);

	for(int i = 0; i < pattern_count; i++) {
		Vector *pattern;
		Vector_Get(ast->matchNode->patterns, i, &pattern);

		size_t pattern_length = Vector_Size(pattern);
		Vector *v = NewVector(AST_GraphEntity *, pattern_length);

		if(!_AST_should_reverse_pattern(pattern)) {
			// No need to reverse, simply clone pattern.
			for(int j = 0; j < pattern_length; j++) {
				AST_GraphEntity *e;
				Vector_Get(pattern, j, &e);
				e = Clone_AST_GraphEntity(e);
				Vector_Push(v, e);
			}
		} else {
			/* Reverse pattern:
			 * Create a new pattern where edges been reversed.
			 * Nodes should be introduced in reverse order:
			 * (C)<-[B]-(A)
			 * (A)-[B]->(C) */
			for(int j = pattern_length - 1; j >= 0; j--) {
				AST_GraphEntity *e;
				Vector_Get(pattern, j, &e);
				e = Clone_AST_GraphEntity(e);

				if(e->t == N_LINK) {
					AST_LinkEntity *l = (AST_LinkEntity *)e;
					// Reverse pattern.
					if(l->direction == N_RIGHT_TO_LEFT) l->direction = N_LEFT_TO_RIGHT;
					else l->direction = N_RIGHT_TO_LEFT;
				}
				Vector_Push(v, e);
			}
		}
		Vector_Push(patterns, v);
		Vector_Clear(
			pattern); // We're reusing the original entities, so don't free them.
	}

	Vector_Free(ast->matchNode->_mergedPatterns);
	Vector_Free(ast->matchNode->patterns);
	free(ast->matchNode);

	// Update AST MATCH clause.
	ast->matchNode = New_AST_MatchNode(patterns);
}

static void _AST_optimize_traversal_direction(AST *ast) {
	/* Inspect each MATCH pattern,
	 * see if the number of edges going from right to left ()<-[]-()
	 * is greater than the number of edges going from left to right ()-[]->()
	 * in which case it's worth reversing the pattern to reduce
	 * matrix transpose operations. */

	bool should_reverse = false;
	size_t pattern_count = Vector_Size(ast->matchNode->patterns);
	for(int i = 0; i < pattern_count; i++) {
		Vector *pattern;
		Vector_Get(ast->matchNode->patterns, i, &pattern);

		if(_AST_should_reverse_pattern(pattern)) {
			should_reverse = true;
			break;
		}
	}

	if(should_reverse) _AST_reverse_match_patterns(ast);
}

void ModifyAST(AST **ast) {
	for(int i = 0; i < array_len(ast); i++) {
		if(ast[i]->matchNode) _AST_optimize_traversal_direction(ast[i]);
		_inlineProperties(ast[i]);
	}
	// Only the final AST may contain a RETURN clause
	AST *final_ast = ast[array_len(ast) - 1];
	_populateReturnAll(final_ast);
}
