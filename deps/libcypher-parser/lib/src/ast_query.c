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


struct query
{
    cypher_astnode_t _astnode;
    unsigned int noptions;
    const cypher_astnode_t **options;
    unsigned int nclauses;
    const cypher_astnode_t *clauses[];
};


static ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size);
static void query_free(cypher_astnode_t *self);


const struct cypher_astnode_vt cypher_query_astnode_vt =
    { .name = "query",
      .detailstr = detailstr,
      .free = query_free };


cypher_astnode_t *cypher_ast_query(cypher_astnode_t * const *options,
        unsigned int noptions, cypher_astnode_t * const *clauses,
        unsigned int nclauses, cypher_astnode_t **children,
        unsigned int nchildren, struct cypher_input_range range)
{
    REQUIRE_TYPE_ALL(options, noptions, CYPHER_AST_QUERY_OPTION, NULL);
    REQUIRE(nclauses > 0, NULL);
    REQUIRE_TYPE_ALL(clauses, nclauses, CYPHER_AST_QUERY_CLAUSE, NULL);

    struct query *node = calloc(1, sizeof(struct query) +
            nclauses * sizeof(cypher_astnode_t *));
    if (node == NULL)
    {
        return NULL;
    }
    if (cypher_astnode_init(&(node->_astnode), CYPHER_AST_QUERY,
            children, nchildren, range))
    {
        goto cleanup;
    }
    if (noptions > 0)
    {
        node->options = mdup(options, noptions * sizeof(cypher_astnode_t *));
        if (node->options == NULL)
        {
            goto cleanup;
        }
        node->noptions = noptions;
    }
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


void query_free(cypher_astnode_t *self)
{
    struct query *node = container_of(self, struct query, _astnode);
    free(node->options);
    cypher_astnode_free(self);
}


unsigned int cypher_ast_query_noptions(const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_QUERY, 0);
    struct query *node = container_of(astnode, struct query, _astnode);
    return node->noptions;
}


const cypher_astnode_t *cypher_ast_query_get_option(
        const cypher_astnode_t *astnode, unsigned int index)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_QUERY, NULL);
    struct query *node = container_of(astnode, struct query, _astnode);
    if (index >= node->noptions)
    {
        return NULL;
    }
    return node->options[index];
}


unsigned int cypher_ast_query_nclauses(const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_QUERY, 0);
    struct query *node = container_of(astnode, struct query, _astnode);
    return node->nclauses;
}


const cypher_astnode_t *cypher_ast_query_get_clause(
        const cypher_astnode_t *astnode, unsigned int index)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_QUERY, NULL);
    struct query *node = container_of(astnode, struct query, _astnode);
    if (index >= node->nclauses)
    {
        return NULL;
    }
    return node->clauses[index];
}


ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size)
{
    REQUIRE_TYPE(self, CYPHER_AST_QUERY, -1);
    struct query *node = container_of(self, struct query, _astnode);

    strncpy(str, "clauses=", size);
    if (size > 0)
    {
        str[size-1] = '\0';
    }
    size_t n = 8;

    ssize_t r = snprint_sequence(str + 8, (size > 8)? size-8 : 0,
            node->clauses, node->nclauses);
    if (r < 0)
    {
        return -1;
    }
    return n + r;
}
