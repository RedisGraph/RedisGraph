import os
import sys
import redis
import string
import random
import unittest
from base import FlowTestsBase
from redisgraph import Graph, Node, Edge

dis_redis = None
redis_graph = None
male = ["Roi", "Alon", "Omri"]
female = ["Hila", "Lucy"]

def random_string(size=6, chars=string.ascii_letters):
    return ''.join(random.choice(chars) for _ in range(size))

def get_redis():
    global dis_redis
    conn = redis.Redis()
    try:
        conn.ping()
        # Assuming RedisGraph is loaded.
    except redis.exceptions.ConnectionError:
        from .disposableredis import DisposableRedis
        # Bring up our own redis-server instance.
        dis_redis = DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')
        dis_redis.start()
        conn = dis_redis.client()
    return conn

class GraphMixLabelsFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "GraphMixLabelsFlowTest"
        global redis_graph
        global redis_con
        redis_con = get_redis()
        GRAPH_ID = random_string()
        redis_graph = Graph(GRAPH_ID, redis_con)
        cls.populate_graph()

    @classmethod
    def tearDownClass(cls):
        if dis_redis is not None:
            dis_redis.stop()

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
        assert (len(actual_result.result_set)-1 == (len(male) * (len(male + female)-1)))
    
    def test_male_to_male(self):
        query = """MATCH (m:male)-[:knows]->(t:male) RETURN m,t ORDER BY m.name"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set)-1 == (len(male) * (len(male)-1)))
    
    def test_male_to_female(self):
        query = """MATCH (m:male)-[:knows]->(t:female) RETURN m,t ORDER BY m.name"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set)-1 == (len(male) * len(female)))
    
    def test_female_to_all(self):
        query = """MATCH (f:female)-[:knows]->(t) RETURN f,t ORDER BY f.name"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set)-1 == (len(female) * (len(male + female)-1)))

    def test_female_to_male(self):
        query = """MATCH (f:female)-[:knows]->(t:male) RETURN f,t ORDER BY f.name"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set)-1 == (len(female) * len(male)))
    
    def test_female_to_female(self):
        query = """MATCH (f:female)-[:knows]->(t:female) RETURN f,t ORDER BY f.name"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set)-1 == (len(female) * (len(female)-1)))
    
    def test_all_to_female(self):
        query = """MATCH (f)-[:knows]->(t:female) RETURN f,t ORDER BY f.name"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set)-1 == (len(male) * len(female)) + (len(female) * (len(female)-1)))

    def test_all_to_male(self):
        query = """MATCH (f)-[:knows]->(t:male) RETURN f,t ORDER BY f.name"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set)-1 == (len(male) * (len(male)-1)) + len(female) * len(male))
    
    def test_all_to_all(self):
        query = """MATCH (f)-[:knows]->(t) RETURN f,t ORDER BY f.name"""
        actual_result = redis_graph.query(query)
        assert (len(actual_result.result_set)-1 == (len(male+female) * (len(male+female)-1)))

if __name__ == '__main__':
    unittest.main()
