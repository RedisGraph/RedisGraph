from common import *
import time

GRAPH_ID = "expire"


class testExpiry():
    def test01_expire_graph(self):
        # create a redisgraph object
        env = Env(decodeResponses=True)
        redis_con = env.getConnection()
        redis_graph = Graph(redis_con, GRAPH_ID)

        # create a single node
        redis_graph.query("create ()")

        # expire key in 100ms
        expire_in_ms = 100
        env.assertTrue(redis_con.pexpire(GRAPH_ID, expire_in_ms))

        # wait for key to expire
        time.sleep((expire_in_ms * 3) / 1000) # convert from sec to ms

        # key should have been evicted by now
        env.assertFalse(redis_con.exists(GRAPH_ID))
        try:
            redis_con.execute_command("GRAPH.SLOWLOG", GRAPH_ID)
        except ResponseError as e:
            env.assertIn("Invalid graph operation on empty key", str(e))
    
    def test02_eviction_graph(self):
        # create a redisgraph object
        env = Env(decodeResponses=True)
        redis_con = env.getConnection()

        # set memory policy
        redis_con.config_set("maxmemory-policy", "allkeys-lru")
        redis_con.config_set("maxmemory", "2mb")

        redis_graph = Graph(redis_con, GRAPH_ID)
        redis_graph_new = Graph(redis_con, GRAPH_ID + "new")

        # create a single node
        redis_graph.query("create ()")

        # create a single node utill OOM exception raised
        ex = None
        while "OOM command not allowed when used memory > 'maxmemory'" not in str(ex):
            try:
                redis_graph_new.query("create ()")
            except ResponseError as e:
                ex = e

        # key should have been evicted by now
        env.assertFalse(redis_con.exists(GRAPH_ID))
        try:
            redis_con.execute_command("GRAPH.SLOWLOG", GRAPH_ID)
        except ResponseError as e:
            env.assertIn("Invalid graph operation on empty key", str(e))

