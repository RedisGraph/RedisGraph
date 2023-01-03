from common import *
from index_utils import *

graph = None


class testTraversalConstruction():
    def __init__(self):
        global graph

        self.env = Env(decodeResponses=True)
        redis_con = self.env.getConnection()
        graph = Graph(redis_con, "TraversalConstruction")
        # Create the graph
        graph.query("RETURN 1")

    # Test differing starting points for the same search pattern
    def test_starting_point(self):
        # Neither the source nor the destination are labeled
        # perform an AllNodeScan from the source node.
        query = """MATCH (a)-[]->(b) RETURN a, b"""
        plan = graph.execution_plan(query)
        self.env.assertIn("All Node Scan | (a)", plan)

        # Destination is labeled, perform a LabelScan from the destination node.
        query = """MATCH (a)-[]->(b:B) RETURN a, b"""
        plan = graph.execution_plan(query)
        self.env.assertIn("Node By Label Scan | (b:B)", plan)

        # Destination is filtered, perform an AllNodeScan from the destination node.
        query = """MATCH (a)-[]->(b) WHERE b.v = 2 RETURN a, b"""
        plan = graph.execution_plan(query)
        self.env.assertIn("All Node Scan | (b)", plan)

        # Destination is labeled but source is filtered, perform an AllNodeScan from the source node.
        query = """MATCH (a)-[]->(b:B) WHERE a.v = 1 OR a.v = 3 RETURN a, b"""
        plan = graph.execution_plan(query)
        self.env.assertIn("All Node Scan | (a)", plan)

        # Both are labeled and source is filtered, perform a LabelScan from the source node.
        query = """MATCH (a:A)-[]->(b:B) WHERE a.v = 3 RETURN a, b"""
        plan = graph.execution_plan(query)
        self.env.assertIn("Node By Label Scan | (a:A)", plan)

        # Both are labeled and dest is filtered, perform a LabelScan from the dest node.
        query = """MATCH (a:A)-[]->(b:B) WHERE b.v = 2 RETURN a, b"""
        plan = graph.execution_plan(query)
        self.env.assertIn("Node By Label Scan | (b:B)", plan)

    # make sure traversal begins with labeled entity
    def test_start_with_label(self):
        queries = ["MATCH (A:L)-->(B)-->(C) RETURN 1",
                   # "MATCH (A)-->(B:L)-->(C) RETURN 1", # improve on this case
                   "MATCH (A)-->(B)-->(C:L) RETURN 1"]

        for q in queries:
            plan = graph.execution_plan(q)
            ops = plan.split(os.linesep)
            ops.reverse()
            self.env.assertTrue("Node By Label Scan" in ops[0])

    # make sure traversal begins with filtered entity
    def test_start_with_filter(self):
        # MATCH (A)-->(B)-->(C) WHERE A.val = 1 RETURN *
        # MATCH (A)-->(B)-->(C) WHERE B.val = 1 RETURN *
        # MATCH (A)-->(B)-->(C) WHERE C.val = 1 RETURN *
        entities = ['A', 'B', 'C']
        for e in entities:
            q = """MATCH (A)-->(B)-->(C) WHERE {}.val = 1 RETURN *""".format(e)
            plan = graph.execution_plan(q)
            ops = plan.split(os.linesep)
            ops.reverse()

            self.env.assertTrue("All Node Scan | ({})".format(e) in ops[0])
            self.env.assertTrue("Filter" in ops[1])

    # make sure traversal begins with bound entity
    def test_start_with_bound(self):
        # MATCH (X) WITH X as A MATCH (A)-->(B)-->(C) RETURN *
        # MATCH (X) WITH X as B MATCH (A)-->(B)-->(C) RETURN *
        # MATCH (X) WITH X as C MATCH (A)-->(B)-->(C) RETURN *
        entities = ['A', 'B', 'C']
        for e in entities:
            q = "MATCH (X) WITH X as {} MATCH (A)-->(B)-->(C) RETURN *".format(e)
            plan = graph.execution_plan(q)
            ops = plan.split(os.linesep)
            ops.reverse()
            self.env.assertTrue("Conditional Traverse | ({}".format(e) in ops[2])

    # make sure traversal begins with bound entity and follows with filter
    def test_start_with_bound_follows_with_filter(self):
        queries = ["MATCH (X) WITH X AS B MATCH (A {v:1})-->(B)-->(C) RETURN *",
                "MATCH (X) WITH X AS B MATCH (A)-->(B)-->(C {v:1}) RETURN *"]
        for q in queries:
            plan = graph.execution_plan(q)
            ops = plan.split(os.linesep)
            ops.reverse()
            self.env.assertTrue("Filter" in ops[3])

    def test_filter_as_early_as_possible(self):
        q = """MATCH (A:L {v: 1})-->(B)-->(C), (B)-->(D:L {v: 1}) RETURN 1"""
        plan = graph.execution_plan(q)
        ops = plan.split(os.linesep)
        ops.reverse()
        self.env.assertTrue("Node By Label Scan" in ops[0]) # scan either A or D
        self.env.assertTrue("Filter" in ops[1]) # filter either A or D
        self.env.assertTrue("Conditional Traverse" in ops[2]) # traverse from A to D or from D to A
        self.env.assertTrue("Conditional Traverse" in ops[3]) # traverse from A to D or from D to A
        self.env.assertTrue("Filter" in ops[4]) # filter either A or D

    def test_long_pattern(self):
        q = """match (a)--(b)--(c)--(d)--(e)--(f)--(g)--(h)--(i)--(j)--(k)--(l) return *"""
        plan = graph.execution_plan(q)
        ops = plan.split(os.linesep)
        self.env.assertEqual(len(ops), 14)

    def test_start_with_index_filter(self):
        # TODO: enable this test, once we'll score higher filters that
        # have the potential turn into index scan
        return

        create_node_exact_match_index(graph, 'L', 'v', sync=True)

        q = """MATCH (a:L {v:1})-[]-(b:L {x:1}) RETURN a, b"""
        plan = graph.execution_plan(q)
        ops = plan.split(os.linesep)
        ops.reverse()
        self.env.assertTrue("Index Scan" in ops[0]) # start with index scan

        q = """MATCH (a:L {x:1})-[]-(b:L {v:1}) RETURN a, b"""
        plan = graph.execution_plan(q)
        ops = plan.split(os.linesep)
        ops.reverse()
        self.env.assertTrue("Index Scan" in ops[0]) # start with index scan

    def test_variable_length_traversal_placement(self):
        # cyclic traversal followed by variable-length traversal
        q = """MATCH (b)<-[*]-(a:L {v: 5})<--(a) WHERE b.v = 10 RETURN a"""
        plan = graph.execution_plan(q)
        ops = plan.split(os.linesep)
        ops.reverse()
        self.env.assertTrue("Node By Label Scan | (a:L)" in ops[0]) # scan A
        self.env.assertTrue("Filter" in ops[1]) # filter A
        self.env.assertTrue("Expand Into" in ops[2]) # traverse from A to itself
        self.env.assertTrue("Conditional Variable Length Traverse" in ops[3]) # var-len traverse from A to B

        # bidirectional variable-length traversal
        q = """MATCH (a:L {v: 5})-[*]-(b:L) WHERE a <> b RETURN b"""
        plan = graph.execution_plan(q)
        ops = plan.split(os.linesep)
        ops.reverse()
        self.env.assertTrue("Node By Label Scan | (a:L)" in ops[0]) # scan A
        self.env.assertTrue("Filter" in ops[1]) # filter A
        self.env.assertTrue("Conditional Variable Length Traverse" in ops[2]) # bidirectional var-len traverse from A to B
