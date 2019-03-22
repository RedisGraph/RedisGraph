import os
import sys
import unittest
from redisgraph import Graph, Node, Edge

# import redis
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
from disposableredis import DisposableRedis

from base import FlowTestsBase

redis_graph = None
male = ["Roi", "Alon", "Omri"]
female = ["Hila", "Lucy"]

def redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

class GraphMixLabelsFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "GraphMixLabelsFlowTest"
        global redis_graph
        cls.r = redis()
        cls.r.start()
        redis_con = cls.r.client()
        redis_graph = Graph("G", redis_con)

        # cls.r = redis.Redis()
        # redis_graph = Graph("G", cls.r)

        cls.populate_graph()

    @classmethod
    def tearDownClass(cls):
        cls.r.stop()
        # pass

    @classmethod
    def populate_graph(cls):
        redis_graph
        
        nodes = {}
         # Create entities
        
        for m in male:
            node = Node(label="male", properties={"name": m})
            redis_graph.add_node(node)
            nodes[m] = node
        
        for f in female:
            node = Node(label="female", properties={"name": f})
            redis_graph.add_node(node)
            nodes[f] = node

        for n in nodes:
            for m in nodes:
                if n == m: continue
                edge = Edge(nodes[n], "knows", nodes[m])
                redis_graph.add_edge(edge)

        redis_graph.commit()

    # Connect a single node to all other nodes.
    def test_male_to_all(self):
        query = """MATCH (m:male)-[:knows]->(t) RETURN m,t ORDER BY m.name"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set) == (len(male) * (len(male + female)-1)))
    
    def test_male_to_male(self):
        query = """MATCH (m:male)-[:knows]->(t:male) RETURN m,t ORDER BY m.name"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set) == (len(male) * (len(male)-1)))
    
    def test_male_to_female(self):
        query = """MATCH (m:male)-[:knows]->(t:female) RETURN m,t ORDER BY m.name"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set) == (len(male) * len(female)))
    
    def test_female_to_all(self):
        query = """MATCH (f:female)-[:knows]->(t) RETURN f,t ORDER BY f.name"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set) == (len(female) * (len(male + female)-1)))

    def test_female_to_male(self):
        query = """MATCH (f:female)-[:knows]->(t:male) RETURN f,t ORDER BY f.name"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set) == (len(female) * len(male)))
    
    def test_female_to_female(self):
        query = """MATCH (f:female)-[:knows]->(t:female) RETURN f,t ORDER BY f.name"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set) == (len(female) * (len(female)-1)))
    
    def test_all_to_female(self):
        query = """MATCH (f)-[:knows]->(t:female) RETURN f,t ORDER BY f.name"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set) == (len(male) * len(female)) + (len(female) * (len(female)-1)))

    def test_all_to_male(self):
        query = """MATCH (f)-[:knows]->(t:male) RETURN f,t ORDER BY f.name"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set) == (len(male) * (len(male)-1)) + len(female) * len(male))
    
    def test_all_to_all(self):
        query = """MATCH (f)-[:knows]->(t) RETURN f,t ORDER BY f.name"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set) == (len(male+female) * (len(male+female)-1)))

if __name__ == '__main__':
    unittest.main()
