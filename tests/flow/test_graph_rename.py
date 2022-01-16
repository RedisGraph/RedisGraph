from RLTest import Env
from redisgraph import Graph

class test_graph_rename():
    def __init__(self):
        self.env = Env()
        self.redis_con = self.env.getConnection()
    
    def test01_dump_rename(self):
        self.redis_con.flushall()

        graph_name = "x"
        redis_graph = Graph(graph_name, self.redis_con)
        
        # Create node
        result = redis_graph.query("CREATE (:L)")
        self.env.assertEquals(result.nodes_created, 1)
        
        # Dump
        data = self.redis_con.dump(graph_name)

        self.redis_con.flushall()

        # Rename
        graph_name = "y"

        # Restore
        self.redis_con.restore(graph_name, 0, data)

        redis_graph = Graph(graph_name, self.redis_con)

        result = redis_graph.query("CREATE (:L)")
        self.env.assertEquals(result.nodes_created, 1)