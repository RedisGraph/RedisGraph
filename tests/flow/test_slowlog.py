import redis
from RLTest import Env
from redisgraph import Graph
from base import FlowTestsBase

GRAPH_ID = "slowlog_test"
redis_con = None
redis_graph = None

class testSlowLog(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_con
        global redis_graph

        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)

    def test_slowlog(self):
        # Issue create query twice.
        redis_graph.query("""CREATE ()""")
        redis_graph.query("""CREATE ()""")

        # Slow log should contain a single entry, no duplicates.
        slowlog = redis_con.execute_command("GRAPH.SLOWLOG " + GRAPH_ID)
        self.env.assertEquals(len(slowlog), 1)

        # Saturate slowlog.
        for i in range(1024):
            q = """CREATE ({v:%s})""" % i
            redis_graph.query(q)
        A = redis_con.execute_command("GRAPH.SLOWLOG " + GRAPH_ID)
        B = redis_con.execute_command("GRAPH.SLOWLOG " + GRAPH_ID)

        # Calling slowlog multiple times should preduce the same result.
        self.env.assertEquals(A, B)

        # Issue a long running query, this should replace an existing entry in the slowlog.
        q = """MATCH (n), (m) WHERE n.v > 0 AND n.v < 500 SET m.v = rand() WITH n, m RETURN SUM(n.v + m.v)"""
        redis_graph.query(q)
        B = redis_con.execute_command("GRAPH.SLOWLOG " + GRAPH_ID)

        self.env.assertNotEqual(A, B)
