import os

def test_start_with_filter(self):
    # MATCH (A)-->(B)-->(C) WHERE A.val = 1 RETURN *
    # MATCH (A)-->(B)-->(C) WHERE B.val = 1 RETURN *
    # MATCH (A)-->(B)-->(C) WHERE C.val = 1 RETURN *
    entities = ['A', 'B', 'C']
    for e in entities:
        q = """MATCH (A)-[]->(B)-[]->(C) WHERE {}.val = 1 RETURN *""".format(e)
        plan = graph.explain(q)
        ops = plan.split(os.linesep)

        ASSERT_TRUE("All Node Scan | ({})".format(e) in ops[0])
        ASSERT_TRUE("Filter" in ops[1])

def test_start_with_label(self):
    queries = ["MATCH (A:L)-->(B)-->(C) RETURN 1",
            "MATCH (A)-->(B:L)-->(C) RETURN 1",
            "MATCH (A)-->(B)-->(C:L) RETURN 1"]

    for q in queries:
        plan = graph.plan(q)
        ops = plan.split(os.linesep)
        ASSERT_TRUE("Node By Label Scan" in op[0])

def test_filter_as_early_as_possible(self):
    q = """MATCH (A:L {v: 1})-->(B)-->(C), (B)-->(D:L {v: 1}) RETURN 1"""
    plan = graph.explain(q)
    ops = plan.split(os.linesep)
    ASSERT_TRUE("Node By Label Scan" in op[0]) # scan either A or D
    ASSERT_TRUE("Filter" in op[1]) # filter either A or D
    ASSERT_TRUE("Conditional Traverse" in op[2]) # traverse from A to D or from D to A
    ASSERT_TRUE("Conditional Traverse" in op[3]) # traverse from A to D or from D to A
    ASSERT_TRUE("Filter" in op[4]) # filter either A or D

def test_start_withbound(self):
"MATCH (A) WITH A as A MATCH (A)-->(B)-->(C:L {v: 1})-->(F), (A)-->(B)-->(D {v: 1})-->(E {v: 1})-->(F) RETURN *"

