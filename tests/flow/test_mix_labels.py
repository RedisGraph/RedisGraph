import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

redis_graph = None
male = ["Roi", "Alon", "Omri"]
female = ["Hila", "Lucy"]

class testGraphMixLabelsFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph("G", redis_con)
        self.populate_graph()

    def populate_graph(self):
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
        self.env.assertEquals(len(actual_result.result_set), (len(male) * (len(male + female)-1)))
    
    def test_male_to_male(self):
        query = """MATCH (m:male)-[:knows]->(t:male) RETURN m,t ORDER BY m.name"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(len(actual_result.result_set), (len(male) * (len(male)-1)))
    
    def test_male_to_female(self):
        query = """MATCH (m:male)-[:knows]->(t:female) RETURN m,t ORDER BY m.name"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(len(actual_result.result_set), (len(male) * len(female)))
    
    def test_female_to_all(self):
        query = """MATCH (f:female)-[:knows]->(t) RETURN f,t ORDER BY f.name"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(len(actual_result.result_set), (len(female) * (len(male + female)-1)))

    def test_female_to_male(self):
        query = """MATCH (f:female)-[:knows]->(t:male) RETURN f,t ORDER BY f.name"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(len(actual_result.result_set), (len(female) * len(male)))
    
    def test_female_to_female(self):
        query = """MATCH (f:female)-[:knows]->(t:female) RETURN f,t ORDER BY f.name"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(len(actual_result.result_set), (len(female) * (len(female)-1)))
    
    def test_all_to_female(self):
        query = """MATCH (f)-[:knows]->(t:female) RETURN f,t ORDER BY f.name"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(len(actual_result.result_set), (len(male) * len(female)) + (len(female) * (len(female)-1)))

    def test_all_to_male(self):
        query = """MATCH (f)-[:knows]->(t:male) RETURN f,t ORDER BY f.name"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(len(actual_result.result_set), (len(male) * (len(male)-1)) + len(female) * len(male))
    
    def test_all_to_all(self):
        query = """MATCH (f)-[:knows]->(t) RETURN f,t ORDER BY f.name"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(len(actual_result.result_set), (len(male+female) * (len(male+female)-1)))
