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


START_TEST (parse_statement_with_no_options)
{
    struct cypher_input_position last = cypher_input_position_zero;
    result = cypher_parse("RETURN 1;", &last, NULL, 0);
    ck_assert_ptr_ne(result, NULL);
    ck_assert_int_eq(last.offset, 9);

    ck_assert(cypher_parse_result_fprint_ast(result, memstream, 0, NULL, 0) == 0);
    fflush(memstream);
    const char *expected = "\n"
"@0  0..9  statement           body=@1\n"
"@1  0..9  > query             clauses=[@2]\n"
"@2  0..8  > > RETURN          projections=[@3]\n"
"@3  7..8  > > > projection    expression=@4, alias=@5\n"
"@4  7..8  > > > > integer     1\n"
"@5  7..8  > > > > identifier  `1`\n";
    ck_assert_str_eq(memstream_buffer, expected);

    ck_assert_int_eq(cypher_parse_result_ndirectives(result), 1);
    const cypher_astnode_t *ast = cypher_parse_result_get_directive(result, 0);
    ck_assert_int_eq(cypher_astnode_type(ast), CYPHER_AST_STATEMENT);
    ck_assert_int_eq(cypher_astnode_range(ast).start.offset, 0);
    ck_assert_int_eq(cypher_astnode_range(ast).end.offset, 9);

    const cypher_astnode_t *body = cypher_ast_statement_get_body(ast);
    ck_assert_int_eq(cypher_astnode_type(body), CYPHER_AST_QUERY);
    ck_assert_int_eq(cypher_astnode_range(body).start.offset, 0);
    ck_assert_int_eq(cypher_astnode_range(body).end.offset, 9);

    ck_assert_int_eq(cypher_ast_statement_noptions(ast), 0);
    ck_assert_ptr_eq(cypher_ast_statement_get_option(ast, 0), NULL);
}
END_TEST


START_TEST (parse_statement_with_cypher_option)
{
    result = cypher_parse("CYPHER RETURN 1;", NULL, NULL, 0);

    ck_assert(cypher_parse_result_fprint_ast(result, memstream, 0, NULL, 0) == 0);
    fflush(memstream);
    const char *expected = "\n"
"@0   0..16  statement           options=[@1], body=@2\n"
"@1   0..7   > CYPHER\n"
"@2   7..16  > query             clauses=[@3]\n"
"@3   7..15  > > RETURN          projections=[@4]\n"
"@4  14..15  > > > projection    expression=@5, alias=@6\n"
"@5  14..15  > > > > integer     1\n"
"@6  14..15  > > > > identifier  `1`\n";
    ck_assert_str_eq(memstream_buffer, expected);

    ck_assert_int_eq(cypher_parse_result_ndirectives(result), 1);
    const cypher_astnode_t *ast = cypher_parse_result_get_directive(result, 0);
    ck_assert_int_eq(cypher_astnode_type(ast), CYPHER_AST_STATEMENT);

    ck_assert_int_eq(cypher_ast_statement_noptions(ast), 1);
    const cypher_astnode_t *option = cypher_ast_statement_get_option(ast, 0);
    ck_assert(cypher_astnode_instanceof(option, CYPHER_AST_STATEMENT_OPTION));
    ck_assert_int_eq(cypher_astnode_type(option), CYPHER_AST_CYPHER_OPTION);

    ck_assert_ptr_eq(cypher_ast_cypher_option_get_version(option), NULL);
    ck_assert_int_eq(cypher_ast_cypher_option_nparams(option), 0);
    ck_assert_ptr_eq(cypher_ast_cypher_option_get_param(option, 0), NULL);
}
END_TEST


