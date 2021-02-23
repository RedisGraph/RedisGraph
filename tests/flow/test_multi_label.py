import redis
from RLTest import Env
from base import FlowTestsBase
from redisgraph import Graph, Node, Edge

redis_graph = None
GRAPH_ID = "multi_label_test"

class testMultiLabel(FlowTestsBase):
    def __init__(self):
        global redis_graph
        self.env = Env(decodeResponses=True)
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)
        self.populate_graph()

    def populate_graph(self):
        # Construct a graph with the form:
        # (v1:L0:L1)-[:E]->(v2:L1)-[:E]->(v3:L1:L2)
        q = """CREATE (:L0:L1 {val:1})-[:E]->(:L1 {val:2})-[:E]->(:L1:L2 {val:3})"""
        redis_graph.query(q)

    # Validate basic multi-label scans.
    def test01_multilabel_scan(self):
        # Issue a query that matches the single (:L0:L1) node.
        query = """MATCH (a:L0:L1) RETURN a.val"""
        query_result = redis_graph.query(query)
        expected_result = [[1]]
        self.env.assertEquals(query_result.result_set, expected_result)

        # Issue a query that matches all 3 nodes with the label :L1.
        query = """MATCH (a:L1) RETURN a.val ORDER BY a.val"""
        query_result = redis_graph.query(query)
        expected_result = [[1],
                           [2],
                           [3]]
        self.env.assertEquals(query_result.result_set, expected_result)

        # Issue a query that matches no nodes, as the (:L0:L2) label disjunction is not present.
        query = """MATCH (a:L0:L2) RETURN a.val"""
        query_result = redis_graph.query(query)
        expected_result = []
        self.env.assertEquals(query_result.result_set, expected_result)

    # Validate basic multi-label scans.
    def test02_multilabel_traversal(self):
        expected_result = [[1, 2]]
        query = """MATCH (a:L0:L1)-[]->(b) RETURN a.val, b.val"""
        query_result = redis_graph.query(query)
        self.env.assertEquals(query_result.result_set, expected_result)

        query = """MATCH (a:L0:L1)-[:E]->(b) RETURN a.val, b.val"""
        query_result = redis_graph.query(query)
        self.env.assertEquals(query_result.result_set, expected_result)

        query = """MATCH (a:L0:L1)-[:E]->(b:L1) RETURN a.val, b.val"""
        query_result = redis_graph.query(query)
        self.env.assertEquals(query_result.result_set, expected_result)

        query = """MATCH (b:L1) WITH (b) MATCH (a:L0:L1)-[:E]->(b) RETURN a.val, b.val"""
        query_result = redis_graph.query(query)
        self.env.assertEquals(query_result.result_set, expected_result)

        query = """MATCH (b:L1) WITH (b) MATCH (a:L0)-[:E]->(b) RETURN a.val, b.val"""
        query_result = redis_graph.query(query)
        self.env.assertEquals(query_result.result_set, expected_result)
