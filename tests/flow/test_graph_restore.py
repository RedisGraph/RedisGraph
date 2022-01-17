from RLTest import Env
from redisgraph import Graph

# test restoring a graph in different scenarios


class test_graph_restore():
    def __init__(self):
        self.env = Env()
        self.redis_con = self.env.getConnection()

    # test restoring a graph using different key
    def test01_restore_using_different_key(self):
        self.redis_con.flushall()

        x = Graph("x", self.redis_con)

        # create data
        result = x.query("CREATE (:L{v: 1})")
        self.env.assertEquals(result.nodes_created, 1)

        # dump graph x
        data = self.redis_con.dump("x")

        # restore graph x in key y
        self.redis_con.restore("y", 0, data)

        y = Graph("y", self.redis_con)

        # add another node to graph y
        result = y.query("CREATE (:L {v: 2})")
        self.env.assertEquals(result.nodes_created, 1)

        # validate data in graph x
        result = x.query("MATCH (a:L) RETURN a.v ORDER BY a.v")
        expected_result = [[1]]
        self.env.assertEquals(result.result_set, expected_result)

        # validate data in graph y
        result = y.query("MATCH (a:L) RETURN a.v ORDER BY a.v")
        expected_result = [[1], [2]]
        self.env.assertEquals(result.result_set, expected_result)
