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
            actualCell = actualRow[cellIdx]
            expectedCell = expectedRow[cellIdx]
            if isinstance(actualCell, basestring):
                actualCell = actualCell.replace("'", "")
                actualCell = actualCell.replace('"', "")
            if isinstance(expectedCell, basestring):
                expectedCell = expectedCell.replace("'", "")
                expectedCell = expectedCell.replace('"', "")
                if expectedCell == "true":
                    expectedCell = True
                if expectedCell == "false":
                    expectedCell = False
                if expectedCell == "null":
                    expectedCell = None
                    
            # Cast to integer if possible.
            if is_number_tryexcept(actualCell):
                actualCell = float(actualCell)
                if actualCell.is_integer():
                    actualCell = int(actualCell)
            
            if is_number_tryexcept(expectedCell):
                expectedCell = float(expectedCell)
                if expectedCell.is_integer():
                    expectedCell = int(expectedCell)

            if isinstance(actualCell, Node):
                res = '('
                if actualCell.alias:
                    res += actualCell.alias
                if actualCell.label:
                    res += ':' + actualCell.label
                if actualCell.properties:
                    props = ','.join(key+': '+str(val) for key, val in actualCell.properties.items())
                    if actualCell.label:
                        res+=" "
                    res += '{' + props + '}'
                res += ')'
                actualCell = res

            if isinstance(actualCell, Edge):
                res = "["
                if actualCell.relation:
                    res += ":" + actualCell.relation
                if actualCell.properties:
                    props = ','.join(key+': '+str(val) for key, val in actualCell.properties.items())
                    if actualCell.relation:
                        res+=" "
                    res += '{' + props + '}'
                res += ']'
                actualCell = res
            # if actualCell is not expectedCell:
            #     print "actualCell: %s differ from expectedCell: %s\n" % (actualCell, expectedCell)
            # else:
            #     print "PERFECTO!\n"
            assert actualCell == expectedCell , "actualCell: %s differ from expectedCell: %s\n" % (actualCell, expectedCell)
