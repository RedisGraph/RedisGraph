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


struct delete_clause
{
    cypher_astnode_t _astnode;
    bool detach;
    unsigned int nexpressions;
    const cypher_astnode_t *expressions[];
};


static ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size);


static const struct cypher_astnode_vt *parents[] =
    { &cypher_query_clause_astnode_vt };

const struct cypher_astnode_vt cypher_delete_astnode_vt =
    { .parents = parents,
      .nparents = 1,
      .name = "DELETE",
      .detailstr = detailstr,
      .free = cypher_astnode_free };


cypher_astnode_t *cypher_ast_delete(bool detach,
        cypher_astnode_t * const *expressions, unsigned int nexpressions,
        cypher_astnode_t **children, unsigned int nchildren,
        struct cypher_input_range range)
{
    REQUIRE(nexpressions > 0, NULL);
    REQUIRE_TYPE_ALL(expressions, nexpressions, CYPHER_AST_EXPRESSION, NULL);

    struct delete_clause *node = calloc(1, sizeof(struct delete_clause) +
            nexpressions * sizeof(cypher_astnode_t *));
    if (node == NULL)
    {
        return NULL;
    }
    if (cypher_astnode_init(&(node->_astnode), CYPHER_AST_DELETE,
            children, nchildren, range))
    {
        goto cleanup;
    }
    node->detach = detach;
    memcpy(node->expressions, expressions,
            nexpressions * sizeof(cypher_astnode_t *));
    node->nexpressions = nexpressions;
    return &(node->_astnode);

    int errsv;
cleanup:
    errsv = errno;
    free(node);
    errno = errsv;
    return NULL;
}


bool cypher_ast_delete_has_detach(const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_DELETE, false);
    struct delete_clause *node =
            container_of(astnode, struct delete_clause, _astnode);
    return node->detach;
}


unsigned int cypher_ast_delete_nexpressions(const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_DELETE, 0);
    struct delete_clause *node =
            container_of(astnode, struct delete_clause, _astnode);
    return node->nexpressions;
}


const cypher_astnode_t *cypher_ast_delete_get_expression(
        const cypher_astnode_t *astnode, unsigned int index)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_DELETE, NULL);
    struct delete_clause *node =
            container_of(astnode, struct delete_clause, _astnode);
    if (index >= node->nexpressions)
    {
        return NULL;
    }
    return node->expressions[index];
}


ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size)
{
    REQUIRE_TYPE(self, CYPHER_AST_DELETE, -1);
    struct delete_clause *node =
            container_of(self, struct delete_clause, _astnode);

    ssize_t r = snprintf(str, size, "%sexpressions=",
            node->detach? "DETACH, ":"");
    if (r < 0)
    {
        return -1;
    }
    size_t n = r;
    r = snprint_sequence(str + n, (n < size)? size-n : 0,
            node->expressions, node->nexpressions);
    if (r < 0)
    {
        return -1;
    }
    return n + r;
}
