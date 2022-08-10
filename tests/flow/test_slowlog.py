from distutils.version import StrictVersion
from common import *

GRAPH_ID = "slowlog_test"
redis_con = None
redis_graph = None

class testSlowLog():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_con
        global redis_graph

        redis_con = self.env.getConnection()
        redis_graph = Graph(redis_con, GRAPH_ID)

    def test01_slowlog(self):
        # Slowlog should fail when graph doesn't exists.
        try:
            slowlog = redis_con.execute_command("GRAPH.SLOWLOG", "NONE_EXISTING_GRAPH")
        except ResponseError as e:
            self.env.assertIn("Invalid graph operation on empty key", str(e))

        # Issue create query twice.
        redis_graph.query("""CREATE ()""")
        redis_graph.query("""CREATE ()""")

        # Slow log should contain a single entry, no duplicates.
        slowlog = redis_con.execute_command("GRAPH.SLOWLOG", GRAPH_ID)
        self.env.assertEquals(len(slowlog), 1)

        # Saturate slowlog.
        for i in range(1024):
            q = """CREATE ({v:%s})""" % i
            redis_graph.query(q)
        A = redis_con.execute_command("GRAPH.SLOWLOG", GRAPH_ID)
        B = redis_con.execute_command("GRAPH.SLOWLOG", GRAPH_ID)

        # Calling slowlog multiple times should preduce the same result.
        self.env.assertEquals(A, B)

        server = redis_con.info("Server")
        if StrictVersion(server["redis_version"]) < StrictVersion("6.2.0"):
            # redis < 6.2.0 not support slowlog time measure
            return

        # Issue a long running query, this should replace an existing entry in the slowlog.
        q = """MATCH (n), (m) WHERE n.v > 0 AND n.v < 500 SET m.v = rand() WITH n, m RETURN SUM(n.v + m.v)"""
        redis_graph.query(q)
        B = redis_con.execute_command("GRAPH.SLOWLOG", GRAPH_ID)

        self.env.assertNotEqual(A, B)

        # get redis slowlog
        slowlog = redis_con.slowlog_get()
        slowlog_commands = [log["command"] for log in slowlog]

        # validate the command added to redis slowlog
        self.env.assertGreater(len(slowlog), 0)
        self.env.assertContains(b"GRAPH.QUERY slowlog_test MATCH (n), (m) WHERE n.v > 0 AND n.v < 500 SET m.v = rand() WITH n, m RETURN SUM(n.v + m.v) --compact", slowlog_commands)

    def test02_slowlog_reset(self):
        # reset none existing slowlog
        try:
            slowlog = redis_con.execute_command("GRAPH.SLOWLOG", "NONE_EXISTING_GRAPH", "RESET")
        except ResponseError as e:
            self.env.assertIn("Invalid graph operation on empty key", str(e))

        # issue an unknown slowlog sub command
        try:
            slowlog = redis_con.execute_command("GRAPH.SLOWLOG", GRAPH_ID, "UNKNOW_SUB_CMD")
        except ResponseError as e:
            self.env.assertIn("Unknown subcommand", str(e))

        # clear an existing slowlog
        slowlog = redis_con.execute_command("GRAPH.SLOWLOG", GRAPH_ID)
        self.env.assertGreater(len(slowlog), 0)

        redis_con.execute_command("GRAPH.SLOWLOG", GRAPH_ID, "RESET")
        redis_con.execute_command("GRAPH.SLOWLOG", GRAPH_ID, "RESET")

        slowlog = redis_con.execute_command("GRAPH.SLOWLOG", GRAPH_ID)
        self.env.assertEquals(len(slowlog), 0)

