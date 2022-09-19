from common import *

redis_graph = None
GRAPH_ID = "G"


class testLock(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(redis_con, GRAPH_ID)

    def test01_multi_merge_lock(self):
        logfilename = self.env.envRunner._getFileName("master", ".log")
        logfile = open(f"{self.env.logDir}/{logfilename}")
        log = logfile.read()

        query = """MERGE (l:N) RETURN l"""
        redis_graph.query(query)

        query = """MERGE (l:L) MERGE (m:M) MERGE (n:N) RETURN l, m, n"""
        redis_graph.query(query)

        log = logfile.read()
        self.env.assertNotContains("forced unlocking commit", log)
