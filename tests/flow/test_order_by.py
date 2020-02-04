from redisgraph import Graph, Node
from base import FlowTestsBase

GRAPH_ID = "order_by_test"
redis_graph = None


class testOrderBy(FlowTestsBase):
    def __init__(self):
        super(testOrderBy, self).__init__()
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

    def test01_multiple_order_by(self):
        # Query with multiple order by operation
        q = """MATCH (n:Person) RETURN n.id, n.name ORDER BY n.id DESC, n.name ASC"""
        expected = [[819, "Bing"], [819, "Qiu"], [622, "Mo"]]
        actual_result = redis_graph.query(q)
        self.env.assertEquals(actual_result.result_set, expected)

        # Same query with limit, force use heap sort
        q = """MATCH (n:Person) RETURN n.id, n.name ORDER BY n.id DESC, n.name ASC LIMIT 10"""
        actual_result = redis_graph.query(q)
        self.env.assertEquals(actual_result.result_set, expected)

    def test02_order_by_function_on_alias(self):
        # Validate that aliases introduced by the RETURN clause can be accessed by nested ORDER BY expressions.
        q = """MATCH (n:Person) RETURN n.id AS id ORDER BY toInteger(id)"""
        expected = [[622], [819], [819]]
        actual_result = redis_graph.query(q)
        self.env.assertEquals(actual_result.result_set, expected)

    def test03_order_by_input_and_projection_vals(self):
        # Validate that ORDER BY expressions that rely on both inputted and calculated values evaluate properly.
        q = """MATCH (n:Person) RETURN n.id AS id ORDER BY id + n.id"""
        expected = [[622], [819], [819]]
        actual_result = redis_graph.query(q)
        self.env.assertEquals(actual_result.result_set, expected)
