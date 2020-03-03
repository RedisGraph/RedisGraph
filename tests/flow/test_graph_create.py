import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge

from base import FlowTestsBase

GRAPH_ID = "G"
redis_graph = None

class testGraphCreationFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)

    def test01_create_return(self):
        query = """CREATE (a:person {name:'A'}), (b:person {name:'B'})"""
        result = redis_graph.query(query)
        self.env.assertEquals(result.nodes_created, 2)

        query = """MATCH (src:person) CREATE (src)-[e:knows]->(dest {name:'C'}) RETURN src,e,dest ORDER BY ID(src) DESC LIMIT 1"""
        result = redis_graph.query(query)
        self.env.assertEquals(result.nodes_created, 2)
        self.env.assertEquals(result.relationships_created, 2)
        self.env.assertEquals(len(result.result_set), 1)
        self.env.assertEquals(result.result_set[0][0].properties['name'], 'B')

    def test02_create_from_prop(self):
        query = """MATCH (p:person)-[e:knows]->() CREATE (c:clone {doublename: p.name + toLower(p.name), source_of: TYPE(e)}) RETURN c.doublename, c.source_of ORDER BY c.doublename"""
        result = redis_graph.query(query)
        expected_result = [['Aa', 'knows'], ['Bb', 'knows']]

        self.env.assertEquals(result.labels_added, 1)
        self.env.assertEquals(result.nodes_created, 2)
        self.env.assertEquals(result.properties_set, 4)
        self.env.assertEquals(result.result_set, expected_result)

    def test03_create_from_projection(self):
        query = """UNWIND [10,20,30] AS x CREATE (p:person {age:x}) RETURN p.age ORDER BY p.age"""
        result = redis_graph.query(query)
        expected_result = [[10], [20], [30]]
        self.env.assertEquals(result.nodes_created, 3)
        self.env.assertEquals(result.properties_set, 3)
        self.env.assertEquals(result.result_set, expected_result)
