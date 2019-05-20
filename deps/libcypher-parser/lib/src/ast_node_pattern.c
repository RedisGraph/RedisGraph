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


struct node_pattern
{
    cypher_astnode_t _astnode;
    const cypher_astnode_t *identifier;
    const cypher_astnode_t *properties;
    size_t nlabels;
    const cypher_astnode_t *labels[];
};


static ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size);


const struct cypher_astnode_vt cypher_node_pattern_astnode_vt =
    { .name = "node pattern",
      .detailstr = detailstr,
      .free = cypher_astnode_free };


cypher_astnode_t *cypher_ast_node_pattern(const cypher_astnode_t *identifier,
        cypher_astnode_t * const *labels, unsigned int nlabels,
        const cypher_astnode_t *properties, cypher_astnode_t **children,
        unsigned int nchildren, struct cypher_input_range range)
{
    REQUIRE_TYPE_OPTIONAL(identifier, CYPHER_AST_IDENTIFIER, NULL);
    REQUIRE_TYPE_ALL(labels, nlabels, CYPHER_AST_LABEL, NULL);
    REQUIRE(properties == NULL ||
            cypher_astnode_instanceof(properties, CYPHER_AST_MAP) ||
            cypher_astnode_instanceof(properties, CYPHER_AST_PARAMETER), NULL);

    struct node_pattern *node = calloc(1, sizeof(struct node_pattern) +
            nlabels * sizeof(cypher_astnode_t *));
    if (node == NULL)
    {
        return NULL;
    }
    if (cypher_astnode_init(&(node->_astnode), CYPHER_AST_NODE_PATTERN,
                children, nchildren, range))
    {
        goto cleanup;
    }
    node->identifier = identifier;
    memcpy(node->labels, labels, nlabels * sizeof(cypher_astnode_t *));
    node->nlabels = nlabels;
    node->properties = properties;
    return &(node->_astnode);

    int errsv;
cleanup:
    errsv = errno;
    free(node);
    errno = errsv;
    return NULL;
}


const cypher_astnode_t *cypher_ast_node_pattern_get_identifier(
        const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_NODE_PATTERN, NULL);
    struct node_pattern *node =
            container_of(astnode, struct node_pattern, _astnode);
    return node->identifier;
}


unsigned int cypher_ast_node_pattern_nlabels(const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_NODE_PATTERN, 0);
    struct node_pattern *node =
            container_of(astnode, struct node_pattern, _astnode);
    return node->nlabels;
}


const cypher_astnode_t *cypher_ast_node_pattern_get_label(
        const cypher_astnode_t *astnode, unsigned int index)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_NODE_PATTERN, NULL);
    struct node_pattern *node =
            container_of(astnode, struct node_pattern, _astnode);
    if (index >= node->nlabels)
    {
        return NULL;
    }
    return node->labels[index];
}


const cypher_astnode_t *cypher_ast_node_pattern_get_properties(
        const cypher_astnode_t *astnode)
{
    REQUIRE_TYPE(astnode, CYPHER_AST_NODE_PATTERN, NULL);
    struct node_pattern *node =
            container_of(astnode, struct node_pattern, _astnode);
    return node->properties;
}


ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size)
{
    REQUIRE_TYPE(self, CYPHER_AST_NODE_PATTERN, -1);
    struct node_pattern *node =
            container_of(self, struct node_pattern, _astnode);

    size_t n = 0;
    if (n < size)
    {
        str[n] = '(';
    }
    n++;

    ssize_t r;
    if (node->identifier != NULL)
    {
        r = snprintf(str+n, (n < size)? size-n : 0, "@%u",
                node->identifier->ordinal);
        if (r < 0)
        {
            return -1;
        }
        n += r;
    }

    for (unsigned int i = 0; i < node->nlabels; ++i)
    {
        r = snprintf(str+n, (n < size)? size-n : 0, ":@%u",
                node->labels[i]->ordinal);
        if (r < 0)
        {
            return -1;
        }
        n += r;
    }

    if (node->properties != NULL)
    {
        r = snprintf(str+n, (n < size)? size-n : 0, " {@%u}",
                node->properties->ordinal);
        if (r < 0)
        {
            return -1;
        }
        n += r;
    }

    if (n < size)
    {
        str[n] = ')';
    }
    n++;

    return n;
}
