/* vi:set ts=4 sw=4 expandtab:
 *
 * Copyright 2016, Chris Leishman (http://github.com/cleishm)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "../../config.h"
#include "astnode.h"
#include "util.h"
#include <assert.h>


struct constraint
{
    cypher_astnode_t _astnode;
    const cypher_astnode_t *identifier;
    const cypher_astnode_t *label;
    const cypher_astnode_t *expression;
    bool unique;
};


static ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size);


static const struct cypher_astnode_vt *parents[] =
    { &cypher_schema_command_astnode_vt };

const struct cypher_astnode_vt cypher_create_node_prop_constraint_astnode_vt =
    { .parents = parents,
      .nparents = 1,
      .name = "create node prop constraint",
      .detailstr = detailstr,
      .free = cypher_astnode_free };


cypher_astnode_t *cypher_ast_create_node_prop_constraint(
        const cypher_astnode_t *identifier, const cypher_astnode_t *label,
        const cypher_astnode_t *expression, bool unique,
        cypher_astnode_t **children, unsigned int nchildren,
        struct cypher_input_range range)
{
    REQUIRE_TYPE(identifier, CYPHER_AST_IDENTIFIER, NULL);
    REQUIRE_TYPE(label, CYPHER_AST_LABEL, NULL);
    REQUIRE_TYPE(expression, CYPHER_AST_EXPRESSION, NULL);

    struct constraint *node = calloc(1, sizeof(struct constraint));
    if (node == NULL)
    {
        return NULL;
    }
    if (cypher_astnode_init(&(node->_astnode),
            CYPHER_AST_CREATE_NODE_PROP_CONSTRAINT, children, nchildren, range))
    {
        free(node);
        return NULL;
    }
    node->identifier = identifier;
    node->label = label;
    node->expression = expression;
    node->unique = unique;
    return &(node->_astnode);
}


const cypher_astnode_t *cypher_ast_create_node_prop_constraint_get_identifier(
        const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_CREATE_NODE_PROP_CONSTRAINT, NULL);
    struct constraint *node = container_of(astnode, struct constraint, _astnode);
    return node->identifier;
}


const cypher_astnode_t *cypher_ast_create_node_prop_constraint_get_label(
        const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_CREATE_NODE_PROP_CONSTRAINT, NULL);
    struct constraint *node = container_of(astnode, struct constraint, _astnode);
    return node->label;
}


const cypher_astnode_t *cypher_ast_create_node_prop_constraint_get_expression(
        const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_CREATE_NODE_PROP_CONSTRAINT, NULL);
    struct constraint *node = container_of(astnode, struct constraint, _astnode);
    return node->expression;
}


bool cypher_ast_create_node_prop_constraint_is_unique(
        const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_CREATE_NODE_PROP_CONSTRAINT, false);
    struct constraint *node = container_of(astnode, struct constraint, _astnode);
    return node->unique;
}


ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size)
{
    REQUIRE_TYPE(self, CYPHER_AST_CREATE_NODE_PROP_CONSTRAINT, -1);
    struct constraint *node = container_of(self, struct constraint, _astnode);

    return snprintf(str, size, "ON=(@%u:@%u), expression=@%u%s",
            node->identifier->ordinal, node->label->ordinal,
            node->expression->ordinal, node->unique? ", IS UNIQUE" : "");
}
