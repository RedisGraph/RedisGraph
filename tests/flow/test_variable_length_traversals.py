from common import *

dis_redis = None
redis_graph = None
redis_con = None
node_names = ["A", "B", "C", "D"]

# A can reach 3 nodes, B can reach 2 nodes, C can reach 1 node
max_results = 6


class testVariableLengthTraversals(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_con
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(redis_con, "G")
        self.populate_graph()

    def populate_graph(self):
        global redis_graph

        nodes = []
        # Create nodes
        for n in node_names:
            node = Node(label="node", properties={"name": n})
            redis_graph.add_node(node)
            nodes.append(node)

        # Create edges
        for i in range(len(nodes) - 1):
            edge = Edge(nodes[i], "knows", nodes[i+1], properties={"connects": node_names[i] + node_names[i+1]})
            redis_graph.add_edge(edge)

        redis_graph.commit()

    # Sanity check against single-hop traversal
    def test01_conditional_traverse(self):
        query = """MATCH (a)-[e]->(b) RETURN a.name, e.connects, b.name ORDER BY a.name, b.name"""
        actual_result = redis_graph.query(query)
        expected_result = [['A', 'AB', 'B'],
                           ['B', 'BC', 'C'],
                           ['C', 'CD', 'D']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Traversal with no labels
    def test02_unlabeled_traverse(self):
        query = """MATCH (a)-[*]->(b) RETURN a.name, b.name ORDER BY a.name, b.name"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(len(actual_result.result_set), max_results)

        query = """MATCH (a)<-[*]-(b) RETURN a, b ORDER BY a.name, b.name"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(len(actual_result.result_set), max_results)

    # Traversal with labeled source
    def test03_source_labeled(self):
        query = """MATCH (a:node)-[*]->(b) RETURN a.name, b.name ORDER BY a.name, b.name"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(len(actual_result.result_set), max_results)

        query = """MATCH (a:node)<-[*]-(b) RETURN a.name, b.name ORDER BY a.name, b.name"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(len(actual_result.result_set), max_results)

    # Traversal with labeled dest
    def test04_dest_labeled(self):
        query = """MATCH (a)-[*]->(b:node) RETURN a.name, b.name ORDER BY a.name, b.name"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(len(actual_result.result_set), max_results)

        query = """MATCH (a)<-[*]-(b:node) RETURN a.name, b.name ORDER BY a.name, b.name"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(len(actual_result.result_set), max_results)

    # Attempt to traverse non-existent relationship type.
    def test05_invalid_traversal(self):
        query = """MATCH (a)-[:no_edge*]->(b) RETURN a.name"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(len(actual_result.result_set), 0)

    # Test bidirectional traversal
    def test06_bidirectional_traversal(self):
        query = """MATCH (a)-[*]-(b) RETURN a.name, b.name ORDER BY a.name, b.name"""
        actual_result = redis_graph.query(query)
        # The undirected traversal should represent every combination twice.
        self.env.assertEquals(len(actual_result.result_set), max_results * 2)

    def test07_non_existing_edge_traversal_with_zero_length(self):
        # Verify that zero length traversals always return source, even for non existing edges.
        query = """MATCH (a)-[:not_knows*0..1]->(b) RETURN a"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(len(actual_result.result_set), 4)

    # Test traversal with a possibly-null source.
    def test08_optional_source(self):
        query = """OPTIONAL MATCH (a:fake) OPTIONAL MATCH (a)-[*]->(b) RETURN a.name, b.name ORDER BY a.name, b.name"""
        actual_result = redis_graph.query(query)
        expected_result = [[None, None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """OPTIONAL MATCH (a:node {name: 'A'}) OPTIONAL MATCH (a)-[*]->(b {name: 'B'}) RETURN a.name, b.name ORDER BY a.name, b.name"""
        actual_result = redis_graph.query(query)
        expected_result = [['A', 'B']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Test traversals with filters on variable-length edges
    def test09_filtered_edges(self):
        # Test an inline equality predicate
        query = """MATCH (a)-[* {connects: 'BC'}]->(b) RETURN a.name, b.name ORDER BY a.name, b.name"""
        # The filter op should have been optimized out
        plan = redis_graph.execution_plan(query)
        self.env.assertNotIn("Filter", plan)
        actual_result = redis_graph.query(query)
        expected_result = [['B', 'C']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test a WHERE clause predicate
        query = """MATCH (a)-[e*]->(b) WHERE e.connects IN ['BC', 'CD'] RETURN a.name, b.name ORDER BY a.name, b.name"""
        # The filter op should have been optimized out
        plan = redis_graph.execution_plan(query)
        self.env.assertNotIn("Filter", plan)
        actual_result = redis_graph.query(query)
        expected_result = [['B', 'C'],
                           ['B', 'D'],
                           ['C', 'D']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test a WHERE clause predicate with an OR condition
        query = """MATCH (a)-[e*]->(b) WHERE e.connects = 'BC' OR e.connects = 'CD' RETURN a.name, b.name ORDER BY a.name, b.name"""
        # The filter op should have been optimized out
        plan = redis_graph.execution_plan(query)
        self.env.assertNotIn("Filter", plan)
        actual_result = redis_graph.query(query)
        # Expecting the same result
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test the concatenation of multiple predicates
        query = """MATCH (a)-[e*]->(b) WHERE e.connects IN ['AB', 'BC', 'CD'] AND e.connects <> 'CD' RETURN a.name, b.name ORDER BY a.name, b.name"""
        # The filter op should have been optimized out
        plan = redis_graph.execution_plan(query)
        self.env.assertNotIn("Filter", plan)
        actual_result = redis_graph.query(query)
        expected_result = [['A', 'B'],
                           ['A', 'C'],
                           ['B', 'C']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Test the concatenation of AND and OR conditions
        query = """MATCH (a)-[e*]->(b) WHERE e.connects IN ['AB', 'BC', 'CD'] AND (e.connects = 'AB' OR e.connects = 'BC')  AND e.connects <> 'CD' RETURN a.name, b.name ORDER BY a.name, b.name"""
        # The filter op should have been optimized out
        plan = redis_graph.execution_plan(query)
        self.env.assertNotIn("Filter", plan)
        actual_result = redis_graph.query(query)
        expected_result = [['A', 'B'],
                           ['A', 'C'],
                           ['B', 'C']]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # Validate that WHERE clause predicates are applied to edges lower than the minHops value
        query = """MATCH (a)-[e*2..]->(b) WHERE e.connects <> 'AB' RETURN a.name, b.name ORDER BY a.name, b.name"""
        actual_result = redis_graph.query(query)
        expected_result = [['B', 'D']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Test traversals with filters on variable-length edges in WITH...OPTIONAL MATCH constructs
    def test10_filtered_edges_after_segment_change(self):
        # Test a query that produces the subtree:
        #   Project
        #       Filter
        #           All Node Scan | (a)
        #   Optional
        #       Conditional Variable Length Traverse | (a)-[anon_0*1..INF]->(b)
        #           Argument
        #
        # The scan op and the variable-length traversal and its filter are
        # built in different ExecutionPlan segments. The segments must be
        # updated before cloning the Optional subtree,
        # or else the variable-length edge reference will be lost.
        query = """MATCH (a {name: 'A'}) WITH a OPTIONAL MATCH (a)-[* {connects: 'AB'}]->(b) RETURN a.name, b.name ORDER BY a.name, b.name"""
        actual_result = redis_graph.query(query)
        expected_result = [['A', 'B']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Test range-length edges
    def test11_range_length_edges(self):
        # clear previous data
        conn = self.env.getConnection()
        conn.flushall()

        # populate graph
        # create a graph with 4 nodes
        # a->b
        # b->c
        # c->a
        # d->d
        query = """CREATE (a {v:'a'}), (b {v:'b'}), (c {v:'c'}), (d {v:'d'}),
                          (a)-[:R]->(b), (b)-[:R]->(c), (c)-[:R]->(a), (d)-[:R]->(d)"""

        actual_result = redis_graph.query(query)

        # validation queries
        query_to_expected_result = {
            "MATCH p = (a {v:'a'})-[*2]-(c {v:'c'}) RETURN length(p)" : [[2]],
            "MATCH p = (a {v:'a'})-[*2..]-(c {v:'c'}) RETURN length(p)" : [[2]],
            "MATCH p = (a {v:'a'})-[*2..2]-(c {v:'c'}) RETURN length(p)" : [[2]],
            "MATCH p = (a {v:'a'})-[*]-(c {v:'c'}) WITH length(p) AS len RETURN len ORDER BY len" : [[1],[2]],
            "MATCH p = (a {v:'a'})-[*..]-(c {v:'c'}) WITH length(p) as len RETURN len ORDER BY len" : [[1],[2]],
            "MATCH p = (d {v:'d'})-[*0]-() RETURN length(p)" : [[0]],
        }

        # validate query results
        for query, expected_result in query_to_expected_result.items():
            actual_result = redis_graph.query(query)
            self.env.assertEquals(actual_result.result_set, expected_result)
