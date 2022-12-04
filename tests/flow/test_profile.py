from common import *

GRAPH_ID = "profile"

redis_con = None
redis_graph = None


class testProfile(FlowTestsBase):
    def __init__(self):
        global redis_con
        global redis_graph
        self.env = Env(decodeResponses=True)
        redis_con = self.env.getConnection()
        redis_graph = Graph(redis_con, GRAPH_ID)

    def test01_profile(self):
        q = """UNWIND range(1, 3) AS x CREATE (p:Person {v:x})"""
        profile = redis_con.execute_command("GRAPH.PROFILE", GRAPH_ID, q)
        profile = [x[0:x.index(',')].strip() for x in profile]
            
        self.env.assertIn("Create | Records produced: 3", profile)
        self.env.assertIn("Unwind | Records produced: 3", profile)

        q = "MATCH (p:Person) WHERE p.v > 1 RETURN p"
        profile = redis_con.execute_command("GRAPH.PROFILE", GRAPH_ID, q)
        profile = [x[0:x.index(',')].strip() for x in profile]

        self.env.assertIn("Results | Records produced: 2", profile)
        self.env.assertIn("Project | Records produced: 2", profile)
        self.env.assertIn("Filter | Records produced: 2", profile)
        self.env.assertIn("Node By Label Scan | (p:Person) | Records produced: 3", profile)

    def test02_profile_after_op_reset(self):
        # validate that profile works properly on reset operations
        q = """MATCH (a:L)-[*]->() SET a.v = 5"""
        profile = redis_con.execute_command("GRAPH.PROFILE", GRAPH_ID, q)
        profile = [x[0:x.index(',')].strip() for x in profile]
        self.env.assertIn("Update | Records produced: 0", profile)
        self.env.assertIn("Conditional Variable Length Traverse | (a)-[@anon_1*1..INF]->(@anon_0) | Records produced: 0", profile)
        self.env.assertIn("Node By Label Scan | (a:L) | Records produced: 0", profile)
