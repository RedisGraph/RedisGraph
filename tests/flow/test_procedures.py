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

node1 = Node(label="fruit", properties={"name": "Orange1", "value": 1})
node2 = Node(label="fruit", properties={"name": "Orange2", "value": 2})
node3 = Node(label="fruit", properties={"name": "Orange3", "value": 3})
node4 = Node(label="fruit", properties={"name": "Orange4", "value": 4})
node5 = Node(label="fruit", properties={"name": "Banana", "value": 5})

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
        edge = Edge(node1, 'goWellWith', node5)
        redis_graph.add_node(node1)
        redis_graph.add_node(node2)
        redis_graph.add_node(node3)
        redis_graph.add_node(node4)
        redis_graph.add_node(node5)
        redis_graph.add_edge(edge)
        redis_graph.commit()

    # Compares two nodes based on their properties.
    def _compareNodes(self, a, b):
        return a.properties == b.properties

    # Make sure given item is found within resultset.
    def _inResultSet(self, item, resultset):
        for i in range(len(resultset)):
            result = resultset[i][0]
            if _compareNodes(item, result):
                return True
        return False

    # Issue query and validates resultset.
    def queryAndValidate(self, query, expected_results):
        actual_resultset = redis_graph.query(query).result_set
        assert(len(actual_resultset) == len(expected_results))
        for i in range(len(actual_resultset)):
            assert(_inResultSet(expected_results[i], actual_resultset))
    
    # Call procedure, omit yield, expecting all procedure outputs to
    # be included in result-set.
    def test_no_yield(self):
        query = """CALL db.idx.fulltext.queryNodes('fruit', 'query')"""
        actual_result = redis_graph.query(query)        
        assert(len(actual_result.result_set) is 2)
        assert(len(actual_result.result_set[0]) is 2)
        assert(len(actual_result.result_set[1]) is 2)

        header = actual_result.result_set[0]
        data = actual_result.result_set[1]
        assert(header[0] == 'node')
        # assert(header[1] == 'score')
        assert(data[0] is None)
        assert(float(data[1]) == 12.34)

    # Call procedure specify different outputs.
    def test_yield(self):
        query = """CALL db.idx.fulltext.queryNodes('fruit', 'query') YIELD node"""
        actual_result = redis_graph.query(query)        
        assert(len(actual_result.result_set) is 2)
        assert(len(actual_result.result_set[0]) is 1)
        assert(len(actual_result.result_set[1]) is 1)

        header = actual_result.result_set[0]
        data = actual_result.result_set[1]
        assert(header[0] == 'node')        
        assert(data[0] is None)

        # query = """CALL db.idx.fulltext.queryNodes('fruit', 'query') YIELD score"""
        # actual_result = redis_graph.query(query)
        # assert(len(actual_result.result_set) is 2)
        # assert(len(actual_result.result_set[0]) is 1)
        # assert(len(actual_result.result_set[1]) is 1)

        # header = actual_result.result_set[0]
        # data = actual_result.result_set[1]
        # assert(header[0] == 'score')
        # assert(float(data[0]) == 12.34)

        # query = """CALL db.idx.fulltext.queryNodes('fruit', 'query') YIELD node, score"""
        # actual_result = redis_graph.query(query)
        # assert(len(actual_result.result_set) is 2)
        # assert(len(actual_result.result_set[0]) is 2)
        # assert(len(actual_result.result_set[1]) is 2)

        # header = actual_result.result_set[0]
        # data = actual_result.result_set[1]
        # assert(header[0] == 'node')
        # assert(header[1] == 'score')
        # assert(data[0] is None)
        # assert(float(data[1]) == 12.34)

        # query = """CALL db.idx.fulltext.queryNodes('fruit', 'query') YIELD score, node"""
        # actual_result = redis_graph.query(query)
        # assert(len(actual_result.result_set) is 2)
        # assert(len(actual_result.result_set[0]) is 2)
        # assert(len(actual_result.result_set[1]) is 2)

        # header = actual_result.result_set[0]
        # data = actual_result.result_set[1]
        # assert(header[0] == 'score')
        # assert(header[1] == 'node')
        # assert(float(data[0]) == 12.34)
        # assert(data[1] is None)
        
        # Yield an unknown output.
        query = """CALL db.idx.fulltext.queryNodes('fruit', 'query') YIELD unknown"""
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
        query = """CALL db.idx.fulltext.queryNodes('fruit', 'query', 'fruit', 'query') YIELD node"""
        # Expect an error when trying to send too many arguments.
        try:
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    # Test procedure call while mixing a number of addition clauses.
    def test_mix_clauses(self):        
        # Create full-text index.
        query = """CALL db.idx.fulltext.createNodeIndex('fruit', 'name')"""           
        graph.query(query)

        # CALL + RETURN.
        query = """CALL db.idx.fulltext.queryNodes('fruit', 'Orange*') YIELD node
                    RETURN node"""
        expected_results = [node4, node2, node3, node1]
        self.queryAndValidate(query, expected_results)


        # CALL + WHERE + RETURN.
        query = """CALL db.idx.fulltext.queryNodes('fruit', 'Orange*') YIELD node
                    WHERE node.value > 2
                    RETURN node
                    """
        expected_results = [node3, node4]
        self.queryAndValidate(query, expected_results)


        # CALL + WHERE + RETURN + SKIP.        
        query = """CALL db.idx.fulltext.queryNodes('fruit', 'Orange*') YIELD node
                    WHERE node.value > 2
                    RETURN node
                    SKIP 1"""
        expected_results = [node3]
        # not deterministic!
        # self.queryAndValidate(query, expected_results)


        # CALL + WHERE + RETURN + LIMIT.
        query = """CALL db.idx.fulltext.queryNodes('fruit', 'Orange*') YIELD node
                    WHERE node.value > 2
                    RETURN node
                    LIMIT 2"""
        expected_results = [node3, node4]
        self.queryAndValidate(query, expected_results)


        # CALL + WHERE + RETURN + SKIP + LIMIT.
        query = """CALL db.idx.fulltext.queryNodes('fruit', 'Orange*') YIELD node
                    WHERE node.value > 2
                    RETURN node
                    SKIP 1
                    LIMIT 1"""
        # not deterministic!
        # self.queryAndValidate(query, expected_results)


        # CALL + RETURN + ORDER.
        query = """CALL db.idx.fulltext.queryNodes('fruit', 'Orange*') YIELD node
                    RETURN node
                    ORDER BY node.value
                    """
        expected_results = [node1, node2, node3, node4]
        self.queryAndValidate(query, expected_results)


        # CALL + RETURN + ORDER + SKIP.
        query = """CALL db.idx.fulltext.queryNodes('fruit', 'Orange*') YIELD node
                    RETURN node
                    ORDER BY node.value
                    SKIP 1
                    """
        expected_results = [node2, node3, node4]
        self.queryAndValidate(query, expected_results)


        # CALL + RETURN + ORDER + LIMIT.
        query = """CALL db.idx.fulltext.queryNodes('fruit', 'Orange*') YIELD node
                    RETURN node
                    ORDER BY node.value
                    LIMIT 2
                    """
        expected_results = [node1, node2]
        self.queryAndValidate(query, expected_results)


        # CALL + RETURN + ORDER + SKIP + LIMIT.
        query = """CALL db.idx.fulltext.queryNodes('fruit', 'Orange*') YIELD node
                    RETURN node
                    ORDER BY node.value
                    SKIP 1
                    LIMIT 1
                    """
        expected_results = [node2]
        self.queryAndValidate(query, expected_results)


        # CALL + WHERE + RETURN + ORDER.
        query = """CALL db.idx.fulltext.queryNodes('fruit', 'Orange*') YIELD node
                    WHERE node.value > 2
                    RETURN node
                    ORDER BY node.value"""
        expected_results = [node3, node4]
        self.queryAndValidate(query, expected_results)


        # CALL + WHERE + RETURN + ORDER + SKIP.
        query = """CALL db.idx.fulltext.queryNodes('fruit', 'Orange*') YIELD node
                    WHERE node.value > 2
                    RETURN node
                    ORDER BY node.value
                    SKIP 1"""
        expected_results = [node4]
        self.queryAndValidate(query, expected_results)


        # CALL + WHERE + RETURN + ORDER + LIMIT.
        query = """CALL db.idx.fulltext.queryNodes('fruit', 'Orange*') YIELD node
                    WHERE node.value > 2
                    RETURN node
                    ORDER BY node.value
                    LIMIT 1"""
        expected_results = [node3]
        self.queryAndValidate(query, expected_results)


        # CALL + WHERE + RETURN + ORDER + SKIP + LIMIT.
        query = """CALL db.idx.fulltext.queryNodes('fruit', 'Orange*') YIELD node
                    WHERE node.value > 2
                    RETURN node
                    ORDER BY node.value
                    SKIP 1
                    LIMIT 1"""
        expected_results = [node4]
        self.queryAndValidate(query, expected_results)

        # CALL + MATCH + RETURN.
        query = """CALL db.idx.fulltext.queryNodes('fruit', 'Orange*') YIELD node
            MATCH (node)-[]->(z)
            RETURN z"""
        expected_results = [node5]
        queryAndValidate(query, expected_results)

if __name__ == '__main__':
    unittest.main()
