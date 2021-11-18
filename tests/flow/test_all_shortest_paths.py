import redis
from RLTest import Env
from redisgraph import Graph, Node, Edge

from base import FlowTestsBase

nodes        =  []
GRAPH_ID     =  "all_shortest_paths"
redis_graph  =  None

class testAllShortestPaths(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)
        self.populate_graph()

    def populate_graph(self):
        # Construct a graph with the form:
        # (v1)-[:E]->(v2)-[:E]->(v3)-[:E]->(v4), (v1)-[:E]->(v5)-[:E2]->(v4), (v2)-[:E]->(v4)

        global nodes
        for v in range(1, 6):
            node = Node(label="L", properties={"v": v})
            nodes.append(node)
            redis_graph.add_node(node)

        edge = Edge(nodes[0], "E", nodes[1], properties={"weight": 1})
        redis_graph.add_edge(edge)

        edge = Edge(nodes[1], "E", nodes[2], properties={"weight": 1})
        redis_graph.add_edge(edge)

        edge = Edge(nodes[2], "E", nodes[3], properties={"weight": 1})
        redis_graph.add_edge(edge)

        edge = Edge(nodes[0], "E", nodes[4], properties={"weight": 1})
        redis_graph.add_edge(edge)

        edge = Edge(nodes[4], "E2", nodes[3], properties={"weight": 1})
        redis_graph.add_edge(edge)

        edge = Edge(nodes[1], "E2", nodes[3], properties={"weight": 2})
        redis_graph.add_edge(edge)

        redis_graph.commit()

    def test01_invalid_shortest_paths(self):
        query = """MATCH (a {v: 1}), (b {v: 4}), p = shortestPath((a)-[*]->(b)) RETURN p"""
        try:
            redis_graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertIn("RedisGraph currently only supports shortestPath in WITH or RETURN clauses", str(e))

    def test02_all_shortest_paths(self):
        query = """MATCH (a {v: 1}), (b {v: 4}) WITH a, b MATCH p = allShortestPaths((a)-[*]->(b)) RETURN nodes(p) AS nodes ORDER BY nodes"""
        actual_result = redis_graph.query(query)
        # The 2 2-hop traversals should be found
        expected_result = [[[nodes[0], nodes[1], nodes[3]]],
                           [[nodes[0], nodes[4], nodes[3]]]]
        self.env.assertEqual(actual_result.result_set, expected_result)
        # Verify that a right-to-left traversal produces the same results
        query = """MATCH (a {v: 1}), (b {v: 4}) WITH a, b MATCH p = allShortestPaths((b)<-[*]-(a)) RETURN nodes(p) AS nodes ORDER BY nodes"""
        self.env.assertEqual(actual_result.result_set, expected_result)

    def test03_all_shortest_min_hops(self):
        query = """MATCH (a {v: 1}), (b {v: 4}) WITH a, b MATCH p = allShortestPaths((a)-[*3..]->(b)) RETURN nodes(p) AS nodes ORDER BY nodes"""
        actual_result = redis_graph.query(query)
        # The 1 3-hop traversal should be found
        expected_result = [[[nodes[0], nodes[1], nodes[2], nodes[3]]]]
        self.env.assertEqual(actual_result.result_set, expected_result)
        # Verify that a right-to-left traversal produces the same results
        query = """MATCH (a {v: 1}), (b {v: 4}) WITH a, b MATCH p = allShortestPaths((b)<-[*3..]-(a)) RETURN nodes(p) AS nodes ORDER BY nodes"""
        self.env.assertEqual(actual_result.result_set, expected_result)

    def test04_all_shortest_edge_filter(self):
        query = """MATCH (a {v: 1}), (b {v: 4}) WITH a, b MATCH p = allShortestPaths((a)-[* {weight: 1}]->(b)) RETURN nodes(p) AS nodes ORDER BY nodes"""
        actual_result = redis_graph.query(query)
        # The 1 2-hop traversal with all edge weights of 1 should be found
        expected_result = [[[nodes[0], nodes[4], nodes[3]]]]
        self.env.assertEqual(actual_result.result_set, expected_result)
        # Verify that a right-to-left traversal produces the same results
        query = """MATCH (a {v: 1}), (b {v: 4}) WITH a, b MATCH p = allShortestPaths((b)<-[* {weight: 1}]-(a)) RETURN nodes(p) AS nodes ORDER BY nodes"""
        self.env.assertEqual(actual_result.result_set, expected_result)
