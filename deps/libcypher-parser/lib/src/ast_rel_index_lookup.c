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


struct rel_index_lookup
{
    cypher_astnode_t _astnode;
    const cypher_astnode_t *identifier;
    const cypher_astnode_t *index_name;
    const cypher_astnode_t *prop_name;
    const cypher_astnode_t *lookup;
};


static ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size);


static const struct cypher_astnode_vt *parents[] =
    { &cypher_start_point_astnode_vt };

const struct cypher_astnode_vt cypher_rel_index_lookup_astnode_vt =
    { .parents = parents,
      .nparents = 1,
      .name = "rel index lookup",
      .detailstr = detailstr,
      .free = cypher_astnode_free };


cypher_astnode_t *cypher_ast_rel_index_lookup(
        const cypher_astnode_t *identifier, const cypher_astnode_t *index_name,
        const cypher_astnode_t *prop_name, const cypher_astnode_t *lookup,
        cypher_astnode_t **children, unsigned int nchildren,
        struct cypher_input_range range)
{
    REQUIRE_TYPE(identifier, CYPHER_AST_IDENTIFIER, NULL);
    REQUIRE_TYPE(index_name, CYPHER_AST_INDEX_NAME, NULL);
    REQUIRE_TYPE(prop_name, CYPHER_AST_PROP_NAME, NULL);
    REQUIRE(cypher_astnode_instanceof(lookup, CYPHER_AST_STRING) ||
            cypher_astnode_instanceof(lookup, CYPHER_AST_PARAMETER), NULL);

    struct rel_index_lookup *node = calloc(1, sizeof(struct rel_index_lookup));
    if (node == NULL)
    {
        return NULL;
    }
    if (cypher_astnode_init(&(node->_astnode), CYPHER_AST_REL_INDEX_LOOKUP,
            children, nchildren, range))
    {
        free(node);
        return NULL;
    }
    node->identifier = identifier;
    node->index_name = index_name;
    node->prop_name = prop_name;
    node->lookup = lookup;
    return &(node->_astnode);
}


const cypher_astnode_t *cypher_ast_rel_index_lookup_get_identifier(
        const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_REL_INDEX_LOOKUP, NULL);
    struct rel_index_lookup *node =
            container_of(astnode, struct rel_index_lookup, _astnode);
    return node->identifier;
}


const cypher_astnode_t *cypher_ast_rel_index_lookup_get_index_name(
        const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_REL_INDEX_LOOKUP, NULL);
    struct rel_index_lookup *node =
            container_of(astnode, struct rel_index_lookup, _astnode);
    return node->index_name;
}


const cypher_astnode_t *cypher_ast_rel_index_lookup_get_prop_name(
        const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_REL_INDEX_LOOKUP, NULL);
    struct rel_index_lookup *node =
            container_of(astnode, struct rel_index_lookup, _astnode);
    return node->prop_name;
}


const cypher_astnode_t *cypher_ast_rel_index_lookup_get_lookup(
        const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_REL_INDEX_LOOKUP, NULL);
    struct rel_index_lookup *node =
            container_of(astnode, struct rel_index_lookup, _astnode);
    return node->lookup;
}


ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size)
{
    REQUIRE_TYPE(self, CYPHER_AST_REL_INDEX_LOOKUP, -1);
    struct rel_index_lookup *node =
            container_of(self, struct rel_index_lookup, _astnode);
    return snprintf(str, size, "@%u = rel:@%u(@%u = @%u)",
                node->identifier->ordinal, node->index_name->ordinal,
                node->prop_name->ordinal, node->lookup->ordinal);
}
