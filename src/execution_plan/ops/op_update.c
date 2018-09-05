/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_update.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Forward declarations. */
void _OpUpdate_BuildUpdateEvalCtx(OpUpdate* op, AST_SetNode *setNode, QueryGraph *q);

OpBase* NewUpdateOp(RedisModuleCtx *ctx, AST_Query *ast, QueryGraph *q, ResultSet *result_set, const char *graphName) {
    OpUpdate* op_update = calloc(1, sizeof(OpUpdate));
    op_update->ast = ast;
    op_update->ctx = ctx;
    op_update->graphName = graphName;
    op_update->result_set = result_set;
    op_update->update_expressions = NULL;
    op_update->update_expressions_count = 0;
    op_update->entities_to_update_count = 0;
    op_update->entities_to_update_cap = 16; /* 16 seems reasonable number to start with. */
    op_update->entities_to_update = malloc(sizeof(EntityUpdateEvalCtx) * op_update->entities_to_update_cap);

    _OpUpdate_BuildUpdateEvalCtx(op_update, ast->setNode, q);

    // Set our Op operations
    OpBase_Init(&op_update->op);
    op_update->op.name = "Update";
    op_update->op.type = OPType_CREATE;
    op_update->op.consume = OpUpdateConsume;
    op_update->op.reset = OpUpdateReset;
    op_update->op.free = OpUpdateFree;

    return (OpBase*)op_update;
}

/* Build an evaluation tree foreach update expression.
 * Save each eval tree and updated graph entity within 
 * the update op. */
void _OpUpdate_BuildUpdateEvalCtx(OpUpdate* op, AST_SetNode *setNode, QueryGraph *graph) {
    op->update_expressions_count = Vector_Size(setNode->set_elements);
    op->update_expressions = malloc(sizeof(EntityUpdateEvalCtx) * op->update_expressions_count);

    for(int i = 0; i < op->update_expressions_count; i++) {
        AST_SetElement *element;
        Vector_Get(setNode->set_elements, i, &element);

        /* Get a reference to the updated entity. */
        op->update_expressions[i].entity = QueryGraph_GetEntityRef(graph, element->entity->alias);
        op->update_expressions[i].alias = strdup(element->entity->alias);
        op->update_expressions[i].property = strdup(element->entity->property);
        op->update_expressions[i].exp = AR_EXP_BuildFromAST(element->exp, graph);
    }
}

/* Until the execution plan will avoid visiting the same entity 
 * more than once, we'll have to delay updates until all entities 
 * are processed, and so _OpUpdate_QueueUpdate will queue up 
 * all information necessary to perform an update. */
void _OpUpdate_QueueUpdate(OpUpdate *op, EntityUpdateEvalCtx *eval_ctx, EntityProperty *dest_entity_prop, SIValue new_value) {
    /* Make sure we've got enough room in queue. */
    if(op->entities_to_update_count == op->entities_to_update_cap) {
        op->entities_to_update_cap *= 2;
        op->entities_to_update = realloc(op->entities_to_update, 
                                         op->entities_to_update_cap * sizeof(EntityUpdateEvalCtx));
    }

    int i = op->entities_to_update_count;
    op->entities_to_update[i].alias = eval_ctx->alias;
    op->entities_to_update[i].id = (*eval_ctx->entity)->id;
    op->entities_to_update[i].dest_entity_prop = dest_entity_prop;
    op->entities_to_update[i].new_value = new_value;
    op->entities_to_update_count++;
}

