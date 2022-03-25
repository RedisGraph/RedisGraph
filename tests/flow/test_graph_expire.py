import time
import redis
from RLTest import Env
from base import FlowTestsBase
from redisgraph import Graph, Node, Edge

redis_graph = None
redis_con = None
GRAPH_ID = "expire_test"

class testMap(FlowTestsBase):
    def __init__(self):
        global redis_graph
        self.env = Env(decodeResponses=True)
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)
        self.populate_graph()

    def expire_graph(self):
        self.env.assertTrue(redis_con.exists(GRAPH_ID))    
        self.env.assertTrue( redis_con.expire(GRAPH_ID, 100))
        time.sleep(0.3)
        self.env.assertFalse(redis_con.exists(GRAPH_ID))    
        try:
            slowlog = redis_con.execute_command("GRAPH.SLOWLOG", GRAPH_ID)
        except ResponseError as e:
            self.env.assertIn("Invalid graph operation on empty key", str(e)
 
