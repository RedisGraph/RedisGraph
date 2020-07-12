import sys
import os
import ast

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../utils/')

import assertions
import graphs

resultset = None
exception = None
params = None

def before_feature(context):
    global params
    params = None

@given(u'the binary-tree-1 graph')
def step_impl(context):
    graphs.binary_tree_graph1()

@given(u'the binary-tree-2 graph')
def step_impl(context):
    graphs.binary_tree_graph2()

@given(u'any graph')
def step_impl(context):
    graphs.any_graph()

@given(u'an empty graph')
def step_impl(context):
    graphs.any_graph()

@then(u'parameters are')
@given(u'parameters are')
def set_params(context):
    global params

    params = {}
    for row in context.table.rows:
        params[row[0]]=row[1]

@given(u'having executed')
@when(u'having executed')
@then(u'having executed')
@when(u'executing control query')
@when(u'executing query')
def step_impl(context):
    global resultset
    global exception
    global params

    exception = None
    query = context.text
    try:
        resultset = graphs.query(query, params)
    except Exception as error:
        resultset = None
        exception = error
    params = None

@then(u'the result should be empty')
def step_impl(context):
    assertions.assert_empty_resultset(resultset)

@then(u'the side effects should be')
def step_impl(context):
    stat = context.table.headings[0]
    value = int(context.table.headings[1])
    assertions.assert_statistics(resultset, stat, value)

    for row in context.table:
        stat = row[0]
        value = int(row[1])
        assertions.assert_statistics(resultset, stat, value)

@then(u'no side effects')
def step_impl(context):
    assertions.assert_no_modifications(resultset)

@then(u'the result should be')
def step_impl(context):
    if exception:
        raise exception
    expected_length = len(context.table.rows)
    assertions.assert_resultset_length(resultset, expected_length)
    assertions.assert_resultsets_equals(resultset, context.table)

@then(u'the result should be, in order')
def step_impl(context):
    if exception:
        raise exception
    expected_length = len(context.table.rows)
    assertions.assert_resultset_length(resultset, expected_length)
    assertions.assert_resultsets_equals_in_order(resultset, context.table)

@then(u'a TypeError should be raised at runtime: PropertyAccessOnNonMap')
def step_impl(context):
    global exception
    assert exception != None
    assert "expected a map" in exception.message

@then(u'a ArgumentError should be raised at runtime: NumberOutOfRange')
def step_impl(context):
    global exception
    assert exception != None
    assert "ArgumentError" in exception.message

@then(u'a SyntaxError should be raised at compile time: InvalidAggregation')
def step_impl(context):
    assert exception != None
    assert "Invalid use of aggregating function" in exception.message

@then(u'a SyntaxError should be raised at compile time: UndefinedVariable')
def step_impl(context):
    assert exception != None
    assert "not defined" in exception.message

@then(u'a SyntaxError should be raised at compile time: VariableAlreadyBound')
def step_impl(context):
    global exception
    assert exception != None
    assert "can't be redeclared" in exception.message

@then(u'a TypeError should be raised at runtime: ListElementAccessByNonInteger')
def step_impl(context):
    global exception
    assert exception != None
    assert "expected Integer" in exception.message

@then(u'a SyntaxError should be raised at compile time: InvalidArgumentType')
def step_impl(context):
    global exception
    assert exception != None
    assert "Type mismatch" in exception.message


@then(u'a TypeError should be raised at runtime: InvalidArgumentValue')
def step_impl(context):
    global exception
    assert exception != None
    assert "Type mismatch" in exception.message

@then(u'a SyntaxError should be raised at compile time: DifferentColumnsInUnion')
def step_impl(context):
    global exception
    assert exception != None
    assert "must have the same column names." in exception.message

@then(u'a SyntaxError should be raised at compile time: InvalidClauseComposition')
def step_impl(context):
    global exception
    assert exception != None
    assert "Invalid combination" in exception.message

@then(u'a SyntaxError should be raised at compile time: NestedAggregation')
def step_impl(context):
    global exception
    assert exception != None
    assert "Can't use aggregate functions inside of aggregate functions" in exception.message

@then(u'a SyntaxError should be raised at compile time: UnknownFunction')
def step_impl(context):
    global exception
    assert exception != None
    assert "Unknown function" in exception.message

@then(u'a SyntaxError should be raised at compile time: NonConstantExpression')
def step_impl(context):
    global exception
    assert exception != None
    assert "invalid type" in exception.message

@then(u'a SyntaxError should be raised at compile time: NoExpressionAlias')
def step_impl(context):
    global exception
    assert exception != None
    assert "must be aliased" in exception.message

@then(u'a SyntaxError should be raised at compile time: InvalidNumberLiteral')
def step_impl(context):
    global exception
    assert exception != None
    assert "Invalid numeric value" in exception.message

@then(u'a SyntaxError should be raised at compile time: RequiresDirectedRelationship')
def step_impl(context):
    global exception
    assert exception != None
    assert "Only directed relationships" in exception.message
