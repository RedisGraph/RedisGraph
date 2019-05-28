def is_number_tryexcept(s):
    """ Returns True is string is a number. """
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
            actualCell = actualRow[cellIdx]
            expectedCell = expectedRow[cellIdx]
            if isinstance(actualCell, basestring):
                actualCell = actualCell.replace("'", "")
                actualCell = actualCell.replace('"', "")
            if isinstance(expectedCell, basestring):
                expectedCell = expectedCell.replace("'", "")
                expectedCell = expectedCell.replace('"', "")

            # Cast to integer if possible.
            if is_number_tryexcept(actualCell):
                actualCell = float(actualCell)
                if actualCell.is_integer():
                    actualCell = int(actualCell)
            
            if is_number_tryexcept(expectedCell):
                expectedCell = float(expectedCell)
                if expectedCell.is_integer():
                    expectedCell = int(expectedCell)

            # if actualCell is not expectedCell:
            #     print "actualCell: %s differ from expectedCell: %s\n" % (actualCell, expectedCell)
            # else:
            #     print "PERFECTO!\n"
            assert actualCell == expectedCell, "actualCell: %s differ from expectedCell: %s\n" % (actualCell, expectedCell)
