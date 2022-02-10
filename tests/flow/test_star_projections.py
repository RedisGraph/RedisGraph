import redis
from RLTest import Env
from redisgraph import Graph

GRAPH_ID = "starProjection"
redis_graph = None

class testStarProjections():

    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)

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

        query = """UNWIND range(1, 2) AS x UNWIND range(3, 4) AS y WITH * SKIP 1 LIMIT 2 RETURN *"""
        actual_result = redis_graph.query(query)
        expected = [[1, 4],
                    [2, 3]]
        self.env.assertEqual(actual_result.result_set, expected)

        query = """UNWIND range(5, 0, -1) AS x WITH x ORDER BY x SKIP 2 LIMIT 3 RETURN *"""
        actual_result = redis_graph.query(query)
        expected = [[2],
                    [3],
                    [4]]
        self.env.assertEqual(actual_result.result_set, expected)

