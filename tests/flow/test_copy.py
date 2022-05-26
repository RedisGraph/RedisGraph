import random
from common import *

redis_con = None
port = None

class testGraphCopy():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        self.redis_con = self.env.getConnection()

    # assert graph A and B are the same
    def assert_graphs_eq(self, A, B):
        A = Graph(A, self.redis_con)
        B = Graph(B, self.redis_con)

        #-----------------------------------------------------------------------
        # validations queries
        #-----------------------------------------------------------------------

        queries = [
                "MATCH (n) RETURN count(n)",        # number of nodes
                "MATCH ()-[e]->() RETURN count(e)", # number of edges
                "CALL db.indexes()",                # indices
                "CALL db.labels()",                 # labels
                "CALL db.relationshipTypes()",      # relationship types
                "MATCH (n) RETURN n",               # validate nodes
                "MATCH ()-[e]->() RETURN e"         # validate edges
                ]

        for q in queries:
            A_res = A.query(q)
            B_res = B.query(q)
            self.env.assertEquals(A_res.result_set, B_res.result_set)

    def test00_copy_empty_graph(self):
        g = Graph("empty", self.redis_con)

        # create an empty graph
        g.query("RETURN 1")

        # copy graph
        self.env.execute_command("COPY", "empty", "clone")

        self.assert_graphs_eq(self, "empty", "clone")

    def test01_empty_graph_with_indices(self):
        pass

    def test02_single_node_graph(self):
        pass

    def test03_single_edge_graph(self):
        pass

    def test04_copy_empty_graph(self):
        pass
        # populate the source graph
        self.populate_graph("FROM")
        # copy the source graph to the key "TO"
        self.env.execute_command("COPY FROM TO")
        # connect to the new graph
        graph = Graph("TO", redis_con)

        # expecting 5 person entities
        query = """MATCH (p:person) RETURN COUNT(p)"""
        actual_result = graph.query(query)
        nodeCount = actual_result.result_set[0][0]
        self.env.assertEquals(nodeCount, 5)

        query = """MATCH (p:person) WHERE p.name='Alon' RETURN COUNT(p)"""
        actual_result = graph.query(query)
        nodeCount = actual_result.result_set[0][0]
        self.env.assertEquals(nodeCount, 1)

        # expecting 3 country entities
        query = """MATCH (c:country) RETURN COUNT(c)"""
        actual_result = graph.query(query)
        nodeCount = actual_result.result_set[0][0]
        self.env.assertEquals(nodeCount, 3)

        query = """MATCH (c:country) WHERE c.name = 'Israel' RETURN COUNT(c)"""
        actual_result = graph.query(query)
        nodeCount = actual_result.result_set[0][0]
        self.env.assertEquals(nodeCount, 1)

        # expecting 2 visit edges
        query = """MATCH (n:person)-[e:visit]->(c:country) WHERE e.purpose='pleasure' RETURN COUNT(e)"""
        actual_result = graph.query(query)
        edgeCount = actual_result.result_set[0][0]
        self.env.assertEquals(edgeCount, 2)

        # verify indexes exist
        indices = graph.query("""CALL db.indexes()""").result_set
        expected_indices = [
                ['exact-match', 'country', ['name', 'population'], 'english', [], 'NODE'],
                ['exact-match', 'person', ['name', 'height'], 'english', [], 'NODE'],
                ['full-text', 'person', ['text'], 'english', ['a', 'b'], 'NODE'],
                ['exact-match', 'visit', ['_src_id', '_dest_id', 'purpose'], 'english', [], 'RELATIONSHIP']
        ]

        self.env.assertEquals(len(indices), len(expected_indices))
        for index in indices:
            self.env.assertIn(index, indices)

