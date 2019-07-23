/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./call.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"
#include "../ast_common.h"
#include "../../procedures/procedure.h"

AST_ProcedureCallNode *New_AST_ProcedureCallNode(char *procedure,
        char **arguments, char **yield) {
	AST_ProcedureCallNode *procCall = rm_malloc(sizeof(AST_ProcedureCallNode));
	procCall->procedure = procedure;
	procCall->arguments = arguments;
	procCall->yield = yield;
	return procCall;
}

void ProcedureCallClause_ReferredEntities(const AST_ProcedureCallNode *node,
        TrieMap *referred_entities) {
	// Procedure call doesn't refers to any entities.
	return;
}

void ProcedureCallClause_DefinedEntities(const AST_ProcedureCallNode *node,
        TrieMap *definedEntities) {
	if(!node) return;

	ProcedureCtx *proc = Proc_Get(node->procedure);
	if(!proc) return;

	for(int i = 0; i < array_len(node->yield); i++) {
		for(int j = 0; j < array_len(proc->output); j++) {
			ProcedureOutput *out = proc->output[j];
			if(strcmp(node->yield[i], out->name) == 0) {
				// TODO: leak!
				AST_GraphEntity *entity = rm_calloc(1, sizeof(AST_GraphEntity));
				switch(out->type) {
				case T_NODE:
					entity->t = N_ENTITY;
					entity->alias = out->name;
					break;
				case T_EDGE:
					entity->t = N_LINK;
					entity->alias = out->name;
					break;
				default:
					entity->t = N_SCALAR;
					entity->alias = out->name;
					break;
				}

				TrieMap_Add(definedEntities,
				            node->yield[i],
				            strlen(node->yield[i]),
				            entity,
				            TrieMap_DONT_CARE_REPLACE);
				break;
			}
		}
	}
}

void Free_AST_ProcedureCallNode(AST_ProcedureCallNode *node) {
	if(!node) return;

	free(node->procedure);

	if(node->arguments) {
		for(int i = 0; i < array_len(node->arguments); i++) free(node->arguments[i]);
		array_free(node->arguments);
	}

	if(node->yield) {
		for(int i = 0; i < array_len(node->yield); i++) free(node->yield[i]);
		array_free(node->yield);
	}

	rm_free(node);
}
