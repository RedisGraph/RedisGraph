import os
import sys
import redis
from RLTest import Env
from redisgraph import Graph, Node, Edge

from base import FlowTestsBase

nodes        =  []
edges        =  []
GRAPH_ID     =  "path_projections"
redis_graph  =  None

class testPathProjections(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)
        self.populate_graph()

    def populate_graph(self):
        # Construct a graph with the form:
        # (v1)-[:E]->(v2)-[:E]->(v3)-[:E]->(v4), (v1)-[:E]->(v5)-[:E2]->(v4)

        global nodes
        global edges
        for v in range(1, 6):
            node = Node(label="L", properties={"v": v})
            nodes.append(node)
            redis_graph.add_node(node)

        edge = Edge(nodes[0], "E", nodes[1], properties={"connects": "12"})
        redis_graph.add_edge(edge)
        edges.append(edge)

        edge = Edge(nodes[1], "E", nodes[2], properties={"connects": "23"})
        redis_graph.add_edge(edge)
        edges.append(edge)

        edge = Edge(nodes[2], "E", nodes[3], properties={"connects": "34"})
        redis_graph.add_edge(edge)
        edges.append(edge)

        edge = Edge(nodes[0], "E", nodes[4], properties={"connects": "15"})
        redis_graph.add_edge(edge)
        edges.append(edge)

        edge = Edge(nodes[4], "E2", nodes[3], properties={"connects": "54"})
        redis_graph.add_edge(edge)
        edges.append(edge)

        redis_graph.commit()

    def test01_single_source_projection(self):
        query = """MATCH (a {v: 1}) RETURN a, (a)-[]->() AS path"""
        #  query = """MATCH (a {v: 1}) WITH a, (a)-[]->() AS path RETURN a, nodes(path), relationships(path)"""
        actual_result = redis_graph.query(query)
        # Node 1 should be returned with an array of its two outgoing edges
        #  expected_result = [nodes[0], [edges[0], edges[3]]]
        #  self.env.assertEqual(actual_result.result_set, expected_result)

    #  def test02_multi_source_projection(self):

    #  def test03_multiple_projections(self):
