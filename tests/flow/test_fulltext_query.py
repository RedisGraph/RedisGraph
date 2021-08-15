import os
import sys
from RLTest import Env
from redisgraph import Graph, Node

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

GRAPH_ID = "query"
graph = None

class testFulltextIndexQuery(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global graph
        redis_con = self.env.getConnection()
        graph = Graph(GRAPH_ID, redis_con)
        self.populate_graph()

    def populate_graph(self):
        graph.query("CALL db.idx.fulltext.createNodeIndex('L1', 'v')")
        graph.query("CALL db.idx.fulltext.createNodeIndex({ label: 'L2', stopwords: ['redis'] }, 'v')")

        a = Node(label="L1", properties={"v": 'hello redis world'})
        graph.add_node(a)

        b = Node(label="L2", properties={"v": 'hello redis world'})
        graph.add_node(b)

        graph.flush()

    # full-text query
    def test01_fulltext_query(self):
        expected_result = graph.query("MATCH (n:L1) RETURN n")
        # fulltext query L1 for hello 
        result = graph.query("CALL db.idx.fulltext.queryNodes('L1', 'hello')")
        self.env.assertEquals(result.result_set[0][0], expected_result.result_set[0][0])

        # fulltext query L1 for redis 
        result = graph.query("CALL db.idx.fulltext.queryNodes('L1', 'redis')")
        self.env.assertEquals(result.result_set[0][0], expected_result.result_set[0][0])

        expected_result = graph.query("MATCH (n:L2) RETURN n")

        # fulltext query L2 for hello 
        result = graph.query("CALL db.idx.fulltext.queryNodes('L2', 'hello')")
        self.env.assertEquals(result.result_set[0][0], expected_result.result_set[0][0])

        # fulltext query L2 for redis 
        result = graph.query("CALL db.idx.fulltext.queryNodes('L2', 'redis')")
        self.env.assertEquals(result.result_set, [])