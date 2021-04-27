import os
from RLTest import Env
from redisgraph import Graph

graph = None

class testTraversalConstruction():
    def __init__(self):
        global graph

        self.env = Env(decodeResponses=True)
        redis_con = self.env.getConnection()
        graph = Graph("TraversalConstruction", redis_con)

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

