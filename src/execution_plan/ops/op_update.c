/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_update.h"
#include "../../util/rmalloc.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Build an evaluation context foreach update expression. */
static void _BuildUpdateEvalCtx(OpUpdate* op, AST *ast) {
    AST_SetNode *setNode = ast->setNode;
    op->update_expressions_count = Vector_Size(setNode->set_elements);
    op->update_expressions = rm_malloc(sizeof(EntityUpdateEvalCtx) * op->update_expressions_count);

    for(int i = 0; i < op->update_expressions_count; i++) {
        Attribute_ID prop_id;
        AST_GraphEntity *ge;
        AST_SetElement *element;

        /* Get a reference to the entity in the SET clause. */
        Vector_Get(setNode->set_elements, i, &element);
        /* Get a reference to the entity in the MATCH clause. */
        ge = MatchClause_GetEntity(ast->matchNode, element->entity->alias);

        char *property = element->entity->property;
        SchemaType schema_type = (ge->t == N_ENTITY) ? SCHEMA_NODE : SCHEMA_EDGE;

        /* Update operation may create a new attributes if one
         * does not exists, in which case introduce attribute to schema. */
        prop_id = Attribute_GetID(schema_type, property);
        if(prop_id == ATTRIBUTE_NOTFOUND) {
            /* This is a new attribute, add it to schema. */
            Schema *s;
            GraphContext *gc = op->gc;
            if(ge->label) {
                /* Label is specified, add attribute to label schema. */
                 s = GraphContext_GetSchema(gc, ge->label, schema_type);
            }
            else {
                /* Label is not specified add attribute to unified schema. */
                s = GraphContext_GetUnifiedSchema(gc, schema_type);
            }
            /* It might be that query is trying to update 
             * an attribute of a none existing label
             * in which case consume will never be called. */
            if(s) prop_id = Schema_AddAttribute(s, schema_type, property);
        }

        /* Track all required informantion to perform an update. */
        op->update_expressions[i].ge = ge;
        op->update_expressions[i].attribute = property;
        op->update_expressions[i].attribute_idx = prop_id;
        op->update_expressions[i].exp = AR_EXP_BuildFromAST(ast, element->exp);
        op->update_expressions[i].entityRecIdx = AST_GetAliasID(op->ast, element->entity->alias);
    }
}

/* Delay updates until all entities are processed, 
 * _QueueUpdate will queue up all information necessary to perform an update. */
static void _QueueUpdate(OpUpdate *op, Entity *entity, AST_GraphEntity *ge, char *attribute, Attribute_ID prop_idx, SIValue new_value) {
    /* Make sure we've got enough room in queue. */
    if(op->pending_updates_count == op->pending_updates_cap) {
        op->pending_updates_cap *= 2;
        op->pending_updates = rm_realloc(op->pending_updates, 
                                         op->pending_updates_cap * sizeof(EntityUpdateCtx));
    }

    int i = op->pending_updates_count;
    op->pending_updates[i].ge = ge;
    op->pending_updates[i].new_value = new_value;
    op->pending_updates[i].attribute = attribute;
    op->pending_updates[i].attribute_idx = prop_idx;
    op->pending_updates[i].entity_reference = entity;
    op->pending_updates_count++;
}

/* Retrieves schema describing updated entity. */
static Schema* _GetSchema(EntityUpdateCtx *ctx) {
    AST_GraphEntity *ge = ctx->ge;
    Entity *e = ctx->entity_reference;
    GraphContext *gc = GraphContext_GetFromLTS();
    SchemaType t = (ge->t == N_ENTITY) ? SCHEMA_NODE : SCHEMA_EDGE;

    // Try to get schema.
    if(ge->label) return GraphContext_GetSchema(gc, ge->label, t);
    
    /* Label isn't specified, try to get it.
     * In the case of unlabeled edges, for the timebeing we'll work with
     * the unified schema. */
    if(t == SCHEMA_EDGE) return GraphContext_GetUnifiedSchema(gc, t);

    // Unlabeled node.
    int label_id = Graph_GetNodeLabel(gc->g, e->id);

    // Node isn't labeled, use unified schema.
    if(label_id == GRAPH_NO_LABEL) return GraphContext_GetUnifiedSchema(gc, t);

    // Node is labeled, get specified schema.
    return GraphContext_GetSchemaByID(gc, label_id, SCHEMA_NODE);
}

/* Make sure schema is aware of updated attribute. */
static void _UpdateSchema(EntityUpdateCtx *ctx) {
    Schema *s = _GetSchema(ctx);
    assert(s);
    Schema_AddAttribute(s, SCHEMA_NODE, ctx->attribute);
}

