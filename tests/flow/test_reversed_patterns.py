import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge

from base import FlowTestsBase

GRAPH_ID = "G"
redis_con = None
redis_graph = None


class testReversedPatterns(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_graph
        global redis_con
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)
        self.populate_graph()

    def populate_graph(self):
        global redis_graph
        if not redis_con.exists(GRAPH_ID):
            # Create entities
            srcNode = Node(label="L", properties={"name": "SRC"})
            destNode = Node(label="L", properties={"name": "DEST"})
            redis_graph.add_node(srcNode)
            redis_graph.add_node(destNode)
            edge = Edge(srcNode, 'E', destNode)
            redis_graph.add_edge(edge)
            redis_graph.commit()

    # Verify that edges are not modified after entity deletion
    def test01_reversed_pattern(self):
        leftToRight = """MATCH (a:L)-[b]->(c:L) RETURN a, TYPE(b), c"""
        rightToLeft = """MATCH (c:L)<-[b]-(a:L) RETURN a, TYPE(b), c"""
        leftToRightResult = redis_graph.query(leftToRight)
        rightToLeftResult = redis_graph.query(rightToLeft)
        self.env.assertEquals(leftToRightResult.result_set, rightToLeftResult.result_set)