import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

GRAPH_ID = "G"
redis_graph = None

class testPagerankFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)

    def test_pagerank(self):
        # Pagerank considers only nodes of given label and
        # relation of given relationship type
        # multiple edges between two nodes are considered as a single connection.
        queries = [
            # Single Label, single connection.
            "CREATE (a:L {v:1})-[:R]->(b:L {v:2})",
            # Single Label, multi connection.
            "CREATE (a:L {v:1})-[:R]->(b:L {v:2}), (a)-[:R]->(b)",
            # Multi Label, single connection.
            "CREATE (a:L {v:1})-[:R]->(b:L {v:2}), (:X)-[:R]->(:X)",
            # Multi Label, multi connection.
            "CREATE (a:L {v:1})-[:R]->(b:L {v:2}), (a)-[:R]->(b), (:X)-[:R]->(:X)"
        ]

        for q in queries:
            self.env.cmd('flushall')
            redis_graph.query(q)
            q = """CALL algo.pageRank('L', 'R') YIELD node, score RETURN node.v, score"""
            resultset = redis_graph.query(q).result_set

            # 2) 1) 1) (integer) 2
            # 2) "0.777813196182251"
            # 2) 1) (integer) 1
            # 2) "0.22218681871891"
            self.env.assertEqual(len(resultset), 2)
            self.env.assertEqual(resultset[0][0], 2)
            self.env.assertAlmostEqual(resultset[0][1], 0.777813196182251, 0.0001)
            self.env.assertEqual(resultset[1][0], 1)
            self.env.assertAlmostEqual(resultset[1][1], 0.22218681871891, 0.0001)
