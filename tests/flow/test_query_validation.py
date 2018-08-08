import os
import sys
import unittest
from redisgraph import Graph, Node, Edge

import redis
from .disposableredis import DisposableRedis

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
        global redis_graph
        query = """MATCH (n) RETURN noneExistingFunc(n.age) AS cast"""
        try:
            result = redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    # Make sure function validation is type case insensitive.
    def test02_case_insensitive_function_name(self):
        global redis_graph
        try:
            query = """MATCH (n) RETURN mAx(n.age)"""
            result = redis_graph.query(query)
        except redis.exceptions.ResponseError:
            # function validation should be case insensitive.
            assert(False)

if __name__ == '__main__':
    unittest.main()
