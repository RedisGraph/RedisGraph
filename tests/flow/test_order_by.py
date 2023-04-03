from common import *

GRAPH_ID = "order_by_test"
redis_graph = None


class testOrderBy(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(redis_con, GRAPH_ID)
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

    def test02_foreach(self):
        """Tests that ORDER BY works properly with FOREACH before it"""

        # clean db
        self.env.flush()
        redis_graph = Graph(self.env.getConnection(), GRAPH_ID)

        res = redis_graph.query("CREATE (:N {v: 1}), (:N {v: 2})")
        self.env.assertEquals(res.nodes_created, 2)

        res = redis_graph.query(
            """
            MATCH (n:N)
            FOREACH(node in [n] |
                SET n.v = n.v
            )
            RETURN n
            ORDER BY n.v DESC
            """
        )

        # assert the order of the results
        self.env.assertEquals(res.result_set[0][0], Node(label='N', properties={'v': 2}))
        self.env.assertEquals(res.result_set[1][0], Node(label='N', properties={'v': 1}))

        res = redis_graph.query(
            """
            MATCH (n:N)
            FOREACH(node in [n] |
                SET n.v = n.v
            )
            RETURN n
            ORDER BY n.v ASC
            """
        )

        # assert the order of the results
        self.env.assertEquals(res.result_set[0][0], Node(label='N', properties={'v': 1}))
        self.env.assertEquals(res.result_set[1][0], Node(label='N', properties={'v': 2}))
