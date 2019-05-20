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


struct remove_property
{
    cypher_astnode_t _astnode;
    const cypher_astnode_t *property;
};


static ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size);


static const struct cypher_astnode_vt *parents[] =
    { &cypher_remove_item_astnode_vt };

const struct cypher_astnode_vt cypher_remove_property_astnode_vt =
    { .parents = parents,
      .nparents = 1,
      .name = "remove property",
      .detailstr = detailstr,
      .free = cypher_astnode_free };


cypher_astnode_t *cypher_ast_remove_property(const cypher_astnode_t *property,
        cypher_astnode_t **children, unsigned int nchildren,
        struct cypher_input_range range)
{
    REQUIRE_TYPE(property, CYPHER_AST_PROPERTY_OPERATOR, NULL);

    struct remove_property *node = calloc(1, sizeof(struct remove_property));
    if (node == NULL)
    {
        return NULL;
    }
    if (cypher_astnode_init(&(node->_astnode), CYPHER_AST_REMOVE_PROPERTY,
            children, nchildren, range))
    {
        free(node);
        return NULL;
    }
    node->property = property;
    return &(node->_astnode);
}


const cypher_astnode_t *cypher_ast_remove_property_get_property(
        const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_REMOVE_PROPERTY, NULL);
    struct remove_property *node =
            container_of(astnode, struct remove_property, _astnode);
    return node->property;
}


ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size)
{
    REQUIRE_TYPE(self, CYPHER_AST_REMOVE_PROPERTY, -1);
    struct remove_property *node =
            container_of(self, struct remove_property, _astnode);
    return snprintf(str, size, "prop=@%u", node->property->ordinal);
}
