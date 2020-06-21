import redis
from RLTest import Env
from redisgraph import Graph, Node, Edge
from base import FlowTestsBase

GRAPH_ID = "union_test"
redis_graph = None

class testUnion(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)
        self.populate_graph()

    def populate_graph(self):
        global redis_graph
        # Construct a graph with the form:
        # (v1)-[:E1]->(v2)-[:E2]->(v3)
        node_props = ['v1', 'v2', 'v3']

        nodes = {}
        for idx, v in enumerate(node_props):
            node = Node(label="L", properties={"v": v})
            nodes[v] = node
            redis_graph.add_node(node)

        edge = Edge(nodes['v1'], "E1", nodes['v2'], properties={"v": "v1_v2"})
        redis_graph.add_edge(edge)

        edge = Edge(nodes['v2'], "E2", nodes['v3'], properties={"v": "v2_v3"})
        redis_graph.add_edge(edge)

        redis_graph.flush()

    def test01_union(self):
        q = """RETURN 1 as one UNION ALL RETURN 1 as one"""
        result = redis_graph.query(q)
        # Expecting 2 identical records.
        self.env.assertEquals(len(result.result_set), 2)
        expected_result = [[1],
                           [1]]
        self.env.assertEquals(result.result_set, expected_result)

        q = """RETURN 1 as one UNION RETURN 1 as one"""
        result = redis_graph.query(q)
        # Expecting a single record, duplicate removed.
        self.env.assertEquals(len(result.result_set), 1)
        expected_result = [[1]]
        self.env.assertEquals(result.result_set, expected_result)

        q = """MATCH a = () return length(a) AS len UNION ALL MATCH b = () RETURN length(b) AS len"""
        result = redis_graph.query(q)
        # 3 records from each sub-query, coresponding to each path matched.
        self.env.assertEquals(len(result.result_set), 6)

    def test02_invalid_union(self):
        try:
            # projection must be exactly the same.
            q = """RETURN 1 as one UNION RETURN 1 as two"""
            redis_graph.query(q)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    # Performing UNION with the same left and right side should
    # produce the same result as evaluating just one side.
    def test03_union_deduplication(self):
        non_union_query = """MATCH (a)-[]->(b) RETURN a.v, b.v ORDER BY a.v, b.v"""
        non_union_result = redis_graph.query(non_union_query)

        union_query = """MATCH (a)-[]->(b) RETURN a.v, b.v ORDER BY a.v, b.v
                         UNION
                         MATCH (a)-[]->(b) RETURN a.v, b.v ORDER BY a.v, b.v"""
        union_result = redis_graph.query(union_query)
        self.env.assertEquals(union_result.result_set, non_union_result.result_set)

    # A syntax error should be raised on edge alias reuse in one side of a union.
    def test04_union_invalid_reused_edge(self):
        try:
            query = """MATCH ()-[e]->()-[e]->() RETURN e
                       UNION
                       MATCH ()-[e]->() RETURN e"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    # An edge alias appearing on both sides of a UNION is expected.
    def test05_union_valid_reused_edge(self):
        query = """MATCH ()-[e]->() RETURN e.v ORDER BY e.v
                   UNION
                   MATCH ()-[e]->() RETURN e.v ORDER BY e.v
                   UNION
                   MATCH ()-[e]->() RETURN e.v ORDER BY e.v"""
        result = redis_graph.query(query)

        expected_result = [["v1_v2"],
                           ["v2_v3"]]
        self.env.assertEquals(result.result_set, expected_result)

    # Union should be capable of collating nodes and edges in a single column.
    def test06_union_nodes_with_edges(self):
        query = """MATCH ()-[e]->() RETURN e
                   UNION
                   MATCH (e) RETURN e"""
        union_result = redis_graph.query(query)

        # All 3 nodes and 2 edges should be returned.
        self.env.assertEquals(len(union_result.result_set), 5)

        query = """MATCH ()-[e]->() RETURN e
                   UNION ALL
                   MATCH (e) RETURN e"""
        union_all_result = redis_graph.query(query)

        # The same results should be produced regardless of whether ALL is specified.
        self.env.assertEquals(union_result.result_set, union_all_result.result_set)
