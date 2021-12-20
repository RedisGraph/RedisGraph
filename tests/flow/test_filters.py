from RLTest import Env
from redisgraph import Graph

class testFilters():
    def __init__(self):
        self.env = Env(decodeResponses=True)

    def test01_filter(self):
        g = Graph("g", self.env.getConnection())
        g.query("UNWIND range(1, 5) AS x CREATE (:N { v: x, b: x % 2 = 0 })")

        expected = [[i, j] for i in range(1, 6) for j in range(1, 6) if i % 2 == 0 and j % 2 == 0]
        result = g.query("MATCH (n:N), (m:N) WHERE n.b AND m.b RETURN n.v, m.v ORDER BY n.v, m.v")
        self.env.assertEqual(result.result_set,  expected)

        expected = [[i, j] for i in range(1, 6) for j in range(1, 6) if i % 2 == 0 or j % 2 == 0]
        result = g.query("MATCH (n:N), (m:N) WHERE n.b OR m.b RETURN n.v, m.v ORDER BY n.v, m.v")
        self.env.assertEqual(result.result_set,  expected)

        expected = [[i, j] for i in range(1, 6) for j in range(1, 6) if i % 2 != j % 2]
        result = g.query("MATCH (n:N), (m:N) WHERE n.b XOR m.b RETURN n.v, m.v ORDER BY n.v, m.v")
        self.env.assertEqual(result.result_set,  expected)

        expected = [[i, j] for i in range(1, 6) for j in range(1, 6) if not (i % 2 == 0 and j % 2 == 0)]
        result = g.query("MATCH (n:N), (m:N) WHERE NOT (n.b AND m.b) RETURN n.v, m.v ORDER BY n.v, m.v")
        self.env.assertEqual(result.result_set,  expected)

        expected = [[i, j] for i in range(1, 6) for j in range(1, 6) if not (i % 2 == 0 or j % 2 == 0)]
        result = g.query("MATCH (n:N), (m:N) WHERE NOT (n.b OR m.b) RETURN n.v, m.v ORDER BY n.v, m.v")
        self.env.assertEqual(result.result_set,  expected)

        expected = [[i, j] for i in range(1, 6) for j in range(1, 6) if not (i % 2 != j % 2)]
        result = g.query("MATCH (n:N), (m:N) WHERE NOT (n.b XOR m.b) RETURN n.v, m.v ORDER BY n.v, m.v")
        self.env.assertEqual(result.result_set,  expected)