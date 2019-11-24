import redis
from redisgraph import Graph
from base import FlowTestsBase

GRAPH_ID = "union_test"
redis_graph = None

class testUnion(FlowTestsBase):
    def __init__(self):
        super(testUnion, self).__init__()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)

    def test_union(self):
        q = """RETURN 1 as one UNION ALL RETURN 1 as one"""
        result = redis_graph.query(q)
        # Expecting 2 identical records.
        self.env.assertEquals(len(result.result_set), 2)

        q = """RETURN 1 as one UNION RETURN 1 as one"""
        result = redis_graph.query(q)
        # Expecting a single record, duplicate removed.
        self.env.assertEquals(len(result.result_set), 1)

        # Issue #741, create some data for path matching.
        q = """CREATE (), (), ()"""
        redis_graph.query(q)
        q = """MATCH a = () return length(a) AS len UNION ALL MATCH b = () RETURN length(b) AS len"""
        result = redis_graph.query(q)
        # 3 records from each sub-query, coresponding to each path matched.
        self.env.assertEquals(len(result.result_set), 6)

    def test_invalid_union(self):
        try:
            # projection must be exactly the same.
            q = """RETURN 1 as one UNION RETURN 1 as two""" 
            redis_graph.query(q)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    def test_union_order(self):
        q = """UNWIND [1,2,3] AS i RETURN i LIMIT 1 UNION ALL UNWIND [1,2,3] AS i RETURN i ORDER BY i DESC"""
        result_set = redis_graph.query(q).result_set
        expected_result = [[1], [3], [2], [1]]
        self.env.assertEquals(result_set, expected_result)

        q = """UNWIND [1,2,3] AS i RETURN i LIMIT 1 UNION ALL UNWIND [1,2,3] AS i RETURN i ORDER BY i ASC"""
        result_set = redis_graph.query(q).result_set
        expected_result = [[1], [1], [2], [3]]
        self.env.assertEquals(result_set, expected_result)
