from numbers import Number
from redisgraph import Node, Edge
def is_number_tryexcept(s):
    """ Returns True if string is a number. """
    if not isinstance(s, (Number, basestring)):
        return False
    try:
        float(s)
        return True
    except ValueError:
        return False
    
def prepareActualValue(actualValue):
        actualValue = prepareString(actualValue)
        actualValue = stringToNumeric(actualValue)
        actualValue = nodeToString(actualValue)
        actualValue = edgeToString(actualValue)
        return actualValue

def prepareExpectedValue(expectedValue):
    # string preparation
    expectedValue = prepareString(expectedValue)
    if expectedValue == "true":
        expectedValue = True
    if expectedValue == "false":
        expectedValue = False
    if expectedValue == "null":
        expectedValue = None
    expectedValue = stringToNumeric(expectedValue)
    return expectedValue

def prepareString(value):
    if isinstance(value, basestring):
        value = value.replace("'", "")
        value = value.replace('"', "")
    return value

def stringToNumeric(value):
    if is_number_tryexcept(value):
        value = float(value)
        if value.is_integer():
            value = int(value)
    return value

def nodeToString(value):
    if isinstance(value, Node):
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
    if isinstance(value, Edge):
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
    resultset.pretty_print()
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
            expectedValue= prepareExpectedValue(expectedRow[cellIdx])
            assert actualValue == expectedValue , "actualCell: %s differ from expectedCell: %s\n" % (actualValue, expectedValue)
