#include "op_update.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Forward declarations. */
void _OpUpdate_BuildUpdateEvalCtx(OpUpdate* op, AST_SetNode *ast, QueryGraph *graph);

OpBase* NewUpdateOp(AST_SetNode *ast, QueryGraph *graph, ResultSet *result_set) {
    OpUpdate* op_update = calloc(1, sizeof(OpUpdate));
    op_update->result_set = result_set;
    op_update->request_refresh = 1;
    op_update->update_expressions = NULL;
    op_update->update_expressions_count = 0;
    op_update->entities_to_update_count = 0;
    op_update->entities_to_update_cap = 16; /* 16 seems reasonable number to start with. */
    op_update->entities_to_update = malloc(sizeof(EntityUpdateEvalCtx) * op_update->entities_to_update_cap);

    _OpUpdate_BuildUpdateEvalCtx(op_update, ast, graph);

    // Set our Op operations
    op_update->op.name = "Update";
    op_update->op.type = OPType_CREATE;
    op_update->op.consume = OpUpdateConsume;
    op_update->op.reset = OpUpdateReset;
    op_update->op.free = OpUpdateFree;
    op_update->op.modifies = NULL;
    return (OpBase*)op_update;
}

/* Build an evaluation tree foreach update expression.
 * Save each eval tree and updated graph entity within 
 * the update op. */
void _OpUpdate_BuildUpdateEvalCtx(OpUpdate* op, AST_SetNode *ast, QueryGraph *graph) {
    op->update_expressions_count = Vector_Size(ast->set_elements);
    op->update_expressions = malloc(sizeof(EntityUpdateEvalCtx) * op->update_expressions_count);

    for(int i = 0; i < op->update_expressions_count; i++) {
        AST_SetElement *element;
        Vector_Get(ast->set_elements, i, &element);

        /* Get a reference to the updated entity. */
        op->update_expressions[i].entity = QueryGraph_GetEntityRef(graph, element->entity->alias);
        op->update_expressions[i].property = strdup(element->entity->property);
        op->update_expressions[i].exp = AR_EXP_BuildFromAST(element->exp, graph);
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
                                         op->entities_to_update_cap * sizeof(EntityUpdateEvalCtx));
    }

    int i = op->entities_to_update_count;
    op->entities_to_update[i].dest_entity_prop = dest_entity_prop;
    op->entities_to_update[i].new_value = new_value;
    op->entities_to_update_count++;
}

OpResult OpUpdateConsume(OpBase *opBase, QueryGraph* graph) {
    OpUpdate *op = (OpUpdate*)opBase;

    if(op->request_refresh) {
        op->request_refresh = 0;
        return OP_REFRESH;
    }

    /* Evaluate each update expression and store result 
     * for later execution. */
    EntityUpdateEvalCtx *update_expressions = op->update_expressions;
    for(int i = 0; i < op->update_expressions_count; i++, update_expressions++) {
        SIValue new_value = AR_EXP_Evaluate(update_expressions->exp);
        /* Find ref to property. */
        GraphEntity *entity = *update_expressions->entity;
        for(int j = 0; j < entity->prop_count; j++) {
            if(strcmp(entity->properties[j].name, update_expressions->property) == 0) {
                _OpUpdate_QueueUpdate(op, &entity->properties[j], new_value);
                break;
            }
        }
    }

    /* TODO: handle case where property is missing from entity,
     * should we add a new property? YES! */    
    op->request_refresh = 1;
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
    op->result_set->properties_set = op->entities_to_update_count;
}

void OpUpdateFree(OpBase *ctx) {
    OpUpdate *op = (OpUpdate*)ctx;
    _UpdateEntities(op);
    
    /* Free each update context. */
    for(int i = 0; i < op->update_expressions_count; i++) {
        free(op->update_expressions[i].property);
        AR_EXP_Free(op->update_expressions[i].exp);
    }

    free(op->update_expressions);
    free(op->entities_to_update);
    free(op);
}
