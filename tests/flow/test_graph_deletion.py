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

class GraphDeletionFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "GraphDeletionFlowTest"
        global redis_graph
        cls.r = redis()
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

        # Connect Roi to Alon via another edge type.
        edge = Edge(nodes["Roi"], "SameBirthday", nodes["Alon"])
        redis_graph.add_edge(edge)
        redis_graph.commit()

    # Delete edges pointing into either Boaz or Ori.
    def test01_delete_edges(self):
        query = """MATCH (s:person)-[e:know]->(d:person) WHERE d.name = "Boaz" OR d.name = "Ori" DELETE e"""
        actual_result = redis_graph.query(query)
        assert (actual_result.relationships_deleted == 12)
        assert (actual_result.nodes_deleted == 0)

    # Make sure there are no edges going into either Boaz or Ori.
    def test02_verify_edge_deletion(self):
        query = """MATCH (s:person)-[e:know]->(d:person)                    
                    WHERE d.name = "Boaz" AND d.name = "Ori"
                    RETURN COUNT(s)"""
        actual_result = redis_graph.query(query)
        assert(len(actual_result.result_set) is 1)

    # Remove 'know' edge connecting Roi to Alon
    # Leaving a single edge of type SameBirthday
    # connecting the two.
    def test03_delete_typed_edge(self):
        query = """MATCH (s:person {name: "Roi"})-[e:know]->(d:person {name: "Alon"})
                   DELETE e"""
        actual_result = redis_graph.query(query)
        assert (actual_result.relationships_deleted == 1)
        assert (actual_result.nodes_deleted == 0)

    # Make sure Roi is still connected to Alon
    # via the "SameBirthday" type edge.
    def test04_verify_delete_typed_edge(self):
        query = """MATCH (s:person {name: "Roi"})-[e:SameBirthday]->(d:person {name: "Alon"})
                   RETURN COUNT(s)"""
        actual_result = redis_graph.query(query)
        assert(len(actual_result.result_set) is 2) # header row + record.

        query = """MATCH (s:person {name: "Roi"})-[e:know]->(d:person {name: "Alon"})
                   RETURN COUNT(s)"""
        actual_result = redis_graph.query(query)
        assert(len(actual_result.result_set) is 1) # Only header row.

    # Remove both Alon and Boaz from the graph. 
    def test05_delete_nodes(self):
        query = """MATCH (s:person)
                    WHERE s.name = "Boaz" OR s.name = "Alon"
                    DELETE s"""
        actual_result = redis_graph.query(query)        
        assert (actual_result.relationships_deleted == 0)
        assert (actual_result.nodes_deleted == 2)

    # Make sure Alon and Boaz are not in the graph.
    def test06_get_deleted_nodes(self):
        query = """MATCH (s:person)
                    WHERE s.name = "Boaz" OR s.name = "Alon"
                    RETURN s"""
        actual_result = redis_graph.query(query)
        assert(len(actual_result.result_set) is 1) # Only header row.

    # Make sure Alon and Boaz are the only removed nodes.
    def test07_verify_node_deletion(self):
        query = """MATCH (s:person)
                   RETURN COUNT(s)"""
        actual_result = redis_graph.query(query)
        nodeCount = int(float(actual_result.result_set[1][0]))
        assert(nodeCount == 5)

    def test08_delete_entire_graph(self):
        # Make sure graph exists.
        query = """MATCH (n) RETURN COUNT(n)"""
        result = redis_graph.query(query)
        nodeCount = int(float(result.result_set[1][0]))
        assert(nodeCount > 0)
        
        # Delete graph.
        redis_graph.delete()

        # Try to query a deleted graph should raise an exception.
        try:
            redis_graph.query(query)
            assert(False)
        except:
            pass

if __name__ == '__main__':
    unittest.main()
