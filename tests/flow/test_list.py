from RLTest import Env
from redisgraph import Graph, Node, Edge

from base import FlowTestsBase

redis_graph = None
GRAPH_ID = "G"


class testList(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)

    def test01_collect(self):
        for i in range(10):
            redis_graph.add_node(Node())
        redis_graph.commit()

        query = """MATCH (n) RETURN collect(n)"""
        result = redis_graph.query(query)
        result_set = result.result_set
        self.env.assertEquals(len(result_set), 1)
        self.env.assertTrue(all(isinstance(n, Node) for n in result_set[0][0]))

    def test02_unwind(self):
        query = """CREATE ()"""
        redis_graph.query(query)
        query = """unwind(range(0,10)) as x return x"""
        result_set = redis_graph.query(query).result_set
        expected_result = [[0], [1], [2], [3],
                           [4], [5], [6], [7], [8], [9], [10]]
        self.env.assertEquals(result_set, expected_result)