START_TEST (parse_statement_with_cypher_option_containing_version)
{
    result = cypher_parse("CYPHER 3.0 RETURN 1;", NULL, NULL, 0);

    ck_assert(cypher_parse_result_fprint_ast(result, memstream, 0, NULL, 0) == 0);
    fflush(memstream);
    const char *expected = "\n"
"@0   0..20  statement           options=[@1], body=@3\n"
"@1   0..10  > CYPHER            version=@2\n"
"@2   7..10  > > string          \"3.0\"\n"
"@3  11..20  > query             clauses=[@4]\n"
"@4  11..19  > > RETURN          projections=[@5]\n"
"@5  18..19  > > > projection    expression=@6, alias=@7\n"
"@6  18..19  > > > > integer     1\n"
"@7  18..19  > > > > identifier  `1`\n";
    ck_assert_str_eq(memstream_buffer, expected);

    ck_assert_int_eq(cypher_parse_result_ndirectives(result), 1);
    const cypher_astnode_t *ast = cypher_parse_result_get_directive(result, 0);
    ck_assert_int_eq(cypher_astnode_type(ast), CYPHER_AST_STATEMENT);

    ck_assert_int_eq(cypher_ast_statement_noptions(ast), 1);
    const cypher_astnode_t *option = cypher_ast_statement_get_option(ast, 0);
    ck_assert(cypher_astnode_instanceof(option, CYPHER_AST_STATEMENT_OPTION));
    ck_assert_int_eq(cypher_astnode_type(option), CYPHER_AST_CYPHER_OPTION);

    const cypher_astnode_t *version =
            cypher_ast_cypher_option_get_version(option);
    ck_assert_int_eq(cypher_astnode_type(version), CYPHER_AST_STRING);
    ck_assert_str_eq(cypher_ast_string_get_value(version), "3.0");

    ck_assert_int_eq(cypher_ast_cypher_option_nparams(option), 0);
    ck_assert_ptr_eq(cypher_ast_cypher_option_get_param(option, 0), NULL);
}
END_TEST


START_TEST (parse_statement_with_cypher_option_containing_params)
{
    result = cypher_parse("CYPHER runtime=fast RETURN 1;",
            NULL, NULL, 0);

    ck_assert(cypher_parse_result_fprint_ast(result, memstream, 0, NULL, 0) == 0);
    fflush(memstream);
    const char *expected = "\n"
"@0   0..29  statement             options=[@1], body=@5\n"
"@1   0..19  > CYPHER              params=[@2]\n"
"@2   7..19  > > cypher parameter  @3 = @4\n"
"@3   7..14  > > > string          \"runtime\"\n"
"@4  15..19  > > > string          \"fast\"\n"
"@5  20..29  > query               clauses=[@6]\n"
"@6  20..28  > > RETURN            projections=[@7]\n"
"@7  27..28  > > > projection      expression=@8, alias=@9\n"
"@8  27..28  > > > > integer       1\n"
"@9  27..28  > > > > identifier    `1`\n";
    ck_assert_str_eq(memstream_buffer, expected);

    ck_assert_int_eq(cypher_parse_result_ndirectives(result), 1);
    const cypher_astnode_t *ast = cypher_parse_result_get_directive(result, 0);
    ck_assert_int_eq(cypher_astnode_type(ast), CYPHER_AST_STATEMENT);

    ck_assert_int_eq(cypher_ast_statement_noptions(ast), 1);
    const cypher_astnode_t *option = cypher_ast_statement_get_option(ast, 0);
    ck_assert(cypher_astnode_instanceof(option, CYPHER_AST_STATEMENT_OPTION));
    ck_assert_int_eq(cypher_astnode_type(option), CYPHER_AST_CYPHER_OPTION);

    ck_assert_ptr_eq(cypher_ast_cypher_option_get_version(option), NULL);

    ck_assert_int_eq(cypher_ast_cypher_option_nparams(option), 1);
    const cypher_astnode_t *param =
            cypher_ast_cypher_option_get_param(option, 0);
    ck_assert_int_eq(cypher_astnode_type(param),
            CYPHER_AST_CYPHER_OPTION_PARAM);

    const cypher_astnode_t *name =
            cypher_ast_cypher_option_param_get_name(param);
    ck_assert_int_eq(cypher_astnode_type(name), CYPHER_AST_STRING);
    const cypher_astnode_t *value =
            cypher_ast_cypher_option_param_get_value(param);
    ck_assert_int_eq(cypher_astnode_type(value), CYPHER_AST_STRING);

    ck_assert_str_eq(cypher_ast_string_get_value(name), "runtime");
    ck_assert_str_eq(cypher_ast_string_get_value(value), "fast");
}
END_TEST


