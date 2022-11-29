from common import *

class test_execution_plan_print():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global graph
        redis_con = self.env.getConnection()
        graph = Graph(redis_con, "g")
        # Create key
        graph.query("RETURN 1")
    
    # conditional traverse after a label_scan should not print
    # the label scanned by the label_scan
    def test01_conditional_traverse(self):
        # empty graph (first test) --> order of traversal stays as in query (A --> B).
        plan = graph.execution_plan("MATCH (n:A:B) RETURN n")

        # label A is traversed first
        self.env.assertIn("Node By Label Scan | (n:A)", plan)
        # conditional traverse prints B only
        self.env.assertIn("Conditional Traverse | (n:B)->(n:B)", plan)
    
    # expand-into should not print the label traversed in
    # a label-scan that occurred before it.
    def test02_expand_into(self):
        plan = graph.execution_plan("MATCH (n:A:B:C) RETURN n")

        # label A is traversed first
        self.env.assertIn("Node By Label Scan | (n:A)", plan)
        # expand_into does not print A
        self.env.assertIn("(n:B:C)", plan)
    
    # after optimize_label_scan operates (switching label-traversal order)
    # printing of conditional-traverse should be updated as well.
    def test03_optimize_label_scan_replace_conditional_traverse(self):
        # create a node with label A, so that B will
        # be the first label scanned after optimization
        graph.query("CREATE (:A)")

        plan = graph.execution_plan("MATCH (n:A:B) RETURN n")

        # make sure optimize_label_scan worked
        self.env.assertIn("Node By Label Scan | (n:B)", plan)

        # make sure that conditional-traverse prints correct label A
        self.env.assertIn("Conditional Traverse | (n:A)->(n:A)", plan)
    
    # after optimize_label_scan operates (switching label-traversal order)
    # printing of expand_into should be updated as well.
    def test04_optimize_label_scan_replace_expand_into(self):
        # create a node with label C, so that B will
        # be the first label scanned after optimization
        # (since nodes with labels A and C exist now, while B doesn't)
        graph.query("CREATE (:C)")

        plan = graph.execution_plan("MATCH (n:A:B:C) RETURN n")
        # make sure optimize_label_scan worked
        self.env.assertIn("Node By Label Scan | (n:B)", plan)
        # make sure that expand-into prints correct labels A and C
        self.env.assertIn("Expand Into | (n:A:C)->(n:A:C)", plan)
    
    # Make sure the conditional-traverse and expand-into operations
    # which do not come after a label_scan print all labels.
    def test05_operations_not_after_scan(self):
        self.env.flush()
        # create key
        graph.query("RETURN 1")

        plan = graph.execution_plan("match p=(n:A:B)-[*]-(m:C:D) RETURN p")

        # A is traversed first
        self.env.assertIn("Node By Label Scan | (n:A)", plan)
        # B is printed alone in conditional-traverse
        self.env.assertIn("Conditional Traverse | (n:B)->(n:B)", plan)
        # conditional-variable-length-traverse prints all labels
        self.env.assertIn("Conditional Variable Length Traverse | (n)-[@anon_0*1..INF]->(m)", plan)
        # expand-into prints labels C and D
        self.env.assertIn("Expand Into | (m:C:D)->(m:C:D)", plan)