OpResult OpUpdateConsume(OpBase *opBase, QueryGraph* graph) {
    OpUpdate *op = (OpUpdate*)opBase;
    OpBase *child = op->op.children[0];
    OpResult res = child->consume(child, graph);
    if(res != OP_OK) return res;

    /* Evaluate each update expression and store result 
     * for later execution. */
    EntityUpdateEvalCtx *update_expression = op->update_expressions;
    for(int i = 0; i < op->update_expressions_count; i++, update_expression++) {
        SIValue new_value = AR_EXP_Evaluate(update_expression->exp);
        /* Find ref to property. */
        GraphEntity *entity = *update_expression->entity;
        int j = 0;
        for(; j < entity->prop_count; j++) {
            if(strcmp(entity->properties[j].name, update_expression->property) == 0) {
                _OpUpdate_QueueUpdate(op, update_expression, &entity->properties[j], new_value);
                break;
            }
        }

        if(j == entity->prop_count) {
            /* Property does not exists for entity, create it.
             * For the time being set the new property value to PROPERTY_NOTFOUND.
             * Once we commit the update, we'll set the actual value. */
            GraphEntity_Add_Properties(entity, 1, &update_expression->property, PROPERTY_NOTFOUND);
            _OpUpdate_QueueUpdate(op, update_expression, &entity->properties[entity->prop_count-1], new_value);
        }
    }

    return OP_OK;
}

OpResult OpUpdateReset(OpBase *ctx) {
    return OP_OK;
}

/* Updates indices for all entities getting modified. This must be invoked
 * before _UpdateEntities, while we still have access to the original property. */
void _UpdateIndices(OpUpdate *op) {
    for(int i = 0; i < op->entities_to_update_count; i++) {
        // Locate node label
        char *alias = op->entities_to_update[i].alias;
        AST_GraphEntity* ge = MatchClause_GetEntity(op->ast->matchNode, alias);
        // Only interested in nodes, not edges
        if (ge->t != N_ENTITY) continue;

        LabelStore *store = LabelStore_Get(op->ctx, STORE_NODE, op->graphName, ge->label);
        if (!store) continue;

        EntityProperty *dest_entity_prop = op->entities_to_update[i].dest_entity_prop;
        SIValue new_value = op->entities_to_update[i].new_value;

        NodeID id = op->entities_to_update[i].id;
        Indices_UpdateNode(op->ctx, store, op->graphName, id, dest_entity_prop, &new_value);
    }
}

/* Executes delayed updates. */
void _UpdateEntities(OpUpdate *op) {
    for(int i = 0; i < op->entities_to_update_count; i++) {
        EntityProperty *dest_entity_prop = op->entities_to_update[i].dest_entity_prop;
        SIValue new_value = op->entities_to_update[i].new_value;
        // TODO free dest_entity_prop->value?
        dest_entity_prop->value = new_value;
    }
    if(op->result_set)
        op->result_set->stats.properties_set = op->entities_to_update_count;
}

/* Update tracked schemas according to set properties.
 * Because SET can be used to introduce new properties
 * We have to update our schemas to track newly created properties. */
void _UpdateSchemas(const OpUpdate *op) {

    AST_SetNode *setNode = op->ast->setNode;
    for(int i = 0; i < op->update_expressions_count; i++) {
        AST_SetElement *setElement;

        Vector_Get(setNode->set_elements, i, &setElement);
        char *entityAlias = setElement->entity->alias;
        char *entityProp = setElement->entity->property;

        /* Locate node label. */
        AST_GraphEntity* ge = MatchClause_GetEntity(op->ast->matchNode, entityAlias);
        char *l = ge->label;
        LabelStoreType t = (ge->t == N_ENTITY) ? STORE_NODE : STORE_EDGE;

        LabelStore *store = LabelStore_Get(op->ctx, t, op->graphName, l);
        if (!store) continue;

        LabelStore *allStore = LabelStore_Get(op->ctx, t, op->graphName, NULL);
        LabelStore_UpdateSchema(store, 1, &entityProp);
        LabelStore_UpdateSchema(allStore, 1, &entityProp);
    }
}

void OpUpdateFree(OpBase *ctx) {
    OpUpdate *op = (OpUpdate*)ctx;
    _UpdateIndices(op);
    _UpdateEntities(op);
    _UpdateSchemas(op);
    
    /* Free each UpdateEval context. */
    for(int i = 0; i < op->update_expressions_count; i++) {
        free(op->update_expressions[i].alias);
        free(op->update_expressions[i].property);
        AR_EXP_Free(op->update_expressions[i].exp);
    }

    free(op->update_expressions);
    free(op->entities_to_update);
}

