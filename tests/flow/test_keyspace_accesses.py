import os
import sys
import redis
from RLTest import Env
from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../')
from demo import QueryInfo

graph = None
redis_con = None
GRAPH_ID = "G"
NEW_GRAPH_ID = "G2"

class testKeyspaceAccesses(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global graph
        global redis_con
        redis_con = self.env.getConnection()
        graph = Graph(GRAPH_ID, redis_con)
    
    def test00_test_data_valid_after_rename(self):
        global graph
        node0 = Node(node_id=0, label="L", properties={'name':'x', 'age':1})
        graph.add_node(node0)
        graph.flush()
        redis_con.rename(GRAPH_ID, NEW_GRAPH_ID)

        graph = Graph(NEW_GRAPH_ID, redis_con)

        node1 = Node(node_id=0, label="L", properties={'name':'x', 'age':1})
        graph.add_node(node1)
        graph.flush()

        query = "MATCH (n) return n"
        expected_results = [[node0], [node1]]
        query_info = QueryInfo(query = query, description="Tests data is valid after renaming", expected_result = expected_results)
        self._assert_resultset_and_expected_mutually_included(graph.query(query), query_info)

    # Graph queries should fail gracefully on accessing non-graph keys.
    def test01_graph_access_on_invalid_key(self):
        redis_con.set("integer_key", 5)
        graph = Graph("integer_key", redis_con)
        try:
            query = """MATCH (n) RETURN noneExistingFunc(n.age) AS cast"""
            graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("WRONGTYPE" in e.message)
            pass

    # Fail gracefully on attempting a graph deletion of an empty key.
    def test02_graph_delete_on_empty_key(self):
        graph = Graph("nonexistent_key", redis_con)
        try:
            graph.delete()
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("empty key" in e.message)
            pass
