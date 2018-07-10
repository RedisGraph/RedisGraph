import os
import sys
import unittest
from redisgraph import Graph, Node, Edge

# import redis
from .disposableredis import DisposableRedis

from base import FlowTestsBase

redis_graph = None

def redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

class GraphDeletionFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
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
        global redis_graph
        
        nodes = {}
         # Create entities
        people = ["Roi", "Alon", "Ailon", "Boaz", "Tal", "Omri", "Ori"]
        for p in people:
            node = Node(label="person", properties={"name": p})
            redis_graph.add_node(node)
            nodes[p] = node

        # Fully connected graph
        for src in nodes:
            for dest in nodes:
                if src != dest:
                    edge = Edge(nodes[src], "know", nodes[dest])
                    redis_graph.add_edge(edge)

        redis_graph.commit()

    # Delete edges pointing into either Boaz or Ori.
    def test01_delete_edges(self):
        global redis_graph
        query = """MATCH(s:person)-[e:know]->(d:person) WHERE d.name = "Boaz" OR d.name = "Ori" DELETE e"""
        actual_result = redis_graph.query(query)
        assert (actual_result.relationships_deleted == 12)
        assert (actual_result.nodes_deleted == 0)

    # Make sure there are no edges going into either Boaz or Ori.
    def test02_verify_edge_deletion(self):
        global redis_graph
        query = """MATCH(s:person)-[e:know]->(d:person)                    
                    WHERE d.name = "Boaz" AND d.name = "Ori"
                    RETURN COUNT(s)"""
        actual_result = redis_graph.query(query)
        assert(actual_result.result_set is None)

    # Remove both Alon and Boaz from the graph. 
    def test03_delete_nodes(self):
        global redis_graph
        query = """MATCH(s:person)
                    WHERE s.name = "Boaz" OR s.name = "Alon"
                    DELETE s"""
        actual_result = redis_graph.query(query)        
        assert (actual_result.relationships_deleted == 0)
        assert (actual_result.nodes_deleted == 2)

    # Make sure Alon and Boaz are not in the graph.
    def test04_get_deleted_nodes(self):
        global redis_graph
        query = """MATCH(s:person)
                    WHERE s.name = "Boaz" OR s.name = "Alon"
                    RETURN s"""
        actual_result = redis_graph.query(query)
        assert(actual_result.result_set is None)

    # Make sure Alon and Boaz are the only removed nodes.
    def test05_verify_node_deletion(self):
        global redis_graph
        query = """MATCH(s:person)
                   RETURN COUNT(s)"""
        actual_result = redis_graph.query(query)
        nodeCount = int(float(actual_result.result_set[1][0]))
        assert(nodeCount == 5)

if __name__ == '__main__':
    unittest.main()
