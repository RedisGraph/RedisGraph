/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_update.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Forward declarations. */
void _OpUpdate_BuildUpdateEvalCtx(OpUpdate* op, AST_SetNode *setNode);

OpBase* NewUpdateOp(GraphContext *gc, AST_Query *ast, ResultSet *result_set) {
    OpUpdate* op_update = calloc(1, sizeof(OpUpdate));
    op_update->gc = gc;
    op_update->ast = ast;
    op_update->result_set = result_set;
    op_update->update_expressions = NULL;
    op_update->update_expressions_count = 0;
    op_update->entities_to_update_count = 0;
    op_update->entities_to_update_cap = 16; /* 16 seems reasonable number to start with. */
    op_update->entities_to_update = malloc(sizeof(EntityUpdateCtx) * op_update->entities_to_update_cap);

    _OpUpdate_BuildUpdateEvalCtx(op_update, ast->setNode);

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
void _OpUpdate_BuildUpdateEvalCtx(OpUpdate* op, AST_SetNode *setNode) {
    op->update_expressions_count = Vector_Size(setNode->set_elements);
    op->update_expressions = malloc(sizeof(EntityUpdateEvalCtx) * op->update_expressions_count);

    for(int i = 0; i < op->update_expressions_count; i++) {
        AST_SetElement *element;
        Vector_Get(setNode->set_elements, i, &element);

        /* Get a reference to the updated entity. */
        op->update_expressions[i].ge = MatchClause_GetEntity(op->ast->matchNode,
                                                             element->entity->alias);
        op->update_expressions[i].property = element->entity->property;
        op->update_expressions[i].exp = AR_EXP_BuildFromAST(element->exp);
    }
}

/* Until the execution plan will avoid visiting the same entity 
 * more than once, we'll have to delay updates until all entities 
 * are processed, and so _OpUpdate_QueueUpdate will queue up 
 * all information necessary to perform an update. */
void _OpUpdate_QueueUpdate(OpUpdate *op, EntityProperty *dest_entity_prop, SIValue new_value) {
    /* Make sure we've got enough room in queue. */
    if(op->entities_to_update_count == op->entities_to_update_cap) {
        op->entities_to_update_cap *= 2;
        op->entities_to_update = realloc(op->entities_to_update, 
                                         op->entities_to_update_cap * sizeof(EntityUpdateCtx));
    }

    int i = op->entities_to_update_count;
    op->entities_to_update[i].dest_entity_prop = dest_entity_prop;
    op->entities_to_update[i].new_value = new_value;
    op->entities_to_update_count++;
}

/* If a new property was introduced, add it to the schema associated
 * with its label. */
void _UpdateSchema(const OpUpdate *op, EntityUpdateEvalCtx *expression, EntityID id) {
    LabelStore *store = NULL;
    AST_GraphEntity *ge = expression->ge;
    LabelStoreType t = (ge->t == N_ENTITY) ? STORE_NODE : STORE_EDGE;
    if (ge->label) {
        /* If a label was provided on the match clause, retrieve the associated store. */
        // TODO In this scenario, the previous method (updating stores once per
        // distinct update expression) was sufficient. This is rather more work, as it
        // trigger for every entity that receives a new property. Once we implement
        // multi-label, regardless, checking the MATCH clause will not be adequate.
        store = GraphContext_GetStore(op->gc, ge->label, t);
    } else {
        /* Label was not provided; seek the appropriate store from the GraphContext. */
        if (t == STORE_NODE) {
          store = GraphContext_FindNodeLabel(op->gc, id);
        } else {
          // TODO Need to retrieve the relation type store given an edge ID.
          // This might be achievable by accessing the QueryGraph.
        }
    }
    if (store == NULL) return; // Unlabeled entity

    /* Add property to store if not already present. */
    LabelStore_UpdateSchema(store, 1, &expression->property);
}

OpResult OpUpdateConsume(OpBase *opBase, Record *r) {
    OpUpdate *op = (OpUpdate*)opBase;
    OpBase *child = op->op.children[0];
    OpResult res = child->consume(child, r);
    if(res != OP_OK) return res;

    /* Evaluate each update expression and store result 
     * for later execution. */
    EntityUpdateEvalCtx *update_expression = op->update_expressions;
    for(int i = 0; i < op->update_expressions_count; i++, update_expression++) {
        SIValue new_value = AR_EXP_Evaluate(update_expression->exp, *r);
        /* Find ref to property. */
        SIValue entry = Record_GetEntry(*r, update_expression->ge->alias);
        GraphEntity *entity = (GraphEntity*) entry.ptrval;
        int j = 0;
        for(; j < entity->prop_count; j++) {
            if(strcmp(entity->properties[j].name, update_expression->property) == 0) {
                _OpUpdate_QueueUpdate(op, &entity->properties[j], new_value);
                break;
            }
        }

        if(j == entity->prop_count) {
            /* Property does not exists for entity, create it.
             * For the time being set the new property value to PROPERTY_NOTFOUND.
             * Once we commit the update, we'll set the actual value. */
            GraphEntity_Add_Properties(entity, 1, &update_expression->property, PROPERTY_NOTFOUND);
            _OpUpdate_QueueUpdate(op, &entity->properties[entity->prop_count-1], new_value);

            /* Add new property to schema associated with this entity. Updates do not need to be enqueued,
             * as they only use the new property's name and repeated insertions have no effect. */
            _UpdateSchema(op, update_expression, entity->id);
        }
    }

    return OP_OK;
}

OpResult OpUpdateReset(OpBase *ctx) {
    return OP_OK;
}

/* Executes delayed updates. */
void _UpdateEntities(OpUpdate *op) {
    for(int i = 0; i < op->entities_to_update_count; i++) {
        EntityProperty *dest_entity_prop = op->entities_to_update[i].dest_entity_prop;
        SIValue new_value = op->entities_to_update[i].new_value;
        dest_entity_prop->value = new_value;
    }
    if(op->result_set)
        op->result_set->stats.properties_set = op->entities_to_update_count;
}

/* Update the schemas not scoped to specific labels
 * to include any properties that were just introduced
 * by the SET clause. */
void _UpdateGenericSchemas(const OpUpdate *op) {
    EntityUpdateEvalCtx *update_expression;
    for(int i = 0; i < op->update_expressions_count; i++) {
        char *entityAlias = op->update_expressions[i].ge->alias;
        char *entityProp = op->update_expressions[i].property;

        LabelStoreType t = (op->update_expressions[i].ge->t == N_ENTITY) ? STORE_NODE : STORE_EDGE;
        LabelStore *allStore = GraphContext_AllStore(op->gc, t);
        LabelStore_UpdateSchema(allStore, 1, &entityProp);
    }
}

void OpUpdateFree(OpBase *ctx) {
    OpUpdate *op = (OpUpdate*)ctx;
    _UpdateEntities(op);
    _UpdateGenericSchemas(op);
    
    /* Free each update context. */
    for(int i = 0; i < op->update_expressions_count; i++) {
        AR_EXP_Free(op->update_expressions[i].exp);
    }

    free(op->update_expressions);
    free(op->entities_to_update);
}