START_TEST (parse_statement_with_cypher_option_containing_version_and_params)
{
    result = cypher_parse("CYPHER 2.3 runtime=fast planner=slow RETURN 1;",
            NULL, NULL, 0);

    ck_assert(cypher_parse_result_fprint_ast(result, memstream, 0, NULL, 0) == 0);
    fflush(memstream);
    const char *expected = "\n"
" @0   0..46  statement             options=[@1], body=@9\n"
" @1   0..36  > CYPHER              version=@2, params=[@3, @6]\n"
" @2   7..10  > > string            \"2.3\"\n"
" @3  11..23  > > cypher parameter  @4 = @5\n"
" @4  11..18  > > > string          \"runtime\"\n"
" @5  19..23  > > > string          \"fast\"\n"
" @6  24..36  > > cypher parameter  @7 = @8\n"
" @7  24..31  > > > string          \"planner\"\n"
" @8  32..36  > > > string          \"slow\"\n"
" @9  37..46  > query               clauses=[@10]\n"
"@10  37..45  > > RETURN            projections=[@11]\n"
"@11  44..45  > > > projection      expression=@12, alias=@13\n"
"@12  44..45  > > > > integer       1\n"
"@13  44..45  > > > > identifier    `1`\n";
    ck_assert_str_eq(memstream_buffer, expected);

    ck_assert_int_eq(cypher_parse_result_ndirectives(result), 1);
    const cypher_astnode_t *ast = cypher_parse_result_get_directive(result, 0);
    ck_assert_int_eq(cypher_astnode_type(ast), CYPHER_AST_STATEMENT);

    ck_assert_int_eq(cypher_ast_statement_noptions(ast), 1);
    const cypher_astnode_t *option = cypher_ast_statement_get_option(ast, 0);
    ck_assert(cypher_astnode_instanceof(option, CYPHER_AST_STATEMENT_OPTION));
    ck_assert_int_eq(cypher_astnode_type(option), CYPHER_AST_CYPHER_OPTION);

    const cypher_astnode_t *version =
            cypher_ast_cypher_option_get_version(option);
    ck_assert_int_eq(cypher_astnode_type(version), CYPHER_AST_STRING);
    ck_assert_str_eq(cypher_ast_string_get_value(version), "2.3");

    ck_assert_int_eq(cypher_ast_cypher_option_nparams(option), 2);

    const cypher_astnode_t *param =
            cypher_ast_cypher_option_get_param(option, 0);
    ck_assert_int_eq(cypher_astnode_type(param),
            CYPHER_AST_CYPHER_OPTION_PARAM);

    const cypher_astnode_t *name =
            cypher_ast_cypher_option_param_get_name(param);
    ck_assert_int_eq(cypher_astnode_type(name), CYPHER_AST_STRING);
    const cypher_astnode_t *value =
            cypher_ast_cypher_option_param_get_value(param);
    ck_assert_int_eq(cypher_astnode_type(value), CYPHER_AST_STRING);

    ck_assert_str_eq(cypher_ast_string_get_value(name), "runtime");
    ck_assert_str_eq(cypher_ast_string_get_value(value), "fast");

    param = cypher_ast_cypher_option_get_param(option, 1);
    ck_assert_int_eq(cypher_astnode_type(param),
            CYPHER_AST_CYPHER_OPTION_PARAM);

    name = cypher_ast_cypher_option_param_get_name(param);
    ck_assert_int_eq(cypher_astnode_type(name), CYPHER_AST_STRING);
    value = cypher_ast_cypher_option_param_get_value(param);
    ck_assert_int_eq(cypher_astnode_type(value), CYPHER_AST_STRING);

    ck_assert_str_eq(cypher_ast_string_get_value(name), "planner");
    ck_assert_str_eq(cypher_ast_string_get_value(value), "slow");
}
END_TEST


START_TEST (parse_statement_with_explain_option)
{
    result = cypher_parse("EXPLAIN RETURN 1;", NULL, NULL, 0);

    ck_assert(cypher_parse_result_fprint_ast(result, memstream, 0, NULL, 0) == 0);
    fflush(memstream);
    const char *expected = "\n"
"@0   0..17  statement           options=[@1], body=@2\n"
"@1   0..7   > EXPLAIN\n"
"@2   8..17  > query             clauses=[@3]\n"
"@3   8..16  > > RETURN          projections=[@4]\n"
"@4  15..16  > > > projection    expression=@5, alias=@6\n"
"@5  15..16  > > > > integer     1\n"
"@6  15..16  > > > > identifier  `1`\n";
    ck_assert_str_eq(memstream_buffer, expected);

    ck_assert_int_eq(cypher_parse_result_ndirectives(result), 1);
    const cypher_astnode_t *ast = cypher_parse_result_get_directive(result, 0);
    ck_assert_int_eq(cypher_astnode_type(ast), CYPHER_AST_STATEMENT);

    ck_assert_int_eq(cypher_ast_statement_noptions(ast), 1);
    const cypher_astnode_t *option = cypher_ast_statement_get_option(ast, 0);
    ck_assert(cypher_astnode_instanceof(option, CYPHER_AST_STATEMENT_OPTION));
    ck_assert_int_eq(cypher_astnode_type(option), CYPHER_AST_EXPLAIN_OPTION);
}
END_TEST


