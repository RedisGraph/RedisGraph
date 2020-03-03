import os
import sys
from redisgraph import Graph, Node, Edge, Path
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

GRAPH_ID = "p"
redis_con = None
redis_graph = None

class testProfile(FlowTestsBase):
    def __init__(self):
        global redis_con
        global redis_graph
        super(testProfile, self).__init__()
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)

    def test_profile(self):
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
