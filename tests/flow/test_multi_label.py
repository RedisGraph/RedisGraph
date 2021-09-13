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
        q = "CREATE (v1:L0:L1)-[:E]->(v2:L1)-[:E]->(v3:L1:L2)"
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
        labels = ['L' + str(x) for x in range(0, 18)]
        query = "CREATE (n :" + ':'.join(labels) + ") RETURN LABELS(n)"
        query_result = redis_graph.query(query)
        expected_result = [[labels]]
        self.env.assertEquals(query_result.result_set, expected_result)


    # TODO:
    # index scan - make sure index is utilized when node has multiple labels
    #              labels not covered by the index should be enforced!
    # 
    # merge - pattern creation where a multi-label node is created/matched
    # 
    # optional match - multi-label node takes part in an optional match
    #
    # do we support adding / removing labels from an existing node?
    #
    # varaible length traversal - multi label node at the begining/end of a
    #                             variable length pattern
