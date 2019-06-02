import os
import sys
import unittest
from redisgraph import Graph, Node, Edge

# import redis
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
from disposableredis import DisposableRedis

from base import FlowTestsBase

GRAPH_ID = "G"
redis_graph = None

def redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

class GraphCreationFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "GraphCreationFlowTest"
        global redis_graph
        cls.r = redis()
        cls.r.start()
        redis_con = cls.r.client()
        redis_graph = Graph(GRAPH_ID, redis_con)

        # cls.r = redis.Redis()
        # redis_graph = Graph(GRAPH_ID, cls.r)

    @classmethod
    def tearDownClass(cls):
        cls.r.stop()
        # pass

    def test01_create_return(self):
        query = """CREATE (a:person {name:'A'}), (b:person {name:'B'})"""
        result = redis_graph.query(query)
        assert(result.nodes_created == 2)

        query = """MATCH (src:person) CREATE (src)-[e:knows]->(dest {name:'C'}) RETURN src,e,dest ORDER BY ID(src) DESC LIMIT 1"""
        result = redis_graph.query(query)
        assert(result.nodes_created == 2)
        assert(result.relationships_created == 2)
        assert(len(result.result_set) == 1)
        assert(result.result_set[0][0].properties['name'] == 'A')

if __name__ == '__main__':
    unittest.main()
