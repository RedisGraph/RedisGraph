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


struct projection
{
    cypher_astnode_t _astnode;
    const cypher_astnode_t *expression;
    const cypher_astnode_t *alias;
};


static ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size);


const struct cypher_astnode_vt cypher_projection_astnode_vt =
    { .name = "projection",
      .detailstr = detailstr,
      .free = cypher_astnode_free };


cypher_astnode_t *cypher_ast_projection(const cypher_astnode_t *expression,
        const cypher_astnode_t *alias, cypher_astnode_t **children,
        unsigned int nchildren, struct cypher_input_range range)
{
    REQUIRE_TYPE(expression, CYPHER_AST_EXPRESSION, NULL);
    REQUIRE_TYPE_OPTIONAL(alias, CYPHER_AST_IDENTIFIER, NULL);

    struct projection *node = calloc(1, sizeof(struct projection));
    if (node == NULL)
    {
        return NULL;
    }
    if (cypher_astnode_init(&(node->_astnode), CYPHER_AST_PROJECTION,
            children, nchildren, range))
    {
        free(node);
        return NULL;
    }
    node->expression = expression;
    node->alias = alias;
    return &(node->_astnode);
}


const cypher_astnode_t *cypher_ast_projection_get_expression(
        const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_PROJECTION, NULL);
    struct projection *node = container_of(astnode, struct projection, _astnode);
    return node->expression;
}


const cypher_astnode_t *cypher_ast_projection_get_alias(
        const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_PROJECTION, NULL);
    struct projection *node = container_of(astnode, struct projection, _astnode);
    return node->alias;
}


ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size)
{
    REQUIRE_TYPE(self, CYPHER_AST_PROJECTION, -1);
    struct projection *node = container_of(self, struct projection, _astnode);

    ssize_t r = snprintf(str, size, "expression=@%u",
            node->expression->ordinal);
    if (r < 0)
    {
        return -1;
    }
    size_t n = r;
    if (node->alias != NULL)
    {
        r = snprintf(str + n, (n < size)? size-n : 0, ", alias=@%u",
            node->alias->ordinal);
        if (r < 0)
        {
            return -1;
        }
        n += r;
    }
    return n;
}
