import os
import sys
import unittest
from redisgraph import Graph, Node, Edge

import redis
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
from disposableredis import DisposableRedis

from base import FlowTestsBase

redis_graph = None

def disposable_redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

class QueryValidationFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "QueryValidationFlowTest"
        global redis_graph
        cls.r = disposable_redis()
        cls.r.start()
        redis_con = cls.r.client()
        
        # Create a single graph.
        redis_graph = Graph("G", redis_con)
        node = Node(properties={"age": 34})
        redis_graph.add_node(node)
        redis_graph.commit()

    @classmethod
    def tearDownClass(cls):
        cls.r.stop()
        # pass

    # Expect an error when trying to use a function which does not exists.
    def test01_none_existing_function(self):
        query = """MATCH (n) RETURN noneExistingFunc(n.age) AS cast"""
        try:
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    # Make sure function validation is type case insensitive.
    def test02_case_insensitive_function_name(self):
        try:
            query = """MATCH (n) RETURN mAx(n.age)"""
            redis_graph.query(query)
        except redis.exceptions.ResponseError:
            # function validation should be case insensitive.
            assert(False)
    
    def test03_edge_missing_relation_type(self):
        try:
            query = """CREATE (n:Person {age:32})-[]->(:person {age:30})"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    def test04_escaped_quotes(self):
       query = r"CREATE (:escaped{prop1:'single \' char', prop2: 'double \" char', prop3: 'mixed \' and \" chars'})"
       actual_result = redis_graph.query(query)
       assert(actual_result.nodes_created == 1)
       assert(actual_result.properties_set == 3)

       query = r"MATCH (a:escaped) RETURN a.prop1, a.prop2, a.prop3"
       actual_result = redis_graph.query(query)
       expected_result = [["single ' char", 'double " char', 'mixed \' and " chars']]

       assert(actual_result.result_set == expected_result)


if __name__ == '__main__':
    unittest.main()
