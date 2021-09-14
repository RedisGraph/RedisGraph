import redis
from RLTest import Env
from base import FlowTestsBase
from redisgraph import Graph, Node, Edge

redis_graph = None
GRAPH_ID = "multi_label"

class testMultiLabel():
    def __init__(self):
        global redis_graph
        self.env = Env(decodeResponses=True)
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)
        self.populate_graph()

    def populate_graph(self):
        # Construct a graph with the form:
        # (v1:L0:L1)-[:E]->(v2:L1)-[:E]->(v3:L1:L2)
        q = "CREATE (v1:L0:L1 {v: 1})-[:E]->(v2:L1 {v: 2})-[:E]->(v3:L1:L2 {v: 3})"
        redis_graph.query(q)

    # Validate basic multi-label scans.
    def test01_multilabel_scan(self):
        # Issue a query that matches the single (:L0) node.
        query = "MATCH (a:L0) RETURN LABELS(a)"
        query_result = redis_graph.query(query)
        expected_result = [[['L0','L1']]]
        self.env.assertEquals(query_result.result_set, expected_result)

        # Issue a query that matches the single (:L0:L1) node.
        query = "MATCH (a:L0:L1) RETURN LABELS(a)"
        query_result = redis_graph.query(query)
        expected_result = [[['L0','L1']]]
        self.env.assertEquals(query_result.result_set, expected_result)

        # Issue a query that matches the single (:L1:L0) node.
        query = "MATCH (a:L1:L0) RETURN LABELS(a)"
        query_result = redis_graph.query(query)
        expected_result = [[['L0','L1']]]
        self.env.assertEquals(query_result.result_set, expected_result)

        # Issue a query that matches all 3 nodes with the label :L1.
        query = "MATCH (a:L1) RETURN LABELS(a) ORDER BY LABELS(a)"
        query_result = redis_graph.query(query)
        expected_result = [[['L0','L1']], [['L1']], [['L1','L2']]]
        self.env.assertEquals(query_result.result_set, expected_result)

        # Issue a query that matches no nodes, as the (:L0:L2) label disjunction is not present.
        query = "MATCH (a:L0:L2) RETURN LABELS(a)"
        query_result = redis_graph.query(query)
        expected_result = []
        self.env.assertEquals(query_result.result_set, expected_result)

    # Validate basic multi-label traversals.
    def test02_multilabel_traversal(self):
        # (v1:L0:L1)-[:E]->(v2:L1)-[:E]->(v3:L1:L2)
        queries = [
            "MATCH (a:L0:L1)-[]->(b) RETURN LABELS(a), LABELS(b)",
            "MATCH (a:L0:L1)-[:E]->(b) RETURN LABELS(a), LABELS(b)",
            "MATCH (a:L0:L1)-[:E]->(b:L1) RETURN LABELS(a), LABELS(b)",
            "MATCH (b:L1) WITH (b) MATCH (a:L0:L1)-[:E]->(b) RETURN LABELS(a), LABELS(b)",
            "MATCH (b:L1) WITH (b) MATCH (a:L0)-[:E]->(b) RETURN LABELS(a), LABELS(b)",
        ]

        expected_result = [[['L0','L1'], ['L1']]]

        for q in queries:
            query_result = redis_graph.query(q)
            self.env.assertEquals(query_result.result_set, expected_result)

    # Validate that the graph properly handles label counts greater than its default.
    def test03_large_label_count(self):
        # Introduce a node with enough labels to force graph resizes.
        labels = ['L' + str(x) for x in range(10, 28)]
        query = "CREATE (n :" + ':'.join(labels) + ") RETURN LABELS(n)"
        query_result = redis_graph.query(query)
        expected_result = [[labels]]
        self.env.assertEquals(query_result.result_set, expected_result)

    # Validate behavior of index scans on multi-labeled nodes
    def test04_index_scan(self):
        query = """CREATE INDEX ON :L1(v)"""
        query_result = redis_graph.query(query)
        self.env.assertEquals(query_result.indices_created, 1)

        # Query the explicitly created index
        query = """MATCH (a:L1) WHERE a.v > 0 RETURN a.v ORDER BY a.v"""
        plan = redis_graph.execution_plan(query)
        self.env.assertContains("Index Scan", plan)
        query_result = redis_graph.query(query)
        expected_result = [[1],
                           [2],
                           [3]]
        self.env.assertEquals(query_result.result_set, expected_result)

        # Query the explicitly created index on a multi-labeled node
        queries = [
            "MATCH (a:L1:L2) WHERE a.v > 0 RETURN a.v ORDER BY a.v",
            "MATCH (a:L2:L1) WHERE a.v > 0 RETURN a.v ORDER BY a.v"
        ]

        for q in queries:
            plan = redis_graph.execution_plan(q)
            self.env.assertContains("Index Scan", plan)
            query_result = redis_graph.query(q)
            expected_result = [[3]]
            self.env.assertEquals(query_result.result_set, expected_result)

    # Validate the creation of multi-labeled nodes with the MERGE clause
    def test05_multi_label_merge(self):
        query = """MERGE (a:L2:L3 {v: 4}) RETURN labels(a)"""
        query_result = redis_graph.query(query)
        expected_result = [[["L2", "L3"]]]
        self.env.assertEquals(query_result.nodes_created, 1)
        self.env.assertEquals(query_result.labels_added, 1)
        self.env.assertEquals(query_result.result_set, expected_result)

        # Repetition of the query should not create a new node
        query_result = redis_graph.query(query)
        self.env.assertEquals(query_result.nodes_created, 0)
        self.env.assertEquals(query_result.labels_added, 0)
        self.env.assertEquals(query_result.result_set, expected_result)

    # Validate that OPTIONAL MATCH enforces multi-label constraints
    def test06_multi_label_optional_match(self):
        # Traverse to a multi-label destination in an OPTIONAL MATCH
        query = """MATCH (a:L1) OPTIONAL MATCH (a)-[]->(b:L2:L1) RETURN labels(a) AS la, labels(b) AS lb ORDER BY la, lb"""
        query_result = redis_graph.query(query)
        expected_result = [[["L0", "L1"], None],
                           [["L1"], ["L1", "L2"]],
                           [["L1", "L2"], None]]
        self.env.assertEquals(query_result.result_set, expected_result)

        # Specify an additional label for the source node in the OPTIONAL MATCH
        query = """MATCH (a:L0) OPTIONAL MATCH (a:L1)-[]->(b:L1) RETURN labels(a) AS la, labels(b) AS lb ORDER BY la, lb"""
        query_result = redis_graph.query(query)
        expected_result = [[["L0", "L1"], ["L1"]]]
        self.env.assertEquals(query_result.result_set, expected_result)

    # Validate multi-labeled sources and destinations in variable-length traversals
    def test07_multi_label_variable_length_traversal(self):
        query = """MATCH (a {v: 1})-[*]->(b:L1:L2 {v: 3}) RETURN labels(a), labels(b)"""
        query_result = redis_graph.query(query)
        expected_result = [[["L0", "L1"], ["L1", "L2"]]]
        self.env.assertEquals(query_result.result_set, expected_result)

        query = """MATCH (a:L0 {v: 1})-[*]->(b:L1:L2 {v: 3}) RETURN labels(a), labels(b)"""
        query_result = redis_graph.query(query)
        self.env.assertEquals(query_result.result_set, expected_result)

        query = """MATCH (a:L0 {v: 1})-[*]->(b:L1:L2 {v: 3}) RETURN labels(a), labels(b)"""
        query_result = redis_graph.query(query)
        self.env.assertEquals(query_result.result_set, expected_result)

        query = """MATCH (a:L0:L1 {v: 1})-[*]->(b:L2 {v: 3}) RETURN labels(a), labels(b)"""
        query_result = redis_graph.query(query)
        self.env.assertEquals(query_result.result_set, expected_result)

        query = """MATCH (a:L0:L1 {v: 1})-[*]->(b {v: 3}) RETURN labels(a), labels(b)"""
        query_result = redis_graph.query(query)
        self.env.assertEquals(query_result.result_set, expected_result)

        query = """MATCH (a:L0)-[*]->(b:L1:L2 {v: 3}) RETURN labels(a), labels(b)"""
        query_result = redis_graph.query(query)
        self.env.assertEquals(query_result.result_set, expected_result)

        query = """MATCH (a:L0:L1)-[*]->(b:L2 {v: 3}) RETURN labels(a), labels(b)"""
        query_result = redis_graph.query(query)
        self.env.assertEquals(query_result.result_set, expected_result)

        query = """MATCH (a:L0:L1)-[*]->(b {v: 3}) RETURN labels(a), labels(b)"""
        query_result = redis_graph.query(query)
        self.env.assertEquals(query_result.result_set, expected_result)
