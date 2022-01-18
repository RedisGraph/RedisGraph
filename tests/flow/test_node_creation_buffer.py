import redis
from RLTest import Env
from redisgraph import Graph

GRAPH_ID = "NodeCreationBuffer"
conn = None

class testNodeCreationBuffer():
    def __init__(self):
        self.env = Env(decodeResponses=True, moduleArgs='NODE_CREATION_BUFFER 0')
        global conn
        global redis_graph
        conn = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, conn)

    # validate that the configuration exists and is loaded
    def test01_check_config_default(self):
        creation_buffer_size = conn.execute_command("GRAPH.CONFIG", "GET", "NODE_CREATION_BUFFER")[1]
        # values less than 128 (such as 0, which this module was loaded with)
        # will be increased to 128
        self.env.assertEqual(creation_buffer_size, 128)

    # validate that the creation of large graphs works without a large node creation buffer
    def test02_create_entities(self):
        query = """UNWIND range(1, 100000) AS x CREATE (:L {v: x})"""
        actual_result = redis_graph.query(query)
        self.env.assertEqual(actual_result.nodes_created, 100000)
