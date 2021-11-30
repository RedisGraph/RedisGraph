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
        graph.query("CALL db.idx.fulltext.createNodeIndex({ label: 'L2', stopwords: ['redis', 'world'] }, 'v')")
        graph.query("CALL db.idx.fulltext.createNodeIndex('L3', { field: 'v1', weight: 1 }, { field: 'v2', weight: 1 })")
        graph.query("CALL db.idx.fulltext.createNodeIndex('L4', { field: 'v1', weight: 1 }, { field: 'v2', weight: 2 })")
        graph.query("CALL db.idx.fulltext.createNodeIndex('L5', { field: 'v', phonetic: 'dm:en' })")
        graph.query("CALL db.idx.fulltext.createNodeIndex('L6', { field: 'v', nostem: true })")

        a = Node(label="L1", properties={"v": 'hello redis world'})
        graph.add_node(a)

        b = Node(label="L2", properties={"v": 'hello redis world'})
        graph.add_node(b)

        b = Node(label="L3", properties={"v1": 'hello world', "v2": 'hello redis'})
        graph.add_node(b)
        b = Node(label="L4", properties={"v1": 'hello world', "v2": 'hello redis'})
        graph.add_node(b)

        b = Node(label="L5", properties={"v": 'felix'})
        graph.add_node(b)

        b = Node(label="L6", properties={"v": 'there are seven words in this sentence'})
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

        # fulltext query L1 for world 
        result = graph.query("CALL db.idx.fulltext.queryNodes('L1', 'world')")
        self.env.assertEquals(result.result_set[0][0], expected_result.result_set[0][0])

        expected_result = graph.query("MATCH (n:L2) RETURN n")

        # fulltext query L2 for hello 
        result = graph.query("CALL db.idx.fulltext.queryNodes('L2', 'hello')")
        self.env.assertEquals(result.result_set[0][0], expected_result.result_set[0][0])

        # fulltext query L2 for redis 
        result = graph.query("CALL db.idx.fulltext.queryNodes('L2', 'redis')")
        self.env.assertEquals(result.result_set, [])

        # fulltext query L2 for world 
        result = graph.query("CALL db.idx.fulltext.queryNodes('L2', 'world')")
        self.env.assertEquals(result.result_set, [])

        # # fulltext query L1 and L3 for redis and compate score
        # result1 = graph.query("CALL db.idx.fulltext.queryNodes('L3', 'redis')")
        # result2 = graph.query("CALL db.idx.fulltext.queryNodes('L4', 'redis')")
        # self.env.assertEquals(result1.result_set[0][1], result2.result_set[0][1])

        expected_result = graph.query("MATCH (n:L5) RETURN n")

        # fulltext query L5 for phelix
        result = graph.query("CALL db.idx.fulltext.queryNodes('L5', 'phelix')")
        self.env.assertEquals(result.result_set[0][0], expected_result.result_set[0][0])

        expected_result = graph.query("MATCH (n:L6) RETURN n")

        # fulltext query L6 for words
        result = graph.query("CALL db.idx.fulltext.queryNodes('L6', 'words')")
        self.env.assertEquals(result.result_set[0][0], expected_result.result_set[0][0])

        # fulltext query L5 for word
        result = graph.query("CALL db.idx.fulltext.queryNodes('L6', 'word')")
        self.env.assertEquals(result.result_set, [])
