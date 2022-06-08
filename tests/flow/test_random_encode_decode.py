from common import *
from random_graph import create_random_schema, create_random_graph, run_random_graph_ops, ALL_OPS


class test_random_encode_decode(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True,
                       moduleArgs='VKEY_MAX_ENTITY_COUNT 100 NODE_CREATION_BUFFER 100')
        self.redis_con = self.env.getConnection()

    def test01_random_graph(self):
        graph_name = "random_graph"
        redis_graph = Graph(self.redis_con, graph_name)

        nodes, edges = create_random_schema()
        res = create_random_graph(redis_graph, nodes, edges)

        nodes_before = redis_graph.query("MATCH (n) RETURN n")
        edges_before = redis_graph.query("MATCH ()-[e]->() RETURN e")

        self.redis_con.execute_command("DEBUG", "RELOAD")

        nodes_after = redis_graph.query("MATCH (n) RETURN n")
        edges_after = redis_graph.query("MATCH ()-[e]->() RETURN e")

        self.env.assertEquals(nodes_before.result_set, nodes_after.result_set)
        self.env.assertEquals(edges_before.result_set, edges_after.result_set)

        res = run_random_graph_ops(redis_graph, nodes, edges, ALL_OPS)

        nodes_before = redis_graph.query("MATCH (n) RETURN n")
        edges_before = redis_graph.query("MATCH ()-[e]->() RETURN e")

        self.redis_con.execute_command("DEBUG", "RELOAD")

        nodes_after = redis_graph.query("MATCH (n) RETURN n")
        edges_after = redis_graph.query("MATCH ()-[e]->() RETURN e")

        self.env.assertEquals(nodes_before.result_set, nodes_after.result_set)
        self.env.assertEquals(edges_before.result_set, edges_after.result_set)
