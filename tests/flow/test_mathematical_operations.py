from common import *

class testMathematicalOperationsFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        self.redis_con = self.env.getConnection()
        self.graph = Graph(self.redis_con, "G")
        self.populate_graph()

    def populate_graph(self):
        people = ["Roi", "Alon", "Ailon", "Boaz"]
        nodes = {}
        # Create entities
        for idx, p in enumerate(people):
            if idx % 2 == 0:
                labels = ["person"]
            else:
                labels = ["person", "student"]
            node = Node(label=labels, properties={"name": p, "val": idx})
            self.graph.add_node(node)
            nodes[p] = node

        # Fully connected graph
        for src in nodes:
            for dest in nodes:
                if src != dest:
                    edge = Edge(nodes[src], "know", nodes[dest])
                    self.graph.add_edge(edge)

        for src in nodes:
            for dest in nodes:
                if src != dest:
                    edge = Edge(nodes[src], "works_with", nodes[dest])
                    self.graph.add_edge(edge)

        self.graph.commit()
        query = """MATCH (a)-[:know]->(b) CREATE (a)-[:know]->(b)"""
        self.graph.query(query)

    def expect_error(self, query, expected_err_msg):
        try:
            self.graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertIn(expected_err_msg, str(e))

    def expect_type_error(self, query):
        self.expect_error(query, "Type mismatch")

    # GitHub Issue #2291.
    # Ensure that a node can be added to a list.
    def test01_node_add_to_array(self):
        query = """MATCH (a) WITH collect(a) AS as RETURN as[0] + [true, 2, 3]"""
        actual_result = self.graph.query(query)
        expected_result = [Node(label='person', properties = {'name': 'Roi', 'val': 0}), True, 2, 3]
        self.env.assertEquals(actual_result.result_set[0][0], expected_result)

    # Ensure that an edge can be added to a list.
    def test02_edge_add_to_array(self):
        query = """MATCH (a)-[r]->(b) WITH collect(r) AS rs RETURN rs[0] + [true, 2, 3]"""
        actual_result = self.graph.query(query)
        expected_result = [True, 2, 3]
        self.env.assertTrue(isinstance(actual_result.result_set[0][0][0], Edge))
        self.env.assertEquals(actual_result.result_set[0][0][1:], expected_result)

    # Ensure that a path can be added to a list.
    def test03_path_add_to_array(self):
        query = """MATCH path = (a)-[r]->(b) RETURN path + [true, 2, 3]"""
        actual_result = self.graph.query(query)
        expected_result = [True, 2, 3]
        self.env.assertTrue(isinstance(actual_result.result_set[0][0][0], Path))
        self.env.assertEquals(actual_result.result_set[0][0][1:], expected_result)

    # Ensure that a point can be added to a list.
    def test04_point_add_to_array(self):
        query = """MATCH () RETURN point({latitude:1, longitude:2}) + [true, 2, 3] LIMIT 1"""
        actual_result = self.graph.query(query)
        expected_result = [{'latitude': 1, 'longitude': 2}, True, 2, 3]
        self.env.assertEquals(actual_result.result_set[0][0], expected_result)

    # Ensure that complex objects can't be used in an addition operation.
    def test05_adding_complex_objects_with_not_self_causes_error(self):
        args = [
            '2',
            '2.0',
            'true',
            'timestamp()',
            '"string"',
        ]
        query_templates = [
            """MATCH (a) RETURN a + %s LIMIT 1""",
            """MATCH (a) RETURN %s + a LIMIT 1""",
            """MATCH ()-[r]->() RETURN r + %s LIMIT 1""",
            """MATCH ()-[r]->() RETURN %s + r LIMIT 1""",
            """MATCH path = ()-[r]->() RETURN path + %s LIMIT 1""",
            """MATCH path = ()-[r]->() RETURN %s + path LIMIT 1""",
            """MATCH () RETURN point({latitude:1, longitude:2}) + %s LIMIT 1""",
            """MATCH () RETURN %s + point({latitude:1, longitude:2}) LIMIT 1""",
        ]
        for template in query_templates:
            for arg in args:
                self.expect_type_error(template % arg)
