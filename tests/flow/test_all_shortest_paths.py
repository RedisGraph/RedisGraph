import redis
from RLTest import Env
from redisgraph import Graph, Node, Edge

GRAPH_ID     =  "all_shortest_paths"

class testAllShortestPaths():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        redis_con = self.env.getConnection()
        self.redis_graph = Graph(GRAPH_ID, redis_con)
        self.populate_graph()

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

        self.redis_graph.add_node(self.v1)
        self.redis_graph.add_node(self.v2)
        self.redis_graph.add_node(self.v3)
        self.redis_graph.add_node(self.v4)
        self.redis_graph.add_node(self.v5)

        e = Edge(self.v1, "E", self.v2, properties={"weight": 1})
        self.redis_graph.add_edge(e)

        e = Edge(self.v2, "E", self.v3, properties={"weight": 1})
        self.redis_graph.add_edge(e)

        e = Edge(self.v3, "E", self.v4, properties={"weight": 1})
        self.redis_graph.add_edge(e)

        e = Edge(self.v1, "E", self.v5, properties={"weight": 1})
        self.redis_graph.add_edge(e)

        e = Edge(self.v5, "E2", self.v4, properties={"weight": 1})
        self.redis_graph.add_edge(e)

        e = Edge(self.v2, "E2", self.v4, properties={"weight": 2})
        self.redis_graph.add_edge(e)

        self.redis_graph.flush()

    def test01_invalid_shortest_paths(self):
        # running against following graph
        #
        # (v1)-[:E]->(v2)-[:E]->(v3)-[:E]->(v4)
        # (v1)-[:E]->(v5)-[:E2]->(v4)
        # (v2)-[:E2]->(v4)

        query = """MATCH (v1 {v: 1}), (v4 {v: 4}), p = shortestPath((v1)-[*]->(v4))
                   RETURN p"""
        try:
            self.redis_graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertIn("RedisGraph currently only supports shortestPath in WITH or RETURN clauses", str(e))

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

        actual_result = self.redis_graph.query(query)
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

        self.env.assertEqual(actual_result.result_set, expected_result)

    def test03_all_shortest_min_hops(self):
        # running against following graph
        #
        # (v1)-[:E]->(v2)-[:E]->(v3)-[:E]->(v4)
        # (v1)-[:E]->(v5)-[:E2]->(v4)
        # (v2)-[:E2]->(v4)

        query = """MATCH (v1 {v: 1}), (v4 {v: 4})
                   WITH v1, v4
                   MATCH p = allShortestPaths((v1)-[*3..]->(v4))
                   RETURN nodes(p) AS nodes
                   ORDER BY nodes"""

        actual_result = self.redis_graph.query(query)
        # The single 3-hop traversal should be found
        expected_result = [[[self.v1, self.v2, self.v3, self.v4]]]
        self.env.assertEqual(actual_result.result_set, expected_result)

        # Verify that a right-to-left traversal produces the same results
        query = """MATCH (v1 {v: 1}), (v4 {v: 4})
                   WITH v1, v4
                   MATCH p = allShortestPaths((v4)<-[*3..]-(v1))
                   RETURN nodes(p) AS nodes
                   ORDER BY nodes"""
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
        actual_result = self.redis_graph.query(query)
        # The 1 2-hop traversal with all edge weights of 1 should be found
        expected_result = [[[self.v1, self.v5, self.v4]]]
        self.env.assertEqual(actual_result.result_set, expected_result)

        # Verify that a right-to-left traversal produces the same results
        query = """MATCH (v1 {v: 1}), (v4 {v: 4})
                   WITH v1, v4
                   MATCH p = allShortestPaths((v4)<-[* {weight: 1}]-(v1))
                   RETURN nodes(p) AS nodes
                   ORDER BY nodes"""
        self.env.assertEqual(actual_result.result_set, expected_result)

