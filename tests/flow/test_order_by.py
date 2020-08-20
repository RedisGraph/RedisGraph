from RLTest import Env
from redisgraph import Graph, Node
from base import FlowTestsBase

GRAPH_ID = "order_by_test"
redis_graph = None


class testOrderBy(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)
        self.populate_graph()

    def populate_graph(self):
        global redis_graph

        node_props = [(622, "Mo"), (819, "Bing"), (819, "Qiu")]
        for idx, v in enumerate(node_props):
            node = Node(label="Person", properties={"id": v[0], "name": v[1]})
            redis_graph.add_node(node)

        redis_graph.commit()

    def test_multiple_order_by(self):
        # Query with multiple order by operation
        q = """MATCH (n:Person) RETURN n.id, n.name ORDER BY n.id DESC, n.name ASC"""
        expected = [[819, "Bing"], [819, "Qiu"], [622, "Mo"]]
        actual_result = redis_graph.query(q)
        self.env.assertEquals(actual_result.result_set, expected)

        # Same query with limit, force use heap sort
        q = """MATCH (n:Person) RETURN n.id, n.name ORDER BY n.id DESC, n.name ASC LIMIT 10"""
        actual_result = redis_graph.query(q)
        self.env.assertEquals(actual_result.result_set, expected)
