from common import *

redis_graph = None
GRAPH_ID = "force_unlock"


class test_ForceUnlock():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(redis_con, GRAPH_ID)

    # Check that when the last operation
    # did not performed any modifications
    # the locks are released appropriately
    def test01_last_operation_lock(self):
        logfilename = self.env.envRunner._getFileName("master", ".log")
        logfile = open(f"{self.env.logDir}/{logfilename}")

        # populate graph
        redis_graph.query("CREATE(:N)")

        queries = [
            # query is expected to:
            # 1. lock
            # 2. create (l:L) and (m:M)
            # 3. match (n:N)
            # 4. unlock
            """MERGE (l:L) MERGE (m:M) MERGE (n:N) RETURN l, m, n""",
            # query is expected to:
            # 1. lock
            # 2. create (n:N)
            # 3. match (n)
            # 4. unlock
            """CREATE (n:N) WITH 1 AS x MATCH (n) WHERE n.v > 2 SET n.v = 2""",
            # query is expected to:
            # 1. lock
            # 2. create (n:N)
            # 3. match
            # 4. unlock
            """CREATE (n:N) WITH n MATCH (x) WHERE 1 > 2 CREATE  (m:M)"""
        ]

        for query in queries:
            redis_graph.query(query)

        log = logfile.read()
        self.env.assertNotContains("forced unlocking commit", log)
