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
#include <assert.h>


struct statement
{
    cypher_astnode_t _astnode;
    const cypher_astnode_t *body;
    unsigned int noptions;
    const cypher_astnode_t *options[];
};


static ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size);


const struct cypher_astnode_vt cypher_statement_astnode_vt =
    { .name = "statement",
      .detailstr = detailstr,
      .free = cypher_astnode_free };


cypher_astnode_t *cypher_ast_statement(cypher_astnode_t * const *options,
        unsigned int noptions, const cypher_astnode_t *body,
        cypher_astnode_t **children, unsigned int nchildren,
        struct cypher_input_range range)
{
    REQUIRE_TYPE_ALL(options, noptions, CYPHER_AST_STATEMENT_OPTION, NULL);
    REQUIRE(cypher_astnode_instanceof(body, CYPHER_AST_QUERY) ||
            cypher_astnode_instanceof(body, CYPHER_AST_SCHEMA_COMMAND), NULL);

    struct statement *node = calloc(1, sizeof(struct statement) +
            noptions * sizeof(cypher_astnode_t *));
    if (node == NULL)
    {
        return NULL;
    }
    if (cypher_astnode_init(&(node->_astnode), CYPHER_AST_STATEMENT,
            children, nchildren, range))
    {
        goto cleanup;
    }
    memcpy(node->options, options, noptions * sizeof(cypher_astnode_t *));
    node->noptions = noptions;
    node->body = body;
    return &(node->_astnode);

    int errsv;
cleanup:
    errsv = errno;
    free(node);
    errno = errsv;
    return NULL;
}


unsigned int cypher_ast_statement_noptions(const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_STATEMENT, 0);
    struct statement *node = container_of(astnode, struct statement, _astnode);
    return node->noptions;
}


const cypher_astnode_t *cypher_ast_statement_get_option(
        const cypher_astnode_t *astnode, unsigned int index)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_STATEMENT, NULL);
    struct statement *node = container_of(astnode, struct statement, _astnode);
    if (index >= node->noptions)
    {
        return NULL;
    }
    return node->options[index];
}


const cypher_astnode_t *cypher_ast_statement_get_body(
        const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_STATEMENT, NULL);
    struct statement *node = container_of(astnode, struct statement, _astnode);
    return node->body;
}


ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size)
{
    REQUIRE_TYPE(self, CYPHER_AST_STATEMENT, -1);
    struct statement *node = container_of(self, struct statement, _astnode);

    size_t n = 0;

    ssize_t r;
    if (node->noptions > 0)
    {
        if (n < size)
        {
            strncpy(str + n, "options=", size - n);
            str[size-1] = '\0';
        }
        n += 8;

        r = snprint_sequence(str + 8, (size > 8)? size-8 : 0,
                node->options, node->noptions);
        if (r < 0)
        {
            return -1;
        }
        n += r;

        if (n < size)
        {
            strncpy(str + n, ", ", size - n);
            str[size-1] = '\0';
        }
        n += 2;
    }

    r = snprintf(str + n, (n < size)? size-n : 0, "body=@%u",
            node->body->ordinal);
    if (r < 0)
    {
        return -1;
    }
    n += r;

    return n;
}
