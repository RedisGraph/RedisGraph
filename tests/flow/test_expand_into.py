import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge

GRAPH_ID = "G"

# tests different variations of expand-into operation
# the expand-into operation is used when a traversal pattern say
# (a)-[:R]->()-[:R]->(z) has both of it ends already resolved e.g.
# MATCH (a), (z) WITH a,z MATCH (a)-[:R]->()-[:R]->(z) return z

class testExpandInto():
    def __init__(self):
        self.env = Env(decodeResponses=True)

    # test expand into single hop no multi-edge
    # (:A)-[:R]->(:B)
    def test01_single_hop_no_multi_edge(self):
        redis_con = self.env.getConnection()
        graph = Graph(GRAPH_ID, redis_con)
        try:
            graph.delete()
        except:
            pass

        # create graph
        query = "CREATE (:A)-[:R {v:1}]->(:B)"
        graph.query(query)

        # make sure (a) is connected to (b) via a 'R' edge
        query = "MATCH (a:A)-[]->(b:B) WITH a,b MATCH (a)-[:R]->(b) RETURN count(a)"
        plan = graph.execution_plan(query)
        result = graph.query(query)
        self.env.assertIn("Expand Into", plan)
        self.env.assertEquals(1, result.result_set[0][0])

        query = "MATCH (a:A)-[]->(b:B) WITH a,b MATCH (a)-[e:R]->(b) RETURN e.v"
        plan = graph.execution_plan(query)
        result = graph.query(query)
        self.env.assertIn("Expand Into", plan)
        self.env.assertEquals(1, result.result_set[0][0])


    # test expand into single hop multi-edge
    # (:A)-[:R]->(:B), (:A)-[:R]->(:B)
    def test02_single_hop_multi_edge(self):
        redis_con = self.env.getConnection()
        graph = Graph(GRAPH_ID, redis_con)
        graph.delete()

        # create graph
        query = "CREATE (a:A)-[:R {v:1}]->(b:B), (a)-[:R {v:2}]->(b)"
        graph.query(query)

        # make sure (a) is connected to (b) via a 'R' edge
        # there are multiple edges of type 'R' connecting the two
        query = "MATCH (a:A)-[]->(b:B) WITH a,b MATCH (a)-[e:R]->(b) RETURN count(e)"
        plan = graph.execution_plan(query)
        result = graph.query(query)
        self.env.assertIn("Expand Into", plan)
        self.env.assertEquals(2, result.result_set[0][0])

        query = "MATCH (a:A)-[]->(b:B) WITH a,b MATCH (a)-[e:R]->(b) RETURN e.v ORDER BY e.v"
        plan = graph.execution_plan(query)
        result = graph.query(query)
        self.env.assertIn("Expand Into", plan)
        self.env.assertEquals(1, result.result_set[0][0])
        self.env.assertEquals(2, result.result_set[1][0])


    # test expand into multiple hops with no multi-edge
    # (:A)-[:R]->()-[:R]->(:B)
    def test03_multi_hop_no_multi_edge(self):
        redis_con = self.env.getConnection()
        graph = Graph(GRAPH_ID, redis_con)
        graph.delete()

        # create graph
        query = "CREATE (:A)-[:R]->()-[:R]->(:B)"
        graph.query(query)

        # make sure (a) is connected to (b) via a 'R' edge
        # expand-into inspects the result of (F*R*ADJ)[a,b]
        query = "MATCH (a:A)-[*]->(b:B) WITH a,b MATCH (a)-[:R]->()-[]->(b) RETURN count(a)"
        plan = graph.execution_plan(query)
        result = graph.query(query)
        self.env.assertIn("Expand Into", plan)
        self.env.assertEquals(1, result.result_set[0][0])

    # test expand into multiple hops with multi-edge
    # (a:A)-[:R]->(i)-[:R]->(b:B)
    # (a:A)-[:R]->(i)-[:R]->(b:B)
    def test04_multi_hop_multi_edge(self):
        redis_con = self.env.getConnection()
        graph = Graph(GRAPH_ID, redis_con)
        graph.delete()

        # create graph
        query = "CREATE (a:A), (b:B), (i), (a)-[:R]->(i)-[:R]->(b), (a)-[:R]->(i)-[:R]->(b)"
        graph.query(query)

        # make sure (a) is connected to (b) via a 'R' edge
        # there are multiple ways to reach (b) from (a)
        query = "MATCH (a:A)-[*]->(b:B) WITH a,b MATCH (a)-[:R]->()-[]->(b) RETURN count(1)"
        plan = graph.execution_plan(query)
        result = graph.query(query)
        self.env.assertIn("Expand Into", plan)
        self.env.assertEquals(4, result.result_set[0][0])

