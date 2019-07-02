from numbers import Number
from redisgraph import Node, Edge

# Returns True if value is a number or string representation of a number.
def is_numeric(value): 
    # check for value's type to be a number or a string
    if not isinstance(value, (Number, basestring)):
        return False
    try:
        # value is either number or string, try to convert to float
        float(value)
        # conversion succeed
        return True
    except ValueError:
        # value was a string not representing a number
        return False

def removeQuotes(value):
    value = value.replace("'", "")
    value = value.replace('"', "")  
    return value

def toNumeric(value):
    value = float(value)
    if value.is_integer():
        value = int(value)
    return value

def nodeToString(value):
    res = '('
    if value.alias:
        res += value.alias
    if value.label:
        res += ':' + value.label
    if value.properties:
        props = ','.join(key+': '+str(val) for key, val in value.properties.items())
        if value.label:
            res+=" "
        res += '{' + props + '}'
    res += ')'
    value = res  
    return value

def edgeToString(value):
    res = "["
    if value.relation:
        res += ":" + value.relation
    if value.properties:
        props = ','.join(key+': '+str(val) for key, val in value.properties.items())
        if value.relation:
            res+=" "
        res += '{' + props + '}'
    res += ']'
    value = res
    return value
    
# prepare the actual value returned from redisgraph to be in 
# comparison vaiable format of the TCK feature files expected values
def prepareActualValue(actualValue):
    # if value is a numeric string or a number, transform to numeric value
    if is_numeric(actualValue):
        actualValue = toNumeric(actualValue)
    # value is string
    elif isinstance(actualValue, basestring):
        # remove qoutes if any
        actualValue = removeQuotes(actualValue)
    # value is a node
    elif isinstance(actualValue, Node):
        actualValue = nodeToString(actualValue)
    # value is an edge
    elif isinstance(actualValue, Edge):
        actualValue = edgeToString(actualValue)
    else:
        # actual value is null or boolean
        assert isinstance(actualValue, (type(None), bool))
    return actualValue

# prepare the expected value to be in comparison vaiable format
def prepareExpectedValue(expectedValue):
    # the expected value is always string. Do a string preparation
    expectedValue = removeQuotes(expectedValue)
    # in case of boolean value string
    if expectedValue == "true":
        expectedValue = True
    elif expectedValue == "false":
        expectedValue = False
    elif expectedValue == "null":
        expectedValue = None
    # in case of numeric string
    elif is_numeric(expectedValue):
        expectedValue = toNumeric(expectedValue)
    return expectedValue

def assert_empty_resultset(resultset):
    assert len(resultset.result_set) is 0

# check value of a designated statistic
def assert_statistics(resultset, stat, value):
    if stat == "+nodes":
        assert(resultset.nodes_created == value)
    elif stat == "+relationships":
        assert(resultset.relationships_created == value)
    elif stat == "+labels":
        assert(resultset.labels_added == value)
    elif stat == "+properties":
        assert(resultset.properties_set == value)        
    elif stat == "-nodes":
        assert(resultset.nodes_deleted == value)
    else:
        assert(False)

# checks resultset statistics for no graph modifications
def assert_no_modifications(resultset):
    assert(sum([resultset.nodes_created, resultset.nodes_deleted,
           resultset.properties_set, resultset.relationships_created,
           resultset.relationships_deleted]) == 0)

def assert_resultset_length(resultset, length):
    assert(len(resultset.result_set) == length)

def assert_values_equal(actual, expected):
    # prepare values and compare
    actual = prepareActualValue(actual)
    expected = prepareExpectedValue(expected)
    assert actual == expected , "actualCell: %s differ from expectedCell: %s\n" % (actual, expected)

def assert_rows_equal(actualRow, expectedRow):
    expectedRowLength = len(expectedRow)
    for cellIdx in range(expectedRowLength):
        # compare each value
        assert_values_equal(actualRow[cellIdx], expectedRow[cellIdx])

def assert_resultsets_equals(actual, expected):
    rowCount = len(expected.rows)
    # check amount of rows
    assert_resultset_length(actual, rowCount)
    if rowCount > 0:
        # if there are rows, check row length
        assert len(actual.result_set[0]) == len(expected.rows[0])
        for rowIdx in range(rowCount):
            actualRow = actual.result_set[rowIdx]
            expectedRow = expected.rows[rowIdx]
            # compare rows
            assert_rows_equal(actualRow, expectedRow)
            