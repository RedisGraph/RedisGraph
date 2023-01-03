from common import *
from index_utils import *

GRAPH_ID = "query"
graph = None

class testFulltextIndexQuery():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global graph
        redis_con = self.env.getConnection()
        graph = Graph(redis_con, GRAPH_ID)
        self.populate_graph()

    def populate_graph(self):
        graph.query("CALL db.idx.fulltext.createNodeIndex('L1', 'v')")
        graph.query("CALL db.idx.fulltext.createNodeIndex({label: 'L2', stopwords: ['redis', 'world'] }, 'v')")
        graph.query("CALL db.idx.fulltext.createNodeIndex('L3', { field: 'v1', weight: 1 }, { field: 'v2', weight: 2 })")
        graph.query("CALL db.idx.fulltext.createNodeIndex('L4', { field: 'v', phonetic: 'dm:en' })")
        graph.query("CALL db.idx.fulltext.createNodeIndex('L5', { field: 'v', nostem: true })")
        wait_for_indices_to_sync(graph)

        n = Node(label="L1", properties={"v": 'hello redis world'})
        graph.add_node(n)

        n = Node(label="L2", properties={"v": 'hello redis world'})
        graph.add_node(n)

        n = Node(label="L3", properties={"v1": 'hello world', "v2": 'hello redis'})
        graph.add_node(n)

        n = Node(label="L3", properties={"v1": 'hello redis', "v2": 'hello world'})
        graph.add_node(n)

        n = Node(label="L4", properties={"v": 'felix'})
        graph.add_node(n)

        n = Node(label="L5", properties={"v": 'there are seven words in this sentence'})
        graph.add_node(n)

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

        # fulltext query L3 for redis and document that contains redis in v2 is scored higher than document contains redis in v1
        result = graph.query("CALL db.idx.fulltext.queryNodes('L3', 'redis') YIELD node, score RETURN node, score ORDER BY score DESC")
        self.env.assertEquals(result.result_set[0][0].properties["v2"], "hello redis")
        self.env.assertEquals(result.result_set[1][0].properties["v1"], "hello redis")
        self.env.assertGreater(result.result_set[0][1], result.result_set[1][1])

        expected_result = graph.query("MATCH (n:L4 {v:'felix'}) RETURN n")

        # fulltext query L4 for phelix
        result = graph.query("CALL db.idx.fulltext.queryNodes('L4', 'phelix')")
        self.env.assertEquals(result.result_set[0][0], expected_result.result_set[0][0])

        expected_result = graph.query("MATCH (n:L5) RETURN n")

        # fulltext query L5 for 'words' which exists in the document
        result = graph.query("CALL db.idx.fulltext.queryNodes('L5', 'words')")
        self.env.assertEquals(result.result_set[0][0], expected_result.result_set[0][0])

        # fulltext query L5 for 'word' nostem did not removed 's' from 'words'
        # as such no results are expected
        result = graph.query("CALL db.idx.fulltext.queryNodes('L5', 'word')")
        self.env.assertEquals(result.result_set, [])

