import sys
import os

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../utils/')

import assertions
import graphs

resultset = None

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
    assertions.assert_property(resultset, stat, value)

    for row in context.table:
        stat = row[0]
        value = int(row[1])
        assertions.assert_property(resultset, stat, value)

@then(u'no side effects')
def step_impl(context):
    assertions.assert_no_modifications(resultset)

@then(u'the result should be')
def step_impl(context):
    # +1 for the header row.
    expected_length = len(context.table.rows) + 1
    assertions.assert_resultset_length(resultset, expected_length)
    assertions.assert_resultset_content(resultset, context.table)

@then(u'the result should be, in order')
def step_impl(context):
    # +1 for the header row.
    # TODO: validate ORDER.
    expected_length = len(context.table.rows) + 1
    assertions.assert_resultset_length(resultset, expected_length)
