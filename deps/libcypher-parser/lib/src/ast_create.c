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


struct create
{
    cypher_astnode_t _astnode;
    bool unique;
    const cypher_astnode_t *pattern;
};


static ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size);


static const struct cypher_astnode_vt *parents[] =
    { &cypher_query_clause_astnode_vt };

const struct cypher_astnode_vt cypher_create_astnode_vt =
    { .parents = parents,
      .nparents = 1,
      .name = "CREATE",
      .detailstr = detailstr,
      .free = cypher_astnode_free };


cypher_astnode_t *cypher_ast_create(bool unique,
        const cypher_astnode_t *pattern, cypher_astnode_t **children,
        unsigned int nchildren, struct cypher_input_range range)
{
    REQUIRE_TYPE(pattern, CYPHER_AST_PATTERN, NULL);

    struct create *node = calloc(1, sizeof(struct create));
    if (node == NULL)
    {
        return NULL;
    }
    if (cypher_astnode_init(&(node->_astnode), CYPHER_AST_CREATE,
            children, nchildren, range))
    {
        free(node);
        return NULL;
    }
    node->unique = unique;
    node->pattern = pattern;
    return &(node->_astnode);
}


bool cypher_ast_create_is_unique(const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_CREATE, false);
    struct create *node = container_of(astnode, struct create, _astnode);
    return node->unique;
}


const cypher_astnode_t *cypher_ast_create_get_pattern(
        const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_CREATE, NULL);
    struct create *node = container_of(astnode, struct create, _astnode);
    return node->pattern;
}


ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size)
{
    REQUIRE_TYPE(self, CYPHER_AST_CREATE, -1);
    struct create *node = container_of(self, struct create, _astnode);
    return snprintf(str, size, "%spattern=@%d", node->unique? "UNIQUE, " : "",
            node->pattern->ordinal);
}
