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

# prepare the actual value returned from redisgraph to be in 
# comparison vaiable format of the TCK feature files expected values
def prepareActualValue(actualValue):
    # if value is a numeric string or a number, transform to numeric value
    if is_numeric(actualValue):
        actualValue = stringToNumeric(actualValue)
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
        expectedValue = stringToNumeric(expectedValue)
    return expectedValue

def removeQuotes(value):
    value = value.replace("'", "")
    value = value.replace('"', "")  
    return value

def stringToNumeric(value):
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

def assert_empty_resultset(resultset):
    assert len(resultset.result_set) is 0

def assert_property(resultset, stat, value):
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

def assert_no_modifications(resultset):
    assert((resultset.nodes_created + resultset.nodes_deleted +
           resultset.properties_set + resultset.relationships_created +
           resultset.relationships_deleted) == 0)

def assert_resultset_length(resultset, length):
    assert(len(resultset.result_set) == length)

def assert_resultset_content(resultset, expected):
    rowCount = len(expected.rows)
    for rowIdx in range(rowCount):
        actualRow = resultset.result_set[rowIdx]
        actualRowLen = len(actualRow)
        expectedRow = expected.rows[rowIdx]        
        expectedRowLength = len(expectedRow)
        assert(expectedRowLength == actualRowLen)
        for cellIdx in range(expectedRowLength):
            # Strip value from single quotes.
            actualValue = prepareActualValue(actualRow[cellIdx])
            expectedValue = prepareExpectedValue(expectedRow[cellIdx])
            assert actualValue == expectedValue , "actualCell: %s differ from expectedCell: %s\n" % (actualValue, expectedValue)
