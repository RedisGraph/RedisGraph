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


struct foreach_clause
{
    cypher_astnode_t _astnode;
    const cypher_astnode_t *identifier;
    const cypher_astnode_t *expression;
    unsigned int nclauses;
    const cypher_astnode_t *clauses[];
};


static ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size);


static const struct cypher_astnode_vt *parents[] =
    { &cypher_query_clause_astnode_vt };

const struct cypher_astnode_vt cypher_foreach_astnode_vt =
    { .parents = parents,
      .nparents = 1,
      .name = "FOREACH",
      .detailstr = detailstr,
      .free = cypher_astnode_free };


cypher_astnode_t *cypher_ast_foreach(const cypher_astnode_t *identifier,
        const cypher_astnode_t *expression, cypher_astnode_t * const *clauses,
        unsigned int nclauses, cypher_astnode_t **children,
        unsigned int nchildren, struct cypher_input_range range)
{
    REQUIRE_TYPE(identifier, CYPHER_AST_IDENTIFIER, NULL);
    REQUIRE_TYPE(expression, CYPHER_AST_EXPRESSION, NULL);
    REQUIRE(nclauses > 0, NULL);
    REQUIRE_TYPE_ALL(clauses, nclauses, CYPHER_AST_QUERY_CLAUSE, NULL);

    struct foreach_clause *node = calloc(1, sizeof(struct foreach_clause) +
            nclauses * sizeof(cypher_astnode_t *));
    if (node == NULL)
    {
        return NULL;
    }
    if (cypher_astnode_init(&(node->_astnode), CYPHER_AST_FOREACH,
            children, nchildren, range))
    {
        goto cleanup;
    }
    node->identifier = identifier;
    node->expression = expression;
    memcpy(node->clauses, clauses, nclauses * sizeof(cypher_astnode_t *));
    node->nclauses = nclauses;
    return &(node->_astnode);

    int errsv;
cleanup:
    errsv = errno;
    free(node);
    errno = errsv;
    return NULL;
}


const cypher_astnode_t *cypher_ast_foreach_get_identifier(
        const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_FOREACH, NULL);
    struct foreach_clause *node =
            container_of(astnode, struct foreach_clause, _astnode);
    return node->identifier;
}


const cypher_astnode_t *cypher_ast_foreach_get_expression(
        const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_FOREACH, NULL);
    struct foreach_clause *node =
            container_of(astnode, struct foreach_clause, _astnode);
    return node->expression;
}


unsigned int cypher_ast_foreach_nclauses(const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_FOREACH, 0);
    struct foreach_clause *node =
            container_of(astnode, struct foreach_clause, _astnode);
    return node->nclauses;
}


const cypher_astnode_t *cypher_ast_foreach_get_clause(
        const cypher_astnode_t *astnode, unsigned int index)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_FOREACH, NULL);
    struct foreach_clause *node =
            container_of(astnode, struct foreach_clause, _astnode);
    if (index >= node->nclauses)
    {
        return NULL;
    }
    return node->clauses[index];
}


ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size)
{
    REQUIRE_TYPE(self, CYPHER_AST_FOREACH, -1);
    struct foreach_clause *node =
            container_of(self, struct foreach_clause, _astnode);

    size_t n = 0;
    ssize_t r = snprintf(str, size, "[@%u IN @%u | ",
            node->identifier->ordinal, node->expression->ordinal);
    if (r < 0)
    {
        return -1;
    }
    n += r;

    for (unsigned int i = 0; i < node->nclauses; ++i)
    {
        r = snprintf(str+n, (n < size)? size-n : 0, "%s@%u",
                (i > 0)? ", " : "", node->clauses[i]->ordinal);
        if (r < 0)
        {
            return -1;
        }
        n += r;
    }

    if (++n < size)
    {
        str[n-1] = ']';
        str[n] = '\0';
    }

    return n;
}
