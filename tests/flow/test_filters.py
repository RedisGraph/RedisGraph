from RLTest import Env
from redisgraph import Graph
from redis import ResponseError

global g

class testFilters():
    def __init__(self):
        self.env = Env(decodeResponses=True)

    def test01_filter_with_different_predicates(self):
        g = Graph("LogicalOperators", self.env.getConnection())
        # Create 5 nodes with contiguous integer values and alternating true/false values
        g.query("UNWIND range(1, 5) AS x CREATE (:N { v: x, b: x % 2 = 0 })")

        # test and operation
        expected = [[i, j] for i in range(1, 6) for j in range(1, 6) if i % 2 == 0 and j % 2 == 0]
        result = g.query("MATCH (n:N), (m:N) WHERE n.b AND m.b RETURN n.v, m.v ORDER BY n.v, m.v")
        self.env.assertEqual(result.result_set,  expected)

        # test or operation
        expected = [[i, j] for i in range(1, 6) for j in range(1, 6) if i % 2 == 0 or j % 2 == 0]
        result = g.query("MATCH (n:N), (m:N) WHERE n.b OR m.b RETURN n.v, m.v ORDER BY n.v, m.v")
        self.env.assertEqual(result.result_set,  expected)

        # test xor operation
        expected = [[i, j] for i in range(1, 6) for j in range(1, 6) if i % 2 != j % 2]
        result = g.query("MATCH (n:N), (m:N) WHERE n.b XOR m.b RETURN n.v, m.v ORDER BY n.v, m.v")
        self.env.assertEqual(result.result_set,  expected)

        # test negation of and operation
        expected = [[i, j] for i in range(1, 6) for j in range(1, 6) if not (i % 2 == 0 and j % 2 == 0)]
        result = g.query("MATCH (n:N), (m:N) WHERE NOT (n.b AND m.b) RETURN n.v, m.v ORDER BY n.v, m.v")
        self.env.assertEqual(result.result_set,  expected)

        # test negation of or operation
        expected = [[i, j] for i in range(1, 6) for j in range(1, 6) if not (i % 2 == 0 or j % 2 == 0)]
        result = g.query("MATCH (n:N), (m:N) WHERE NOT (n.b OR m.b) RETURN n.v, m.v ORDER BY n.v, m.v")
        self.env.assertEqual(result.result_set,  expected)

        # test negation of xor operation
        expected = [[i, j] for i in range(1, 6) for j in range(1, 6) if not (i % 2 != j % 2)]
        result = g.query("MATCH (n:N), (m:N) WHERE NOT (n.b XOR m.b) RETURN n.v, m.v ORDER BY n.v, m.v")
        self.env.assertEqual(result.result_set,  expected)

    def test02_filter_inlined_predicates(self):
        g = Graph("InlinedPredicates", self.env.getConnection())
        # Create 3 nodes with 'v' and 'name' properties connected by 2 edges with 'e' properties
        g.query("CREATE (:L {v: 1, name: 'Jeffrey'})-[:R {e: 3}]->(:L {v: 10, name: 'Roi'})-[:R {e: 6}]->(:L {v: 50, name: 'Avi'})")

        expected = [[1, 10]]
        # test single node predicate
        result = g.query("MATCH (n:L WHERE n.v = 1)-[]->(m) RETURN n.v, m.v")
        self.env.assertEqual(result.result_set, expected)

        result = g.query("MATCH (n WHERE n.v < 5)-[:R]->(m) RETURN n.v, m.v")
        self.env.assertEqual(result.result_set, expected)

        # test function call in inlined predicate
        result = g.query("MATCH (n:L WHERE left(n.name, 4) = 'Jeff')-[]->(m) RETURN n.v, m.v")
        self.env.assertEqual(result.result_set, expected)

        # test multiple predicates
        result = g.query("MATCH (n:L WHERE n.v < 100)-[r:R WHERE r.e < 5]->(m:L) RETURN n.v, m.v")
        self.env.assertEqual(result.result_set, expected)

        # test predicates referencing other pattern elements (NOTE - this is not supported by Neo4j)
        result = g.query("MATCH (n:L WHERE n.v = m.v / 10)-[]->(m) RETURN n.v, m.v")
        self.env.assertEqual(result.result_set, expected)

        expected = [[1, 10],
                    [10, 50]]
        # test predicate with multiple results
        result = g.query("MATCH (n:L)-[r:R WHERE r.e < 10]->(m:L) RETURN n.v, m.v ORDER BY n.v")
        self.env.assertEqual(result.result_set, expected)

        # test predicate with multiple conditions
        result = g.query("MATCH (n:L)-[r:R WHERE r.e = 3 OR r.e = 6]->(m:L) RETURN n.v, m.v ORDER BY n.v")
        self.env.assertEqual(result.result_set, expected)

        expected = [[1, []],
                    [10, ['Avi']],
                    [50, []]]
        # test filter in pattern comprehension
        result = g.query("MATCH (n:L) RETURN n.v, [(n)-[]->(m WHERE m.v > 25) | m.name] ORDER BY n.v")
        self.env.assertEqual(result.result_set, expected)

    def test03_invalid_inlined_predicates(self):
        g = Graph("InlinedPredicates", self.env.getConnection())
        try:
            g.query("MATCH (n:L WHERE sum(n.v) = 1) RETURN n.v")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Invalid use of aggregating function", str(e))

        try:
            g.query("CREATE (n:L WHERE n.v = 1) RETURN n.v")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Node pattern predicates are not allowed in CREATE", str(e))

        try:
            g.query("CREATE ()-[r:R WHERE r.e = 1]->()")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Edge pattern predicates are not allowed in CREATE", str(e))

        try:
            g.query("MERGE(n:L WHERE n.v = 1) RETURN n.v")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Node pattern predicates are not allowed in MERGE", str(e))

        try:
            g.query("MERGE ()-[r:R WHERE r.e = 1]->()")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Edge pattern predicates are not allowed in MERGE", str(e))