START_TEST (parse_statement_with_profile_option)
{
    result = cypher_parse("PROFILE RETURN 1;", NULL, NULL, 0);

    ck_assert(cypher_parse_result_fprint_ast(result, memstream, 0, NULL, 0) == 0);
    fflush(memstream);
    const char *expected = "\n"
"@0   0..17  statement           options=[@1], body=@2\n"
"@1   0..7   > PROFILE\n"
"@2   8..17  > query             clauses=[@3]\n"
"@3   8..16  > > RETURN          projections=[@4]\n"
"@4  15..16  > > > projection    expression=@5, alias=@6\n"
"@5  15..16  > > > > integer     1\n"
"@6  15..16  > > > > identifier  `1`\n";
    ck_assert_str_eq(memstream_buffer, expected);

    ck_assert_int_eq(cypher_parse_result_ndirectives(result), 1);
    const cypher_astnode_t *ast = cypher_parse_result_get_directive(result, 0);
    ck_assert_int_eq(cypher_astnode_type(ast), CYPHER_AST_STATEMENT);

    ck_assert_int_eq(cypher_ast_statement_noptions(ast), 1);
    const cypher_astnode_t *option = cypher_ast_statement_get_option(ast, 0);
    ck_assert(cypher_astnode_instanceof(option, CYPHER_AST_STATEMENT_OPTION));
    ck_assert_int_eq(cypher_astnode_type(option), CYPHER_AST_PROFILE_OPTION);
}
END_TEST


START_TEST (parse_statement_with_multiple_options)
{
    result = cypher_parse("CYPHER 3.0 PROFILE RETURN 1;", NULL, NULL, 0);

    ck_assert(cypher_parse_result_fprint_ast(result, memstream, 0, NULL, 0) == 0);
    fflush(memstream);
    const char *expected = "\n"
"@0   0..28  statement           options=[@1, @3], body=@4\n"
"@1   0..10  > CYPHER            version=@2\n"
"@2   7..10  > > string          \"3.0\"\n"
"@3  11..18  > PROFILE\n"
"@4  19..28  > query             clauses=[@5]\n"
"@5  19..27  > > RETURN          projections=[@6]\n"
"@6  26..27  > > > projection    expression=@7, alias=@8\n"
"@7  26..27  > > > > integer     1\n"
"@8  26..27  > > > > identifier  `1`\n";
    ck_assert_str_eq(memstream_buffer, expected);

    ck_assert_int_eq(cypher_parse_result_ndirectives(result), 1);
    const cypher_astnode_t *ast = cypher_parse_result_get_directive(result, 0);
    ck_assert_int_eq(cypher_astnode_type(ast), CYPHER_AST_STATEMENT);

    ck_assert_int_eq(cypher_ast_statement_noptions(ast), 2);
    const cypher_astnode_t *option = cypher_ast_statement_get_option(ast, 0);
    ck_assert(cypher_astnode_instanceof(option, CYPHER_AST_STATEMENT_OPTION));
    ck_assert_int_eq(cypher_astnode_type(option), CYPHER_AST_CYPHER_OPTION);

    option = cypher_ast_statement_get_option(ast, 1);
    ck_assert(cypher_astnode_instanceof(option, CYPHER_AST_STATEMENT_OPTION));
    ck_assert_int_eq(cypher_astnode_type(option), CYPHER_AST_PROFILE_OPTION);
}
END_TEST


TCase* statement_tcase(void)
{
    TCase *tc = tcase_create("statement");
    tcase_add_checked_fixture(tc, setup, teardown);
    tcase_add_test(tc, parse_statement_with_no_options);
    tcase_add_test(tc, parse_statement_with_cypher_option);
    tcase_add_test(tc, parse_statement_with_cypher_option_containing_version);
    tcase_add_test(tc, parse_statement_with_cypher_option_containing_params);
    tcase_add_test(tc, parse_statement_with_cypher_option_containing_version_and_params);
    tcase_add_test(tc, parse_statement_with_explain_option);
    tcase_add_test(tc, parse_statement_with_profile_option);
    tcase_add_test(tc, parse_statement_with_multiple_options);
    return tc;
}
