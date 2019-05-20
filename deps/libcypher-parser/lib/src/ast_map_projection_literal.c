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


struct map_projection_literal
{
    cypher_astnode_t _astnode;
    const cypher_astnode_t *prop_name;
    const cypher_astnode_t *expression;
};


static ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size);


static const struct cypher_astnode_vt *parents[] =
    { &cypher_map_projection_selector_astnode_vt };

const struct cypher_astnode_vt cypher_map_projection_literal_astnode_vt =
    { .parents = parents,
      .nparents = 1,
      .name = "literal projection",
      .detailstr = detailstr,
      .free = cypher_astnode_free };


cypher_astnode_t *cypher_ast_map_projection_literal(
        const cypher_astnode_t *prop_name, const cypher_astnode_t *expression,
        cypher_astnode_t **children, unsigned int nchildren,
        struct cypher_input_range range)
{
    REQUIRE_TYPE(prop_name, CYPHER_AST_PROP_NAME, NULL);
    REQUIRE_TYPE(expression, CYPHER_AST_EXPRESSION, NULL);

    struct map_projection_literal *node =
            calloc(1, sizeof(struct map_projection_literal));
    if (node == NULL)
    {
        return NULL;
    }
    if (cypher_astnode_init(&(node->_astnode),
            CYPHER_AST_MAP_PROJECTION_LITERAL, children, nchildren, range))
    {
        free(node);
        return NULL;
    }
    node->prop_name = prop_name;
    node->expression = expression;
    return &(node->_astnode);
}


const cypher_astnode_t *cypher_ast_map_projection_literal_get_prop_name(
        const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_MAP_PROJECTION_LITERAL, NULL);
    struct map_projection_literal *node =
            container_of(astnode, struct map_projection_literal, _astnode);
    return node->prop_name;
}


const cypher_astnode_t *cypher_ast_map_projection_literal_get_expression(
        const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_MAP_PROJECTION_LITERAL, NULL);
    struct map_projection_literal *node =
            container_of(astnode, struct map_projection_literal, _astnode);
    return node->expression;
}


ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size)
{
    REQUIRE_TYPE(self, CYPHER_AST_MAP_PROJECTION_LITERAL, -1);
    struct map_projection_literal *node =
            container_of(self, struct map_projection_literal, _astnode);
    return snprintf(str, size, "@%u: @%u", node->prop_name->ordinal,
            node->expression->ordinal);
}
