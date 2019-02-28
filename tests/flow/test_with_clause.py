import os
import sys
import unittest
from redisgraph import Graph, Node, Edge

# import redis
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
from disposableredis import DisposableRedis

from base import FlowTestsBase

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../demo/social/')
import social_utils

redis_graph = None

def redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

class WithClauseTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "WithClauseTest"
        global redis_graph
        cls.r = redis()
        cls.r.start()
        redis_con = cls.r.client()
        redis_graph = Graph("G", redis_con)

        social_utils.populate_graph(redis_con, redis_graph)

    @classmethod
    def tearDownClass(cls):
        cls.r.stop()
        # pass

    # Verify that the same entities are scanned when a WITH clause is specified
    def test01_validate_with_clause_aggregation(self):
        query = """MATCH (with_entity:person) WITH SUM(with_entity.age) AS with_sum MATCH (match_entity:person) RETURN with_sum, SUM(match_entity.age)"""
        actual_result = redis_graph.query(query)
        assert(len(actual_result.result_set) == 2)
        assert(actual_result.result_set[1][0] == actual_result.result_set[1][1])

if __name__ == '__main__':
    unittest.main()
