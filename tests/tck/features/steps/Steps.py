import sys
import os

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../utils/')

import assertions
import graphs

resultset = None


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

@given(u'having executed')
def step_impl(context):
    global resultset
    query = context.text
    resultset = graphs.query(query)

@when(u'having executed')
def step_impl(context):
    global resultset
    query = context.text
    resultset = graphs.query(query)


@then(u'having executed')
def step_impl(context):
    global resultset
    query = context.text
    resultset = graphs.query(query)

@when(u'executing control query')
def step_impl(context):
    global resultset
    query = context.text
    resultset = graphs.query(query)

@when(u'executing query')
def step_impl(context):
    global resultset
    query = context.text
    resultset = graphs.query(query)

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
    expected_length = len(context.table.rows)
    assertions.assert_resultset_length(resultset, expected_length)
    assertions.assert_resultsets_equals(resultset, context.table)

@then(u'the result should be, in order')
def step_impl(context):
    # TODO: validate ORDER.
    expected_length = len(context.table.rows)
    assertions.assert_resultset_length(resultset, expected_length)
