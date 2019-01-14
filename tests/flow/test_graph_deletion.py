import os
import sys
import redis
import string
import random
import unittest
from base import FlowTestsBase
from redisgraph import Graph, Node, Edge

redis_graph = None
redis_con = None
dis_redis = None

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

class GraphDeletionFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "GraphDeletionFlowTest"
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

    # Count how many nodes contains the `name` attribute
    # remove the `name` attribute from some nodes
    # make sure the count updates accordingly,
    # restore `name` attribute from, verify that count returns to its original value.
    def test01_delete_attribute(self):
        # How many nodes contains the 'name' attribute
        query = """MATCH (n) WHERE EXISTS(n.name)=true RETURN count(n)"""
        actual_result = redis_graph.query(query)
        nodeCount = int(actual_result.result_set[1][0])
        assert(nodeCount == 7)

        # Remove Tal's name attribute.
        query = """MATCH (n) WHERE n.name = 'Tal' SET n.name = NULL"""
        redis_graph.query(query)

        # How many nodes contains the 'name' attribute,
        # should reduce by 1 from previous count.
        query = """MATCH (n) WHERE EXISTS(n.name)=true RETURN count(n)"""
        actual_result = redis_graph.query(query)
        nodeCount = int(actual_result.result_set[1][0])
        assert(nodeCount == 6)

        # Reintroduce Tal's name attribute.
        query = """MATCH (n) WHERE EXISTS(n.name)=false SET n.name = 'Tal'"""
        actual_result = redis_graph.query(query)

        # How many nodes contains the 'name' attribute
        query = """MATCH (n) WHERE EXISTS(n.name)=true RETURN count(n)"""
        actual_result = redis_graph.query(query)
        nodeCount = int(actual_result.result_set[1][0])
        assert(nodeCount == 7)

    # Delete edges pointing into either Boaz or Ori.
    def test02_delete_edges(self):
        query = """MATCH (s:person)-[e:know]->(d:person) WHERE d.name = "Boaz" OR d.name = "Ori" DELETE e"""
        actual_result = redis_graph.query(query)
        assert (actual_result.relationships_deleted == 12)
        assert (actual_result.nodes_deleted == 0)

    # Make sure there are no edges going into either Boaz or Ori.
    def test03_verify_edge_deletion(self):
        query = """MATCH (s:person)-[e:know]->(d:person)                    
                    WHERE d.name = "Boaz" AND d.name = "Ori"
                    RETURN COUNT(s)"""
        actual_result = redis_graph.query(query)
        assert(len(actual_result.result_set) is 1)

    # Remove 'know' edge connecting Roi to Alon
    # Leaving a single edge of type SameBirthday
    # connecting the two.
    def test04_delete_typed_edge(self):
        query = """MATCH (s:person {name: "Roi"})-[e:know]->(d:person {name: "Alon"})
                   DELETE e"""
        actual_result = redis_graph.query(query)
        assert (actual_result.relationships_deleted == 1)
        assert (actual_result.nodes_deleted == 0)

    # Make sure Roi is still connected to Alon
    # via the "SameBirthday" type edge.
    def test05_verify_delete_typed_edge(self):
        query = """MATCH (s:person {name: "Roi"})-[e:SameBirthday]->(d:person {name: "Alon"})
                   RETURN COUNT(s)"""
        actual_result = redis_graph.query(query)
        assert(len(actual_result.result_set) is 2) # header row + record.

        query = """MATCH (s:person {name: "Roi"})-[e:know]->(d:person {name: "Alon"})
                   RETURN COUNT(s)"""
        actual_result = redis_graph.query(query)
        assert(len(actual_result.result_set) is 1) # Only header row.

    # Remove both Alon and Boaz from the graph. 
    def test06_delete_nodes(self):
        query = """MATCH (s:person)
                    WHERE s.name = "Boaz" OR s.name = "Alon"
                    DELETE s"""
        actual_result = redis_graph.query(query)        
        assert (actual_result.relationships_deleted == 0)
        assert (actual_result.nodes_deleted == 2)

    # Make sure Alon and Boaz are not in the graph.
    def test07_get_deleted_nodes(self):
        query = """MATCH (s:person)
                    WHERE s.name = "Boaz" OR s.name = "Alon"
                    RETURN s"""
        actual_result = redis_graph.query(query)
        assert(len(actual_result.result_set) is 1) # Only header row.

    # Make sure Alon and Boaz are the only removed nodes.
    def test08_verify_node_deletion(self):
        query = """MATCH (s:person)
                   RETURN COUNT(s)"""
        actual_result = redis_graph.query(query)
        nodeCount = int(actual_result.result_set[1][0])
        assert(nodeCount == 5)

    def test09_delete_entire_graph(self):
        # Make sure graph exists.
        query = """MATCH (n) RETURN COUNT(n)"""
        result = redis_graph.query(query)
        nodeCount = int(result.result_set[1][0])
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
