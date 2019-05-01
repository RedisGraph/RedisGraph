import os
import sys
import string
import random
import unittest
import redis
from redisgraph import Graph, Node, Edge
from .base import FlowTestsBase

dis_redis = None
redis_graph = None
redis_con = None
node_names = ["A", "B", "C", "D"]

# A can reach 3 nodes, B can reach 2 nodes, C can reach 1 node
max_results = 6

def random_string(size=6, chars=string.ascii_letters):
    return ''.join(random.choice(chars) for _ in range(size))

def get_redis():
    global dis_redis
    conn = redis.Redis()
    try:
        conn.ping()
        # Assuming RedisGraph is loaded.
    except redis.exceptions.ConnectionError:
        from .redis_base import DisposableRedis
        # Bring up our own redis-server instance.
        dis_redis = DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')
        dis_redis.start()
        conn = dis_redis.client()
    return conn

class VariableLengthTraversalsFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "VariableLengthTraversalsFlowTest"
        global redis_graph
        global redis_con
        redis_con = get_redis()
        GRAPH_ID = random_string()
        #  GRAPH_ID = "test"
        redis_graph = Graph(GRAPH_ID, redis_con)
        cls.populate_graph()

    @classmethod
    def tearDownClass(cls):
        if dis_redis is not None:
            dis_redis.stop()

    @classmethod
    def populate_graph(cls):
        global redis_graph

        nodes = []
         # Create nodes
        for n in node_names:
            node = Node(label="node", properties={"name": n})
            redis_graph.add_node(node)
            nodes.append(node)

        # Create edges
        for i in range(len(nodes) - 1):
            edge = Edge(nodes[i], "knows", nodes[i+1], properties={"connects": node_names[i] + node_names[i+1]})
            redis_graph.add_edge(edge)

        redis_graph.commit()

    # Sanity check against single-hop traversal
    def test01_conditional_traverse(self):
        query = """MATCH (a)-[e]->(b) RETURN a, e, b ORDER BY a.name, b.name"""
        actual_result = redis_graph.query(query)
        expected_result = [['a.name', 'e.connects', 'b.name'],
                           ['A', 'AB', 'B'],
                           ['B', 'BC', 'C'],
                           ['C', 'CD', 'D']]
        assert(actual_result.result_set == expected_result)

    # Traversal with no labels
    def test02_unlabeled_traverse(self):
        query = """MATCH (a)-[*]->(b) RETURN a, b ORDER BY a.name, b.name"""
        actual_result = redis_graph.query(query)
        assert(len(actual_result.result_set[1:]) == max_results)

        query = """MATCH (a)<-[*]-(b) RETURN a, b ORDER BY a.name, b.name"""
        actual_result = redis_graph.query(query)
        assert(len(actual_result.result_set[1:]) == max_results)

    # Traversal with labeled source
    def test03_source_labeled(self):
        query = """MATCH (a:node)-[*]->(b) RETURN a, b ORDER BY a.name, b.name"""
        actual_result = redis_graph.query(query)
        assert(len(actual_result.result_set[1:]) == max_results)

        query = """MATCH (a:node)<-[*]-(b) RETURN a, b ORDER BY a.name, b.name"""
        actual_result = redis_graph.query(query)
        assert(len(actual_result.result_set[1:]) == max_results)

    # Traversal with labeled dest
    def test04_dest_labeled(self):
        query = """MATCH (a)-[*]->(b:node) RETURN a, b ORDER BY a.name, b.name"""
        actual_result = redis_graph.query(query)
        assert(len(actual_result.result_set[1:]) == max_results)

        query = """MATCH (a)<-[*]-(b:node) RETURN a, b ORDER BY a.name, b.name"""
        actual_result = redis_graph.query(query)
        assert(len(actual_result.result_set[1:]) == max_results)
