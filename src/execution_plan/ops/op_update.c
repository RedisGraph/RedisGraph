/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_update.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Build an evaluation tree foreach update expression.
 * Save each eval tree and updated graph entity within 
 * the update op. */
void _OpUpdate_BuildUpdateEvalCtx(OpUpdate* op, AST *ast) {
    AST_SetNode *setNode = ast->setNode;
    op->update_expressions_count = Vector_Size(setNode->set_elements);
    op->update_expressions = malloc(sizeof(EntityUpdateEvalCtx) * op->update_expressions_count);

    for(int i = 0; i < op->update_expressions_count; i++) {
        AST_SetElement *element;
        /* Get a reference to the entity in the SET clause. */
        Vector_Get(setNode->set_elements, i, &element);

        op->update_expressions[i].property = element->entity->property;
        op->update_expressions[i].exp = AR_EXP_BuildFromAST(ast, element->exp);

        /* Track the parallel AST entity in the MATCH clause. */
        op->update_expressions[i].ge = MatchClause_GetEntity(op->ast->matchNode, element->entity->alias);
        op->update_expressions[i].entityRecIdx = AST_GetAliasID(op->ast, element->entity->alias);
    }
}

/* Until the execution plan will avoid visiting the same entity 
 * more than once, we'll have to delay updates until all entities 
 * are processed, and so _OpUpdate_QueueUpdate will queue up 
 * all information necessary to perform an update. */
void _OpUpdate_QueueUpdate(OpUpdate *op, Entity *entity, AST_GraphEntity *ge, int prop_idx, SIValue new_value) {
    /* Make sure we've got enough room in queue. */
    if(op->entities_to_update_count == op->entities_to_update_cap) {
        op->entities_to_update_cap *= 2;
        op->entities_to_update = realloc(op->entities_to_update, 
                                         op->entities_to_update_cap * sizeof(EntityUpdateCtx));
    }

    int i = op->entities_to_update_count;
    op->entities_to_update[i].entity_reference = entity;
    op->entities_to_update[i].ge = ge;
    op->entities_to_update[i].prop_idx = prop_idx;
    op->entities_to_update[i].new_value = new_value;
    op->entities_to_update_count++;
}

/* Update tracked schemas according to set properties.
 * Because SET can be used to introduce new properties
 * We have to update our schemas to track newly created properties. */
void _UpdateSchemas(const OpUpdate *op) {
    EntityUpdateEvalCtx update_expression;
    for(int i = 0; i < op->update_expressions_count; i++) {
        update_expression = op->update_expressions[i];
        char *entityProp = update_expression.property;

        /* Update store associated with the entity type. */
        AST_GraphEntity *ge = update_expression.ge;
        LabelStoreType t = (ge->t == N_ENTITY) ? STORE_NODE : STORE_EDGE;
        LabelStore *allStore = GraphContext_AllStore(op->gc, t);
        LabelStore_UpdateSchema(allStore, 1, &entityProp);

        // TODO If the match clause does not provide a label, we must update all the stores
        // affected by the SET clause.
        char *l = ge->label;
        if (!l) continue;
        /* Update store associated with the entity label. */
        LabelStore *store = GraphContext_GetStore(op->gc, l, t);
        if (!store) continue;

        LabelStore_UpdateSchema(store, 1, &entityProp);
    }
}

/* Executes delayed updates. */
void _UpdateEntities(OpUpdate *op) {
    EntityUpdateCtx *entity_to_update;
    for(int i = 0; i < op->entities_to_update_count; i++) {
        entity_to_update = &op->entities_to_update[i];
        // Retrieve variables from update context
        Entity *entity = entity_to_update->entity_reference;
        EntityProperty *property = &entity->properties[entity_to_update->prop_idx];
        SIValue new_value = entity_to_update->new_value;

        // Only worry about index updates for nodes right now
        AST_GraphEntity *ge = entity_to_update->ge;
        if (ge->t == N_ENTITY) {
          LabelStore *store = NULL;
          if (ge->label) store = GraphContext_GetStore(op->gc, ge->label, STORE_NODE);
          GraphContext_UpdateNodeIndices(op->gc, store, entity->id, property, &new_value);
        }
        property->value = new_value;
    }
    if(op->result_set)
        op->result_set->stats.properties_set += op->entities_to_update_count;
}

OpBase* NewUpdateOp(GraphContext *gc, AST *ast, ResultSet *result_set) {
    OpUpdate* op_update = calloc(1, sizeof(OpUpdate));
    op_update->gc = gc;
    op_update->ast = ast;
    op_update->result_set = result_set;
    op_update->update_expressions = NULL;
    op_update->update_expressions_count = 0;
    op_update->entities_to_update_count = 0;
    op_update->entities_to_update_cap = 16; /* 16 seems reasonable number to start with. */
    op_update->entities_to_update = malloc(sizeof(EntityUpdateCtx) * op_update->entities_to_update_cap);

    _OpUpdate_BuildUpdateEvalCtx(op_update, ast);

    // Set our Op operations
    OpBase_Init(&op_update->op);
    op_update->op.name = "Update";
    op_update->op.type = OPType_CREATE;
    op_update->op.consume = OpUpdateConsume;
    op_update->op.reset = OpUpdateReset;
    op_update->op.free = OpUpdateFree;

    return (OpBase*)op_update;
}

Record OpUpdateConsume(OpBase *opBase) {
    OpUpdate *op = (OpUpdate*)opBase;
    OpBase *child = op->op.children[0];
    Record r = child->consume(child);
    if(!r) return NULL;

    /* Evaluate each update expression and store result 
     * for later execution. */
    EntityUpdateEvalCtx *update_expression = op->update_expressions;
    for(int i = 0; i < op->update_expressions_count; i++, update_expression++) {
        SIValue new_value = AR_EXP_Evaluate(update_expression->exp, r);
        /* Find ref to property. */
        GraphEntity *entity = Record_GetGraphEntity(r, update_expression->entityRecIdx);
        int j = 0;
        for(; j < ENTITY_PROP_COUNT(entity); j++) {
            if(strcmp(ENTITY_PROPS(entity)[j].name, update_expression->property) == 0) {
                _OpUpdate_QueueUpdate(op, entity->entity, update_expression->ge,
                                      j, new_value);
                break;
            }
        }

        if(j == ENTITY_PROP_COUNT(entity)) {
            /* Property does not exists for entity, create it.
             * For the time being set the new property value to PROPERTY_NOTFOUND.
             * Once we commit the update, we'll set the actual value. */
            GraphEntity_Add_Properties(entity, 1, &update_expression->property, PROPERTY_NOTFOUND);
            _OpUpdate_QueueUpdate(op, entity->entity, update_expression->ge,
                                  ENTITY_PROP_COUNT(entity)-1, new_value);
        }
    }

    return r;
}

OpResult OpUpdateReset(OpBase *ctx) {
    return OP_OK;
}

void OpUpdateFree(OpBase *ctx) {
    OpUpdate *op = (OpUpdate*)ctx;

    /* Lock everything. */
    Graph_AcquireWriteLock(op->gc->g);
    _UpdateEntities(op);
    _UpdateSchemas(op);
    // Release lock.
    Graph_ReleaseLock(op->gc->g);
    
    /* Free each update context. */
    for(int i = 0; i < op->update_expressions_count; i++) {
        AR_EXP_Free(op->update_expressions[i].exp);
    }

    free(op->update_expressions);
    free(op->entities_to_update);
}
