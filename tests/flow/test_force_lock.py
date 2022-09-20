from common import *

redis_graph = None
GRAPH_ID = "force_unlock"


class test_ForceUnlock():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(redis_con, GRAPH_ID)

    # Check that when the last operation is merge GIL was unlocked
    def test01_merge_lock(self):
        logfilename = self.env.envRunner._getFileName("master", ".log")
        logfile = open(f"{self.env.logDir}/{logfilename}")
        log = logfile.read()

        query = """MERGE (n:N) RETURN n"""
        redis_graph.query(query)

        # query is expected to:
        # 1. lock
        # 2. create (l:L) and (m:M)
        # 3. match (n:N)
        # 4. unlock
        query = """MERGE (l:L) MERGE (m:M) MERGE (n:N) RETURN l, m, n"""
        redis_graph.query(query)

        log = logfile.read()
        self.env.assertNotContains("forced unlocking commit", log)

    # Check that when the last operation is update GIL was unlocked
    def test02_update_lock(self):
        logfilename = self.env.envRunner._getFileName("master", ".log")
        logfile = open(f"{self.env.logDir}/{logfilename}")
        log = logfile.read()

        # query is expected to:
        # 1. lock
        # 2. create (n:N)
        # 3. unlock
        query = """CREATE (n:N) WITH 1 AS x MATCH (n) WHERE n.v > 2 SET n.v = 2"""
        redis_graph.query(query)

        log = logfile.read()
        self.env.assertNotContains("forced unlocking commit", log)

    # Check that when the last operation is create GIL was unlocked
    def test03_create_lock(self):
        logfilename = self.env.envRunner._getFileName("master", ".log")
        logfile = open(f"{self.env.logDir}/{logfilename}")
        log = logfile.read()

        # query is expected to:
        # 1. lock
        # 2. create (n:N)
        # 3. unlock
        query = """CREATE (n:N) WITH n MATCH (x) WHERE 1 > 2 CREATE  (m:M)"""
        redis_graph.query(query)

        log = logfile.read()
        self.env.assertNotContains("forced unlocking commit", log)
