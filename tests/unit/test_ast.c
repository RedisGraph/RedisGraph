/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "../../src/parser/ast.h"
#include "../../src/query_executor.h"

void test_validate_set_clause() {
    char *undefined_alias;
    char *errMsg = NULL;

    // Valid query
    char *query = "MATCH (n { name: 'Andres' }) SET n.surname = 'Taylor'";
    AST_QueryExpressionNode* ast = ParseQuery(query, strlen(query), &errMsg);
    assert(ast != NULL);
    assert(_Validate_SET_Clause(ast, &undefined_alias) == AST_VALID);

    // invalid query
    query = "MATCH (n { name: 'Andres' }) SET n.surname = 'Taylor', m.middle_name = '23'";
    ast = ParseQuery(query, strlen(query), &errMsg);
    assert(ast != NULL);
    assert(_Validate_SET_Clause(ast, &undefined_alias) == AST_INVALID);
    assert(strcmp(undefined_alias, "m not defined") == 0);
    free(undefined_alias);

    // invalid query
    query = "MATCH (n { name: 'Andres' }) SET n.surname = 'Taylor', N.middle_name = '23'";
    ast = ParseQuery(query, strlen(query), &errMsg);
    assert(ast != NULL);
    assert(_Validate_SET_Clause(ast, &undefined_alias) == AST_INVALID);
    assert(strcmp(undefined_alias, "N not defined") == 0);
    free(undefined_alias);
}

void test_validate_delete_clause() {
    char *undefined_alias;
    char *errMsg = NULL;

    // Valid query
    char *query = "MATCH (n { name: 'Andres' }) DELETE n";
    AST_QueryExpressionNode* ast = ParseQuery(query, strlen(query), &errMsg);
    assert(ast != NULL);
    assert(_Validate_DELETE_Clause(ast, &undefined_alias) == AST_VALID);

    // invalid query
    query = "MATCH (n { name: 'Andres' }) DELETE n, m";
    ast = ParseQuery(query, strlen(query), &errMsg);
    assert(ast != NULL);
    assert(_Validate_DELETE_Clause(ast, &undefined_alias) == AST_INVALID);
    assert(strcmp(undefined_alias, "m not defined") == 0);
    free(undefined_alias);

    // invalid query
    query = "MATCH (n { name: 'Andres' }) DELETE n, N";
    ast = ParseQuery(query, strlen(query), &errMsg);
    assert(ast != NULL);
    assert(_Validate_DELETE_Clause(ast, &undefined_alias) == AST_INVALID);
    assert(strcmp(undefined_alias, "N not defined") == 0);
    free(undefined_alias);
}

void test_validate_return_clause() {
    char *undefined_alias;
    char *errMsg = NULL;

    // Valid query
    char *query = "MATCH (n { name: 'Andres' }) RETURN n";
    AST_QueryExpressionNode* ast = ParseQuery(query, strlen(query), &errMsg);
    assert(ast != NULL);
    assert(_Validate_RETURN_Clause(ast, &undefined_alias) == AST_VALID);

    // Valid query
    query = "MATCH (n { name: 'Andres' }) RETURN AVG(2 / ABS(n.age))";
    ast = ParseQuery(query, strlen(query), &errMsg);
    assert(ast != NULL);
    assert(_Validate_RETURN_Clause(ast, &undefined_alias) == AST_VALID);

    // invalid query
    query = "MATCH (n { name: 'Andres' }) RETURN AVG(2 / ABS(m.age))";
    ast = ParseQuery(query, strlen(query), &errMsg);
    assert(ast != NULL);
    assert(_Validate_RETURN_Clause(ast, &undefined_alias) == AST_INVALID);
    assert(strcmp(undefined_alias, "m not defined") == 0);
    free(undefined_alias);

    // invalid query
    query = "MATCH (n { name: 'Andres' }) RETURN n, m";
    ast = ParseQuery(query, strlen(query), &errMsg);
    assert(ast != NULL);
    assert(_Validate_RETURN_Clause(ast, &undefined_alias) == AST_INVALID);
    assert(strcmp(undefined_alias, "m not defined") == 0);
    free(undefined_alias);

    // invalid query
    query = "MATCH (n { name: 'Andres' }) RETURN n, N";
    ast = ParseQuery(query, strlen(query), &errMsg);
    assert(ast != NULL);
    assert(_Validate_RETURN_Clause(ast, &undefined_alias) == AST_INVALID);
    assert(strcmp(undefined_alias, "N not defined") == 0);
    free(undefined_alias);
}

int main(int argc, char **argv) {
    test_validate_set_clause();
	test_validate_delete_clause();
    test_validate_return_clause();
	printf("test_ast - PASS!\n");
    return 0;
}
