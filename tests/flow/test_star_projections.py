from common import *

GRAPH_ID = "starProjection"
redis_graph = None


class testStarProjections():

    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(redis_con, GRAPH_ID)

    # verify that star projections in RETURN clauses perform as
    # expected with all clause modifiers
    def test01_return_star(self):
        query = """UNWIND range(1, 2) AS x UNWIND range(3, 4) AS y RETURN *"""
        actual_result = redis_graph.query(query)
        expected = [[1, 3],
                    [1, 4],
                    [2, 3],
                    [2, 4]]
        self.env.assertEqual(actual_result.result_set, expected)

        query = """UNWIND range(1, 2) AS x UNWIND range(3, 4) AS y RETURN * SKIP 1 LIMIT 2"""
        actual_result = redis_graph.query(query)
        expected = [[1, 4],
                    [2, 3]]
        self.env.assertEqual(actual_result.result_set, expected)

        query = """UNWIND range(5, 0, -1) AS x RETURN * ORDER BY x SKIP 2 LIMIT 3"""
        actual_result = redis_graph.query(query)
        expected = [[2],
                    [3],
                    [4]]
        self.env.assertEqual(actual_result.result_set, expected)

        # RETURN * should produce an error when no variables are bound
        try:
            query = """MATCH () RETURN *"""
            actual_result = redis_graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertContains("RETURN * is not allowed when there are no variables in scope", str(e))

        try:
            query = """CALL db.indexes() RETURN *"""
            actual_result = redis_graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertContains("RETURN * is not allowed when there are no variables in scope", str(e))

    # verify that star projections combined with explicit aliases function properly
    def test02_return_star_and_projections(self):
        # duplicate column names should not result in multiple columns
        query = """UNWIND range(1, 3) AS x RETURN *, x"""
        actual_result = redis_graph.query(query)
        expected = [[1],
                    [2],
                    [3]]
        self.env.assertEqual(actual_result.result_set, expected)

        # aliased columns should return new columns
        query = """UNWIND range(1, 3) AS x RETURN *, x AS l"""
        actual_result = redis_graph.query(query)
        expected = [[1, 1],
                    [2, 2],
                    [3, 3]]
        self.env.assertEqual(actual_result.result_set, expected)

        # expressions should return new columns
        query = """UNWIND range(1, 3) AS x RETURN *, x + 1"""
        actual_result = redis_graph.query(query)
        expected = [[1, 2],
                    [2, 3],
                    [3, 4]]
        self.env.assertEqual(actual_result.result_set, expected)

        # call expressions in RETURN *
        redis_graph.query("CREATE INDEX FOR (n:L) ON (n.v)")
        query = "CALL db.indexes() YIELD label RETURN *"
        actual_result = redis_graph.query(query)
        expected = [['L']]
        self.env.assertEqual(actual_result.result_set, expected)

        # RETURN *, * should produce an error
        try:
            query = """UNWIND range(1, 3) AS x RETURN *, *"""
            actual_result = redis_graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError:
            pass

    # verify that star projections in WITH clauses perform as
    # expected with all clause modifiers
    def test03_with_star(self):
        query = """UNWIND range(1, 2) AS x UNWIND range(3, 4) AS y WITH * RETURN *"""
        actual_result = redis_graph.query(query)
        expected = [[1, 3],
                    [1, 4],
                    [2, 3],
                    [2, 4]]
        self.env.assertEqual(actual_result.result_set, expected)

        # test a sequence of WITH * projections
        query = """UNWIND range(1, 2) AS x UNWIND range(3, 4) AS y WITH * WITH * WITH * RETURN *"""
        actual_result = redis_graph.query(query)
        self.env.assertEqual(actual_result.result_set, expected)

        # test an explicit variable in the RETURN clause
        # after a sequence of WITH * projections
        query = """UNWIND range(1, 2) AS x UNWIND range(3, 4) AS y WITH * WITH * WITH * RETURN x, y"""
        actual_result = redis_graph.query(query)
        self.env.assertEqual(actual_result.result_set, expected)

        query = """UNWIND range(1, 2) AS x UNWIND range(3, 4) AS y WITH * SKIP 1 LIMIT 2 RETURN *"""
        actual_result = redis_graph.query(query)
        expected = [[1, 4],
                    [2, 3]]
        self.env.assertEqual(actual_result.result_set, expected)

        query = """UNWIND range(1, 2) AS x UNWIND range(3, 4) AS y WITH * ORDER BY x SKIP 1 LIMIT 2 WHERE x > 1 RETURN *"""
        actual_result = redis_graph.query(query)
        expected = [[2, 4]]
        self.env.assertEqual(actual_result.result_set, expected)

        query = """UNWIND range(5, 0, -1) AS x WITH x ORDER BY x SKIP 2 LIMIT 3 RETURN *"""
        actual_result = redis_graph.query(query)
        expected = [[2],
                    [3],
                    [4]]
        self.env.assertEqual(actual_result.result_set, expected)

        # test a WITH clause with a WHERE condition
        query = """UNWIND range(1, 2) AS x WITH * WHERE false RETURN *"""
        actual_result = redis_graph.query(query)
        expected = []
        self.env.assertEqual(actual_result.result_set, expected)

        # test a WITH * projection that also introduces a new variable
        query = """UNWIND range(1, 2) AS x WITH *, 3 AS y RETURN *"""
        actual_result = redis_graph.query(query)
        expected = [[1, 3],
                    [2, 3]]
        self.env.assertEqual(actual_result.result_set, expected)

        # test a WITH * projection that also introduces a new variable
        # and is explicitly returned
        query = """UNWIND range(1, 2) AS x WITH *, 3 AS y RETURN x, y"""
        actual_result = redis_graph.query(query)
        self.env.assertEqual(actual_result.result_set, expected)

    # verify that duplicate aliases only result in a single column
    def test04_duplicate_removal(self):
        # create a single node connected to itself
        n = Node(node_id=0, label="L", properties={"v": 1})
        e = Edge(n, "R", n)
        redis_graph.add_node(n)
        redis_graph.add_edge(e)
        redis_graph.flush()

        query = """MATCH (a)-[]->(a) RETURN *"""
        actual_result = redis_graph.query(query)
        expected = [[n]]
        self.env.assertEqual(actual_result.result_set, expected)

        query = """MATCH (a)-[]->(a) RETURN *, a"""
        actual_result = redis_graph.query(query)
        self.env.assertEqual(actual_result.result_set, expected)

    # verify that explicitly returning children that can have predicates
    # alongside a star projection does not result in errors
    def test05_star_and_nonpredicate_children(self):
        # create a single node
        self.env.flush()
        n = Node(node_id=0, label="L", properties={"v": 1})
        redis_graph.add_node(n)
        redis_graph.flush()

        try:
            query = """WITH 5 AS a RETURN *, NONE(t0 IN TRUE * COUNT(*))"""
            redis_graph.query(query)
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("'NONE' function requires a WHERE predicate", str(e))

        query = """MATCH (v0) RETURN *, [()-[]->() | v0]"""
        actual_result = redis_graph.query(query)
        expected = [[n, []]]
        self.env.assertEqual(actual_result.result_set, expected)
