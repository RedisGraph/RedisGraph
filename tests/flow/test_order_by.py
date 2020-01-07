from redisgraph import Graph
from base import FlowTestsBase

GRAPH_ID = "order_by_test"
redis_graph = None


class testOrderBy(FlowTestsBase):
    def __init__(self):
        super(testOrderBy, self).__init__()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)

    def test_multiple_order_by(self):
        q = """CREATE (:Person { id: 622, name: "Mo" })"""
        redis_graph.query(q)

        q = """CREATE (:Person { id: 819, name: "Bing" })"""
        redis_graph.query(q)

        q = """MATCH (n:Person) RETURN n.id ORDER BY n.id DESC, n.name ASC"""
        actual_result = redis_graph.query(q)
        print actual_result.result_set
        expected = [[819], [622]]
        self.env.assertEquals(actual_result.result_set, expected)
