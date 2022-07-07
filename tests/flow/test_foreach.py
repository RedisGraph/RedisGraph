from common import *

class testForeach():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        self.redis_con = self.env.getConnection()
        self.g = Graph(self.redis_con, "g")

    def test01_foreach(self):
        q = "FOREACH (i IN range(1, 10) | CREATE (:N {v: i}))"
        res = self.g.explain(q)
        self.env.assertEquals(res.structured_plan.name, "Create")
        self.env.assertEquals(res.structured_plan.children[0].name, "Unwind")
        res = self.g.query(q)
        self.env.assertEquals(res.nodes_created, 10)
        self.env.assertEquals(res.result_set, [])

        q = "FOREACH (i IN range(1, 10) | CREATE (:N {v: i}) CREATE (:N {v: i}))"
        res = self.g.explain(q)
        self.env.assertEquals(res.structured_plan.name, "Create")
        self.env.assertEquals(res.structured_plan.children[0].name, "Unwind")
        res = self.g.query(q)
        self.env.assertEquals(res.nodes_created, 20)
        self.env.assertEquals(res.result_set, [])

        # q = """FOREACH (i IN range(1, 10) | CREATE (:N {v: i}) CREATE (:N {v: i}))
        #        FOREACH (i IN range(1, 10) | CREATE (:M {v: i}) CREATE (:M {v: i}))"""
        # res = self.g.explain(q)
        # self.env.assertEquals(res.structured_plan.name, "Create")
        # self.env.assertEquals(res.structured_plan.children[0].name, "Unwind")
        # res = self.g.query(q)
        # self.env.assertEquals(res.nodes_created, 40)
        # self.env.assertEquals(res.result_set, [])

        q = "FOREACH (i IN range(1, 10) | FOREACH (j IN range(1, 10) | CREATE (:N {vi: i, vj: j})))"
        res = self.g.explain(q)
        self.env.assertEquals(res.structured_plan.name, "Create")
        self.env.assertEquals(res.structured_plan.children[0].name, "Unwind")
        self.env.assertEquals(res.structured_plan.children[0].children[0].name, "Unwind")
        res = self.g.query(q)
        self.env.assertEquals(res.nodes_created, 100)
        self.env.assertEquals(res.result_set, [])

        self.redis_con.flushall()

        res = self.g.query("CREATE (), ()")
        q = "MATCH (n) FOREACH (i IN range(1, 10) | CREATE (m:N {v: i}))"
        res = self.g.explain(q)
        self.env.assertEquals(res.structured_plan.name, "Create")
        self.env.assertEquals(res.structured_plan.children[0].name, "Unwind")
        self.env.assertEquals(res.structured_plan.children[0].children[0].name, "All Node Scan")
        res = self.g.query(q)
        self.env.assertEquals(res.nodes_created, 20)
        self.env.assertEquals(res.result_set, [])

        self.redis_con.flushall()

        # res = self.g.query("CREATE (), ()")
        # q = "MATCH (n) FOREACH (i IN range(1, 10) | CREATE (m:N {v: i})) RETURN n.v"
        # res = self.g.explain(q)
        # self.env.assertEquals(res.structured_plan.name, "Results")
        # self.env.assertEquals(res.structured_plan.children[0].name, "Project")
        # self.env.assertEquals(res.structured_plan.children[0].children[0].name, "Create")
        # self.env.assertEquals(res.structured_plan.children[0].children[0].children[0].name, "Unwind")
        # self.env.assertEquals(res.structured_plan.children[0].children[0].children[0].children[0].name, "All Node Scan")
        # res = self.g.query(q)
        # self.env.assertEquals(res.nodes_created, 20)
        # self.env.assertEquals(res.result_set, [])
