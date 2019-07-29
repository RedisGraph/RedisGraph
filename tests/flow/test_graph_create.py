import os
import sys
from redisgraph import Graph, Node, Edge

from base import FlowTestsBase

GRAPH_ID = "G"
redis_graph = None

class testGraphCreationFlow(FlowTestsBase):
    def __init__(self):
        super(testGraphCreationFlow, self).__init__()
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
