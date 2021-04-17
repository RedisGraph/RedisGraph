import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge

graph = None

class testCDLP():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global graph
        redis_con = self.env.getConnection()
        graph = Graph("proc_CDLP", redis_con)
        self.populate_graph()

    def populate_graph(self):
        # Construct the graph described in D.2(a) of the Graphalytics spec
        # https://ldbc.github.io/ldbc_graphalytics_docs/graphalytics_spec.pdf
        a = Node(label="A", properties={"v": 1})
        b = Node(label="A", properties={"v": 2})
        c = Node(label="A", properties={"v": 3})
        d = Node(label="B", properties={"v": 4})
        e = Node(label="B", properties={"v": 5})
        f = Node(label="B", properties={"v": 6})
        g = Node(label="B", properties={"v": 7})
        h = Node(label="C", properties={"v": 8})

        graph.add_node(a)
        graph.add_node(b)
        graph.add_node(c)
        graph.add_node(d)
        graph.add_node(e)
        graph.add_node(f)
        graph.add_node(g)
        graph.add_node(h)

        graph.add_edge(Edge(a, "E1", b, properties={"v": '12'}))
        graph.add_edge(Edge(a, "E1", c, properties={"v": '13'}))
        graph.add_edge(Edge(a, "E1", c, properties={"v": '17'}))
        graph.add_edge(Edge(b, "E1", a, properties={"v": '21'}))
        graph.add_edge(Edge(b, "E1", c, properties={"v": '23'}))
        graph.add_edge(Edge(c, "E1", a, properties={"v": '31'}))
        graph.add_edge(Edge(c, "E1", b, properties={"v": '32'}))
        graph.add_edge(Edge(d, "E2", e, properties={"v": '45'}))
        graph.add_edge(Edge(d, "E2", f, properties={"v": '46'}))
        graph.add_edge(Edge(e, "E2", d, properties={"v": '54'}))
        graph.add_edge(Edge(e, "E2", f, properties={"v": '56'}))
        graph.add_edge(Edge(e, "E2", g, properties={"v": '57'}))
        graph.add_edge(Edge(f, "E2", e, properties={"v": '65'}))
        graph.add_edge(Edge(f, "E2", g, properties={"v": '67'}))
        graph.add_edge(Edge(g, "E2", e, properties={"v": '75'}))
        graph.add_edge(Edge(g, "E2", f, properties={"v": '76'}))
        graph.add_edge(Edge(g, "E3", h, properties={"v": '78'}))
        graph.add_edge(Edge(h, "E3", f, properties={"v": '86'}))

        graph.flush()

    def test01_cdlp_unbounded(self):
        global graph
        # Nodes 1-3 are part of community 0 while nodes 4-8 are part of community 3
        query = """CALL algo.labelPropagation(0, NULL, NULL) YIELD node, community_id RETURN community_id, node.v ORDER BY community_id, node.v"""
        actual_result = graph.query(query)
        expected_result = [[0, 1],
                           [0, 2],
                           [0, 3],
                           [3, 4],
                           [3, 5],
                           [3, 6],
                           [3, 7],
                           [3, 8]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test02_cdlp_max_iters(self):
        global graph
        # Limiting max iterations to 5 as described in the Graphalytics spec causes node 4 to belong to a separate community.
        query = """CALL algo.labelPropagation(5, NULL, NULL) YIELD node, community_id RETURN community_id, node.v ORDER BY community_id, node.v"""
        actual_result = graph.query(query)
        expected_result = [[0, 1],
                           [0, 2],
                           [0, 3],
                           [3, 5],
                           [3, 6],
                           [3, 7],
                           [3, 8],
                           [4, 4]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test03_cdlp_filter_labels(self):
        global graph
        # Only consider nodes with label A (i.e. the 3-node community)
        query = """CALL algo.labelPropagation(5, NULL, ["A"]) YIELD node, community_id RETURN community_id, node.v ORDER BY community_id, node.v"""
        actual_result = graph.query(query)
        expected_result = [[1, 1],
                           [1, 2],
                           [1, 3]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Consider nodes with labels A or B (i.e. all nodes but 8)
        query = """CALL algo.labelPropagation(0, NULL, ["A", "B"]) YIELD node, community_id RETURN community_id, node.v ORDER BY community_id, node.v"""
        actual_result = graph.query(query)
        expected_result = [[1, 1],
                           [1, 2],
                           [1, 3],
                           [3, 4],
                           [3, 7],
                           [4, 5],
                           [4, 6]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Consider nodes with all labels
        query = """CALL algo.labelPropagation(0, NULL, ["A", "B", "C"]) YIELD node, community_id RETURN community_id, node.v ORDER BY community_id, node.v"""
        actual_result = graph.query(query)
        expected_result = [[1, 1],
                           [1, 2],
                           [1, 3],
                           [3, 4],
                           [3, 5],
                           [3, 6],
                           [3, 7],
                           [3, 8]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test4_cdlp_filter_edges(self):
        global graph
        # Only consider edges with type E1 (i.e. the 3-node community)
        # The result should be one 3-node community and separate communities for all other nodes
        query = """CALL algo.labelPropagation(0, ["E1"], NULL) YIELD node, community_id RETURN community_id, node.v ORDER BY community_id, node.v"""
        actual_result = graph.query(query)
        expected_result = [[0, 1],
                           [0, 2],
                           [0, 3],
                           [3, 4],
                           [4, 5],
                           [5, 6],
                           [6, 7],
                           [7, 8]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Consider edges with type E1 or E2 (i.e. all edges except those connecting 8)
        query = """CALL algo.labelPropagation(0, ["E1", "E2"], NULL) YIELD node, community_id RETURN community_id, node.v ORDER BY community_id, node.v"""
        actual_result = graph.query(query)
        expected_result = [[0, 1],
                           [0, 2],
                           [0, 3],
                           [3, 4],
                           [3, 7],
                           [4, 5],
                           [4, 6],
                           [7, 8]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        #  # Consider edges with all types
        query = """CALL algo.labelPropagation(0, ["E1", "E2", "E3"], NULL) YIELD node, community_id RETURN community_id, node.v ORDER BY community_id, node.v"""
        actual_result = graph.query(query)
        expected_result = [[0, 1],
                           [0, 2],
                           [0, 3],
                           [3, 4],
                           [3, 5],
                           [3, 6],
                           [3, 7],
                           [3, 8]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test5_cdlp_filter_labels_and_edges(self):
        global graph
        # Only consider nodes with label A and edges with type E1 (i.e. the 3-node community)
        # The result should be one 3-node community
        query = """CALL algo.labelPropagation(0, ["E1"], ["A"]) YIELD node, community_id RETURN community_id, node.v ORDER BY community_id, node.v"""
        actual_result = graph.query(query)
        expected_result = [[1, 1],
                           [1, 2],
                           [1, 3]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Consider edges with type E1 or E2 and nodes with label A or B (i.e. all nodes except 8 and all edges except those connecting 8)
        query = """CALL algo.labelPropagation(0, ["E1", "E2"], ["A", "B"]) YIELD node, community_id RETURN community_id, node.v ORDER BY community_id, node.v"""
        actual_result = graph.query(query)
        expected_result = [[1, 1],
                           [1, 2],
                           [1, 3],
                           [3, 4],
                           [3, 7],
                           [4, 5],
                           [4, 6]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Consider all labels and all types
        query = """CALL algo.labelPropagation(0, ["E1", "E2", "E3"], ["A", "B", "C"]) YIELD node, community_id RETURN community_id, node.v ORDER BY community_id, node.v"""
        actual_result = graph.query(query)
        expected_result = [[1, 1],
                           [1, 2],
                           [1, 3],
                           [3, 4],
                           [3, 5],
                           [3, 6],
                           [3, 7],
                           [3, 8]]
        self.env.assertEquals(actual_result.result_set, expected_result)