/* Introduce updated entity to index. */
static void _UpdateIndex(EntityUpdateCtx *ctx, SIValue *old_value, SIValue *new_value) {
    EntityID node_id = ctx->entity_reference->id;
    Schema *s = _GetSchema(ctx);
    assert(s);

    // See if there's an index on label/property pair.
    Index *idx = Schema_GetIndex(s, ctx->attribute);
    if(!idx) return;

    if(old_value != PROPERTY_NOTFOUND) {
        /* Updating an existing property.
         * remove entity from index using old value. */
        Index_DeleteNode(idx, node_id, old_value);
    }
    
    // Setting an attribute value to NULL remove that attribute.
    if(SIValue_IsNull(*new_value)) return;

    // Add node to index.
    Index_InsertNode(idx, node_id, new_value);
}

/* Executes delayed updates. */
static void _CommitUpdates(OpUpdate *op) {
    EntityUpdateCtx *ctx;

    for(int i = 0; i < op->pending_updates_count; i++) {
        ctx = &op->pending_updates[i];
        
        /* Retrieve GraphEntity:
         * Due to Record freeing we can't maintain the original pointer to GraphEntity object,
         * but only a pointer to an Entity object,
         * to use the GraphEntity_Get, GraphEntity_Add functions we'll use a place holder
         * to hold our entity. */
        GraphEntity graph_entity;
        graph_entity.entity = ctx->entity_reference;

        // Try to get current property value.
        SIValue *old_value = GraphEntity_GetProperty(&graph_entity, ctx->attribute_idx);
        
        // Update index for node entities, edges are not indexed.
        if(ctx->ge->t == N_ENTITY) _UpdateIndex(ctx, old_value, &ctx->new_value);

        if(old_value == PROPERTY_NOTFOUND) {
            // Add new property.
            GraphEntity_AddProperty(&graph_entity, ctx->attribute_idx, ctx->new_value);
            _UpdateSchema(ctx);
        } else {
            // Update property.
            GraphEntity_SetProperty(&graph_entity, ctx->attribute_idx, ctx->new_value);
        }
    }

    if(op->result_set)
        op->result_set->stats.properties_set += op->pending_updates_count;
}

OpBase* NewUpdateOp(GraphContext *gc, AST *ast, ResultSet *result_set) {
    OpUpdate* op_update = calloc(1, sizeof(OpUpdate));
    op_update->gc = gc;
    op_update->ast = ast;
    op_update->result_set = result_set;

    op_update->update_expressions = NULL;
    op_update->update_expressions_count = 0;

    op_update->pending_updates_count = 0;
    op_update->pending_updates_cap = 16; /* 16 seems reasonable number to start with. */
    op_update->pending_updates = rm_malloc(sizeof(EntityUpdateCtx) * op_update->pending_updates_cap);

    // Create a context for each update expression.
    _BuildUpdateEvalCtx(op_update, ast);

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
    // No data, return.
    if(!r) return NULL;

    /* Evaluate each update expression and store result 
     * for later execution. */
    EntityUpdateEvalCtx *update_expression = op->update_expressions;
    for(int i = 0; i < op->update_expressions_count; i++, update_expression++) {
        SIValue new_value = AR_EXP_Evaluate(update_expression->exp, r);
        GraphEntity *entity = Record_GetGraphEntity(r, update_expression->entityRecIdx);
        _QueueUpdate(op,
                     entity->entity,
                     update_expression->ge,
                     update_expression->attribute,
                     update_expression->attribute_idx,
                     new_value);
    }

    return r;
}

OpResult OpUpdateReset(OpBase *ctx) {
    OpUpdate *op = (OpUpdate*)ctx;
    // Reset all pending updates.
    op->pending_updates_count = 0;
    op->pending_updates_cap = 16; /* 16 seems reasonable number to start with. */
    op->pending_updates = rm_realloc(op->pending_updates,
                                     op->pending_updates_cap * sizeof(EntityUpdateCtx));
    return OP_OK;
}

void OpUpdateFree(OpBase *ctx) {
    OpUpdate *op = (OpUpdate*)ctx;

    /* Lock everything. */
    Graph_AcquireWriteLock(op->gc->g);
    _CommitUpdates(op);
    // Release lock.
    Graph_ReleaseLock(op->gc->g);
    
    /* Free each update context. */
    for(int i = 0; i < op->update_expressions_count; i++) {
        AR_EXP_Free(op->update_expressions[i].exp);
    }

    rm_free(op->update_expressions);
    rm_free(op->pending_updates);
}
