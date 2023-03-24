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

        node_props = [(622, "Mo", 30), (819, "Bing", 20), (819, "Qiu", 40)]
        for idx, v in enumerate(node_props):
            node = Node(label="Person", properties={"id": v[0], "name": v[1], "age": v[2]})
            redis_graph.add_node(node)

        redis_graph.commit()

    def expect_error(self, query, expected_err_msg):
        try:
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertIn(expected_err_msg, str(e))

    def get_res_and_assertEquals(self, query, expected_result):
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

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

    def test02_order_by_projections(self):
        # test valid queries
        query_to_expected_result = {
            "MATCH (n:Person) RETURN n.age AS age ORDER BY n.age*1" : [[20], [30], [40]],
            # "MATCH (n:Person) RETURN n.age AS age ORDER BY age*(-1)" : [[40], [30], [20]],
            "MATCH (n:Person) WITH n.age AS age, count(n.age) AS cnt ORDER BY age, cnt RETURN age" : [[20],[30],[40]],
            "MATCH (n:Person) WITH n.age AS age, count(n.age) AS cnt ORDER BY n.age, count(n.age) RETURN age" : [[20],[30],[40]],
            "MATCH (n:Person) WITH n.age AS age, count(n.age) AS cnt ORDER BY n.age * (-1) RETURN age" : [[40], [30], [20]],
            "MATCH (n:Person) WITH n.age AS age, count(n.age) AS cnt ORDER BY (+1) * count(n.age), n.age RETURN age" : [[20],[30],[40]],
            "MATCH (n:Person) WITH n.age AS age RETURN age ORDER BY ((-1) * age)" : [[40], [30], [20]],
            # "MATCH (n:Person) WITH n.age AS age ORDER BY (age * (-1)) RETURN age" : [[40], [30], [20]],
            # "MATCH (n:Person) WITH n.age AS age, count(n.age) AS cnt ORDER BY n.age + count(n.age) RETURN sum(age)" : [[90]],
            "CYPHER offset=10 MATCH (n:Person) WITH n.age AS age, count(n.age) AS cnt ORDER BY $offset + count(n.age) RETURN sum(age)" : [[90]],
        }
        for query, expected_result in query_to_expected_result.items():
            self.get_res_and_assertEquals(query, expected_result)

        # test invalid queries
        variable_agg_error = "Ambiguous Aggregation Expression: in a WITH/RETURN with an aggregation, it is not possible to access variables not projected by the WITH/RETURN."
        function_agg_error = "Ambiguous Aggregation Expression: in a WITH/RETURN with an aggregation, it is not possible to use aggregation functions different to the projected by the WITH."
        queries_with_errors = {
            "MATCH (n:Person) WITH n.id AS age, count(n.age) AS cnt ORDER BY n RETURN 1" : variable_agg_error,
            "MATCH (n:Person) WITH n.id AS age, count(n.age) AS cnt ORDER BY n.name RETURN 1" : variable_agg_error,
            "MATCH (n:Person) WITH n.id AS age, count(n.age) AS cnt ORDER BY max(n.id) RETURN 1" : function_agg_error,
            "MATCH (n:Person) WITH n.id AS age, count(n.age) AS cnt ORDER BY sum(count(n.id)) RETURN 1" : function_agg_error,
            "MATCH (n:Person) WITH n.id AS age, count(n.age) ORDER BY n RETURN 1" : "WITH clause projections must be aliased",
            "MATCH (n:Person) WITH n.id AS age, count(n.age) AS cnt ORDER BY n.id RETURN sum(n.age)" : "n not defined",
            "MATCH (n:Person) RETURN count(n.age) AS agg ORDER BY n.age + count(n.age)" : variable_agg_error,
            "MATCH (n:Person) WITH n.age AS age, count(n.age) AS cnt ORDER BY $missing_parameter + count(n.age) RETURN sum(age)" : "Missing parameters",
            # "MATCH (n:Person) RETURN n.age + n.age, count(*) AS cnt ORDER BY n.age + n.age + count(*)" : "",
        }
        for query, expected_result in queries_with_errors.items():
            self.expect_error(query, expected_result)

    def test03_foreach(self):
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
