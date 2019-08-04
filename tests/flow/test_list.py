from redisgraph import Graph, Node, Edge

from base import FlowTestsBase

redis_graph = None


class testGraphMergeFlow(FlowTestsBase):
    def __init__(self):
        super(testGraphMergeFlow, self).__init__()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph("G", redis_con)

    def test01_collect(self):
        for i in range(10):
            query = """CREATE ()"""
            redis_graph.query(query)

        query = """MATCH (n) RETURN collect(n)"""
        result = redis_graph.query(query)
        result_set = result.result_set
        self.env.assertEquals(len(result_set), 1)
        for i in range(10):
            self.env.assertTrue(isinstance(result_set[0][0][i-1], Node))

    def test02_unwind(self):
        query = """CREATE ()"""
        redis_graph.query(query)
        query = """unwind(range(0,10)) as x return x"""
        result_set = redis_graph.query(query).result_set
        for i in range(0, 10):
            self.env.assertEquals(i, result_set[i][0])
