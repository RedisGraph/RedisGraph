def assert_empty_resultset(resultset):
    assert(resultset.result_set is None)

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
    print "len(resultset.result_set): %d" % len(resultset.result_set)
    print "length: %d" % length
    assert(len(resultset.result_set) == length)
