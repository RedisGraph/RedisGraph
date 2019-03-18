import os
import sys
import unittest
from redisgraph import Graph, Node, Edge

import redis
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
from disposableredis import DisposableRedis

from base import FlowTestsBase

GRAPH_ID = "procedures"
redis_graph = None

def _redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

# Tests built in procedures,
# e.g. db.idx.fulltext.queryNodes
# Test over all procedure behavior in addition to procedure specifics.
class ProceduresTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "ProceduresTest"
        global redis_graph
        cls.r = _redis()
        cls.r.start()
        redis_con = cls.r.client()
        redis_graph = Graph(GRAPH_ID, redis_con)

        # cls.r = redis.Redis()
        # redis_graph = Graph(GRAPH_ID, cls.r)

        cls.populate_graph()

    @classmethod
    def tearDownClass(cls):
        cls.r.stop()
        # pass

    @classmethod
    def populate_graph(cls):
        node = Node(label="person", properties={"name": "Roi"})
        redis_graph.add_node(node)
        redis_graph.commit()

    # Call procedure, omit yield, expecting all procedure outputs to
    # be included in result-set.
    def test_no_yield(self):
        query = """CALL db.idx.fulltext.queryNodes('person', 'query')"""
        actual_result = redis_graph.query(query)        
        assert(len(actual_result.result_set) is 2)
        assert(len(actual_result.result_set[0]) is 2)
        assert(len(actual_result.result_set[1]) is 2)

        header = actual_result.result_set[0]
        data = actual_result.result_set[1]
        assert(header[0] == 'node')
        assert(header[1] == 'score')
        assert(data[0] is None)
        assert(float(data[1]) == 12.34)

    # Call procedure specify different outputs.
    def test_yield(self):
        query = """CALL db.idx.fulltext.queryNodes('person', 'query') YIELD node"""
        actual_result = redis_graph.query(query)        
        assert(len(actual_result.result_set) is 2)
        assert(len(actual_result.result_set[0]) is 1)
        assert(len(actual_result.result_set[1]) is 1)

        header = actual_result.result_set[0]
        data = actual_result.result_set[1]
        assert(header[0] == 'node')        
        assert(data[0] is None)

        query = """CALL db.idx.fulltext.queryNodes('person', 'query') YIELD score"""
        actual_result = redis_graph.query(query)
        assert(len(actual_result.result_set) is 2)
        assert(len(actual_result.result_set[0]) is 1)
        assert(len(actual_result.result_set[1]) is 1)

        header = actual_result.result_set[0]
        data = actual_result.result_set[1]
        assert(header[0] == 'score')
        assert(float(data[0]) == 12.34)

        query = """CALL db.idx.fulltext.queryNodes('person', 'query') YIELD node, score"""
        actual_result = redis_graph.query(query)
        assert(len(actual_result.result_set) is 2)
        assert(len(actual_result.result_set[0]) is 2)
        assert(len(actual_result.result_set[1]) is 2)

        header = actual_result.result_set[0]
        data = actual_result.result_set[1]
        assert(header[0] == 'node')
        assert(header[1] == 'score')
        assert(data[0] is None)
        assert(float(data[1]) == 12.34)

        query = """CALL db.idx.fulltext.queryNodes('person', 'query') YIELD score, node"""
        actual_result = redis_graph.query(query)
        assert(len(actual_result.result_set) is 2)
        assert(len(actual_result.result_set[0]) is 2)
        assert(len(actual_result.result_set[1]) is 2)

        header = actual_result.result_set[0]
        data = actual_result.result_set[1]
        assert(header[0] == 'score')
        assert(header[1] == 'node')
        assert(float(data[0]) == 12.34)
        assert(data[1] is None)
        
        # Yield an unknown output.
        query = """CALL db.idx.fulltext.queryNodes('person', 'query') YIELD unknown"""
        # Expect an error when trying to use an unknown procedure output.
        try:
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass
    
    def test_arguments(self):
        # Omit arguments.
        query = """CALL db.idx.fulltext.queryNodes() YIELD score"""
        # Expect an error when trying to omit arguments.
        try:
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass
        
        # Omit arguments.
        query = """CALL db.idx.fulltext.queryNodes('arg1')"""
        # Expect an error when trying to omit arguments.
        try:
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

        # Overload arguments.
        query = """CALL db.idx.fulltext.queryNodes('person', 'query', 'person', 'query') YIELD node"""
        # Expect an error when trying to send too many arguments.
        try:
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

if __name__ == '__main__':
    unittest.main()
