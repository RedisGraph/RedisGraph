import redis
from RLTest import Env
from redisgraph import Graph, Node, Edge
class testAllShortestPaths():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        redis_con = self.env.getConnection()
        self.graph = Graph("all_shortest_paths", redis_con)
        self.cyclic_graph = Graph("all_shortest_paths_cyclic", redis_con)
        self.populate_graph()
        self.populate_cyclic_graph()

    def populate_graph(self):
        # Construct a graph with the form:
        # (v1)-[:E]->(v2)-[:E]->(v3)-[:E]->(v4)
        # (v1)-[:E]->(v5)-[:E2]->(v4)
        # (v2)-[:E2]->(v4)

        self.v1 = Node(label="L", properties={"v": 1})
        self.v2 = Node(label="L", properties={"v": 2})
        self.v3 = Node(label="L", properties={"v": 3})
        self.v4 = Node(label="L", properties={"v": 4})
        self.v5 = Node(label="L", properties={"v": 5})

        self.graph.add_node(self.v1)
        self.graph.add_node(self.v2)
        self.graph.add_node(self.v3)
        self.graph.add_node(self.v4)
        self.graph.add_node(self.v5)

        e = Edge(self.v1, "E", self.v2, properties={"weight": 1})
        self.graph.add_edge(e)

        e = Edge(self.v2, "E", self.v3, properties={"weight": 1})
        self.graph.add_edge(e)

        e = Edge(self.v3, "E", self.v4, properties={"weight": 1})
        self.graph.add_edge(e)

        e = Edge(self.v1, "E", self.v5, properties={"weight": 1})
        self.graph.add_edge(e)

        e = Edge(self.v5, "E2", self.v4, properties={"weight": 1})
        self.graph.add_edge(e)

        e = Edge(self.v2, "E2", self.v4, properties={"weight": 2})
        self.graph.add_edge(e)

        self.graph.flush()

    def populate_cyclic_graph(self):
        # Construct a graph with the form:
        # (v1)-[:E]->(v2)-[:E]->(v3)-[:E]->(v4)
        # (v1)-[:E]->(v5)-[:E2]->(v4)
        # (v2)-[:E2]->(v4)
        # (v3)-[:E2]->(v1)
        # (v4)-[:E2]->(v1)

        self.cyclic_v1 = Node(label="L", properties={"v": 1})
        self.cyclic_v2 = Node(label="L", properties={"v": 2})
        self.cyclic_v3 = Node(label="L", properties={"v": 3})
        self.cyclic_v4 = Node(label="L", properties={"v": 4})
        self.cyclic_v5 = Node(label="L", properties={"v": 5})

        self.cyclic_graph.add_node(self.cyclic_v1)
        self.cyclic_graph.add_node(self.cyclic_v2)
        self.cyclic_graph.add_node(self.cyclic_v3)
        self.cyclic_graph.add_node(self.cyclic_v4)
        self.cyclic_graph.add_node(self.cyclic_v5)

        e = Edge(self.cyclic_v1, "E", self.cyclic_v2, properties={"weight": 1})
        self.cyclic_graph.add_edge(e)

        e = Edge(self.cyclic_v2, "E", self.cyclic_v3, properties={"weight": 1})
        self.cyclic_graph.add_edge(e)

        e = Edge(self.cyclic_v3, "E", self.cyclic_v4, properties={"weight": 1})
        self.cyclic_graph.add_edge(e)

        e = Edge(self.cyclic_v1, "E", self.cyclic_v5, properties={"weight": 1})
        self.cyclic_graph.add_edge(e)

        e = Edge(self.cyclic_v5, "E2", self.cyclic_v4, properties={"weight": 1})
        self.cyclic_graph.add_edge(e)

        e = Edge(self.cyclic_v2, "E2", self.cyclic_v4, properties={"weight": 2})
        self.cyclic_graph.add_edge(e)

        e = Edge(self.cyclic_v3, "E2", self.cyclic_v1, properties={"weight": 2})
        self.cyclic_graph.add_edge(e)

        e = Edge(self.cyclic_v4, "E2", self.cyclic_v1, properties={"weight": 2})
        self.cyclic_graph.add_edge(e)

        self.cyclic_graph.flush()

    def test01_invalid_shortest_paths(self):
        # running against following graph
        #
        # (v1)-[:E]->(v2)-[:E]->(v3)-[:E]->(v4)
        # (v1)-[:E]->(v5)-[:E2]->(v4)
        # (v2)-[:E2]->(v4)

        # Test unbound variables
        query = """MATCH (v1 {v: 1}), (v4 {v: 4}), p = allShortestPaths((v1)-[*]->(v4))
                   RETURN p"""
        try:
            self.graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertIn("Source and destination must already be resolved to call allShortestPaths", str(e))

        # Test non-node endpoint
        query = """MATCH (v1 {v: 1}) WITH v1, 'stringval' AS v4
                   MATCH p = allShortestPaths((v1)-[*]->(v4))
                   RETURN p"""
        try:
            self.graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertIn("encountered unexpected type in Record; expected Node", str(e))

        # Test NULL endpoint
        query = """MATCH (v1 {v: 1}) OPTIONAL MATCH (v9 {v: v9}) WITH v1, v9 MATCH p = allShortestPaths((v1)-[*]->(v9)) RETURN p"""
        actual_result = self.graph.query(query)
        expected_result = []
        self.env.assertEqual(actual_result.result_set, expected_result)

        query = """MATCH (v1 {v: 1}), (v4 {v: 4})
                   WITH v1, v4
                   MATCH p = allShortestPaths((v1)-[*3..]->(v4))
                   RETURN nodes(p) AS nodes
                   ORDER BY nodes"""

        try:
            self.graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertIn("allShortestPaths(...) does not support a minimal length different from 1", str(e))

        # Test unsupported in return
        query = """MATCH (a),(b)
                   RETURN allShortestPaths((a)-[*]->(b))"""

        try:
            self.graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertIn("RedisGraph support allShortestPaths only in match clauses", str(e))
        
        # Test unsupported in where
        query = """MATCH (a),(b)
                   WHERE size(allShortestPaths((a)-[*]->(b))) > 0
                   RETURN a, b"""

        try:
            self.graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertIn("RedisGraph support allShortestPaths only in match clauses", str(e))

    def test02_all_shortest_paths(self):
        # running against following graph
        #
        # (v1)-[:E]->(v2)-[:E]->(v3)-[:E]->(v4)
        # (v1)-[:E]->(v5)-[:E2]->(v4)
        # (v2)-[:E2]->(v4)

        query = """MATCH (v1 {v: 1}), (v4 {v: 4})
                   WITH v1, v4
                   MATCH p = allShortestPaths((v1)-[*]->(v4))
                   RETURN nodes(p) AS nodes
                   ORDER BY nodes"""

        actual_result = self.graph.query(query)
        # The 2 2-hop traversals should be found
        expected_result = [[[self.v1, self.v2, self.v4]],
                           [[self.v1, self.v5, self.v4]]]
        self.env.assertEqual(actual_result.result_set, expected_result)

        # Verify that a right-to-left traversal produces the same results
        query = """MATCH (v1 {v: 1}), (v4 {v: 4})
                   WITH v1, v4
                   MATCH p = allShortestPaths((v4)<-[*]-(v1))
                   RETURN nodes(p) AS nodes
                   ORDER BY nodes"""

        actual_result = self.graph.query(query)
        expected_result = [[[self.v4, self.v2, self.v1]],
                           [[self.v4, self.v5, self.v1]]]
        self.env.assertEqual(actual_result.result_set, expected_result)

        query = """MATCH (v1 {v: 1}), (v4 {v: 4})
                   WITH v1, v4
                   MATCH p = allShortestPaths((v1)-[*]-(v4))
                   RETURN nodes(p) AS nodes
                   ORDER BY nodes"""

        actual_result = self.graph.query(query)
        expected_result = [[[self.v1, self.v2, self.v4]],
                           [[self.v1, self.v5, self.v4]]]
        self.env.assertEqual(actual_result.result_set, expected_result)

    def test03_all_shortest_multiple_traversals(self):
        # running against following graph
        #
        # (v1)-[:E]->(v2)-[:E]->(v3)-[:E]->(v4)
        # (v1)-[:E]->(v5)-[:E2]->(v4)
        # (v2)-[:E2]->(v4)

        query = """MATCH (v1 {v: 1}), (v4 {v: 4})
                   WITH v1, v4
                   MATCH p = allShortestPaths((v1)-[:E]->(:L)-[*..]->(v4))
                   RETURN nodes(p) AS nodes
                   ORDER BY nodes"""

        actual_result = self.graph.query(query)
        # The single 3-hop traversal should be found
        expected_result = [[[self.v1, self.v2, self.v4]], 
                           [[self.v1, self.v5, self.v4]]]
        self.env.assertEqual(actual_result.result_set, expected_result)

        # Verify that a right-to-left traversal produces the same results
        query = """MATCH (v1 {v: 1}), (v4 {v: 4})
                   WITH v1, v4
                   MATCH p = allShortestPaths((v4)<-[*..]-(:L)<-[:E]-(v1))
                   RETURN nodes(p) AS nodes
                   ORDER BY nodes"""

        actual_result = self.graph.query(query)
        expected_result = [[[self.v4, self.v2, self.v1]], 
                           [[self.v4, self.v5, self.v1]]]
        self.env.assertEqual(actual_result.result_set, expected_result)

    def test04_all_shortest_edge_filter(self):
        # running against following graph
        #
        # (v1)-[:E {weight:1}]->(v2)-[:E {weight:1}]->(v3)-[:E {weight:1}]->(v4)
        # (v1)-[:E {weight:1}]->(v5)-[:E2 {weight:1}]->(v4)
        # (v2)-[:E2 {weight:2}]->(v4)

        query = """MATCH (v1 {v: 1}), (v4 {v: 4})
                   WITH v1, v4
                   MATCH p = allShortestPaths((v1)-[* {weight: 1}]->(v4))
                   RETURN nodes(p) AS nodes
                   ORDER BY nodes"""
        actual_result = self.graph.query(query)
        # The 1 2-hop traversal with all edge weights of 1 should be found
        expected_result = [[[self.v1, self.v5, self.v4]]]
        self.env.assertEqual(actual_result.result_set, expected_result)

        # Verify that a right-to-left traversal produces the same results
        query = """MATCH (v1 {v: 1}), (v4 {v: 4})
                   WITH v1, v4
                   MATCH p = allShortestPaths((v4)<-[* {weight: 1}]-(v1))
                   RETURN nodes(p) AS nodes
                   ORDER BY nodes"""

        actual_result = self.graph.query(query)
        expected_result = [[[self.v4, self.v5, self.v1]]]
        self.env.assertEqual(actual_result.result_set, expected_result)

    def test05_all_shortest_no_results(self):
        # running against following graph
        #
        # (v1)-[:E]->(v2)-[:E]->(v3)-[:E]->(v4)
        # (v1)-[:E]->(v5)-[:E2]->(v4)
        # (v2)-[:E2]->(v4)

        query = """MATCH (v5 {v: 5}), (v3 {v: 3})
                   WITH v5, v3
                   MATCH p = allShortestPaths((v5)-[*]->(v3))
                   RETURN nodes(p) AS nodes
                   ORDER BY nodes"""

        actual_result = self.graph.query(query)
        # No results should be found
        expected_result = []
        self.env.assertEqual(actual_result.result_set, expected_result)

        # Verify that a right-to-left traversal produces the same results
        query = """MATCH (v5 {v: 5}), (v3 {v: 3})
                   WITH v5, v3
                   MATCH p = allShortestPaths((v3)<-[*]-(v5))
                   RETURN nodes(p) AS nodes
                   ORDER BY nodes"""

        actual_result = self.graph.query(query)
        self.env.assertEqual(actual_result.result_set, expected_result)

    def test06_all_shortest_cycle(self):
        # running against following graph
        #
        # (v1)-[:E]->(v2)-[:E]->(v3)-[:E]->(v4)
        # (v1)-[:E]->(v5)-[:E2]->(v4)
        # (v2)-[:E2]->(v4)
        # (v3)-[:E2]->(v1)
        # (v4)-[:E2]->(v1)

        query = """MATCH (v1 {v: 1})
                   WITH v1
                   MATCH p = allShortestPaths((v1)-[*]->(v1))
                   RETURN nodes(p) AS nodes
                   ORDER BY nodes"""

        actual_result = self.cyclic_graph.query(query)
        # 3 paths should be found
        expected_result = [[[self.cyclic_v1, self.cyclic_v3, self.cyclic_v2, self.cyclic_v1]],
                           [[self.cyclic_v1, self.cyclic_v4, self.cyclic_v2, self.cyclic_v1]],
                           [[self.cyclic_v1, self.cyclic_v4, self.cyclic_v5, self.cyclic_v1]]]
        self.env.assertEqual(actual_result.result_set, expected_result)

        # Verify that a right-to-left traversal produces the same results
        query = """MATCH (v1 {v: 1})
                   WITH v1
                   MATCH p = allShortestPaths((v1)<-[*]-(v1))
                   RETURN nodes(p) AS nodes
                   ORDER BY nodes"""

        actual_result = self.cyclic_graph.query(query)
        self.env.assertEqual(actual_result.result_set, expected_result)
