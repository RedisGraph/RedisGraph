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
#include "../../lib/src/cypher-parser.h"
#include "memstream.h"
#include <check.h>
#include <errno.h>
#include <unistd.h>


static cypher_parse_result_t *result;
static char *memstream_buffer;
static size_t memstream_size;
static FILE *memstream;


static void setup(void)
{
    result = NULL;
    memstream = open_memstream(&memstream_buffer, &memstream_size);
    fputc('\n', memstream);
}


static void teardown(void)
{
    cypher_parse_result_free(result);
    fclose(memstream);
    free(memstream_buffer);
}


START_TEST (parse_create_node_prop_index)
{
    struct cypher_input_position last = cypher_input_position_zero;
    result = cypher_parse("CREATE INDEX ON :Foo(bar);", &last, NULL, 0);
    ck_assert_ptr_ne(result, NULL);
    ck_assert_int_eq(last.offset, 26);

    ck_assert(cypher_parse_result_fprint_ast(result, memstream, 0, NULL, 0) == 0);
    fflush(memstream);
    const char *expected = "\n"
"@0   0..26  statement       body=@1\n"
"@1   0..25  > CREATE INDEX  ON=:@2(@3)\n"
"@2  16..20  > > label       :`Foo`\n"
"@3  21..24  > > prop name   `bar`\n";
    ck_assert_str_eq(memstream_buffer, expected);

    ck_assert_int_eq(cypher_parse_result_ndirectives(result), 1);
    const cypher_astnode_t *ast = cypher_parse_result_get_directive(result, 0);
    ck_assert_int_eq(cypher_astnode_type(ast), CYPHER_AST_STATEMENT);
    ck_assert_int_eq(cypher_astnode_range(ast).start.offset, 0);
    ck_assert_int_eq(cypher_astnode_range(ast).end.offset, 26);

    ck_assert_int_eq(cypher_ast_statement_noptions(ast), 0);
    ck_assert_ptr_eq(cypher_ast_statement_get_option(ast, 0), NULL);

    const cypher_astnode_t *body = cypher_ast_statement_get_body(ast);
    ck_assert_int_eq(cypher_astnode_type(body), CYPHER_AST_CREATE_NODE_PROP_INDEX);
    ck_assert_int_eq(cypher_astnode_range(body).start.offset, 0);
    ck_assert_int_eq(cypher_astnode_range(body).end.offset, 25);

    const cypher_astnode_t *label =
            cypher_ast_create_node_prop_index_get_label(body);
    ck_assert_int_eq(cypher_astnode_type(label), CYPHER_AST_LABEL);
    ck_assert_str_eq(cypher_ast_label_get_name(label), "Foo");

    const cypher_astnode_t *prop_name =
            cypher_ast_create_node_prop_index_get_prop_name(body);
    ck_assert_int_eq(cypher_astnode_type(prop_name), CYPHER_AST_PROP_NAME);
    ck_assert_str_eq(cypher_ast_prop_name_get_value(prop_name), "bar");
}
END_TEST


START_TEST (parse_drop_node_prop_index)
{
    struct cypher_input_position last = cypher_input_position_zero;
    result = cypher_parse("/* drop! */DROP INDEX ON /* a label */ :Foo(bar);", &last, NULL, 0);
    ck_assert_ptr_ne(result, NULL);
    ck_assert_int_eq(last.offset, 49);

    ck_assert(cypher_parse_result_fprint_ast(result, memstream, 0, NULL, 0) == 0);
    fflush(memstream);
    const char *expected = "\n"
"@0   2..9   block_comment      /* drop! */\n"
"@1  11..49  statement          body=@2\n"
"@2  11..48  > DROP INDEX       ON=:@4(@5)\n"
"@3  27..36  > > block_comment  /* a label */\n"
"@4  39..43  > > label          :`Foo`\n"
"@5  44..47  > > prop name      `bar`\n";
    ck_assert_str_eq(memstream_buffer, expected);

    ck_assert_int_eq(cypher_parse_result_ndirectives(result), 1);
    const cypher_astnode_t *ast = cypher_parse_result_get_directive(result, 0);
    ck_assert_int_eq(cypher_astnode_type(ast), CYPHER_AST_STATEMENT);
    ck_assert_int_eq(cypher_astnode_range(ast).start.offset, 11);
    ck_assert_int_eq(cypher_astnode_range(ast).end.offset, 49);

    ck_assert_int_eq(cypher_ast_statement_noptions(ast), 0);
    ck_assert_ptr_eq(cypher_ast_statement_get_option(ast, 0), NULL);

    const cypher_astnode_t *body = cypher_ast_statement_get_body(ast);
    ck_assert_int_eq(cypher_astnode_type(body), CYPHER_AST_DROP_NODE_PROP_INDEX);
    ck_assert_int_eq(cypher_astnode_range(body).start.offset, 11);
    ck_assert_int_eq(cypher_astnode_range(body).end.offset, 48);

    const cypher_astnode_t *label =
            cypher_ast_drop_node_prop_index_get_label(body);
    ck_assert_int_eq(cypher_astnode_type(label), CYPHER_AST_LABEL);
    ck_assert_str_eq(cypher_ast_label_get_name(label), "Foo");

    const cypher_astnode_t *prop_name =
            cypher_ast_drop_node_prop_index_get_prop_name(body);
    ck_assert_int_eq(cypher_astnode_type(prop_name), CYPHER_AST_PROP_NAME);
    ck_assert_str_eq(cypher_ast_prop_name_get_value(prop_name), "bar");
}
END_TEST


TCase* indexes_tcase(void)
{
    TCase *tc = tcase_create("indexes");
    tcase_add_checked_fixture(tc, setup, teardown);
    tcase_add_test(tc, parse_create_node_prop_index);
    tcase_add_test(tc, parse_drop_node_prop_index);
    return tc;
}
