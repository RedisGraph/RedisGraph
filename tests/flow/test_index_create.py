import os
import sys
import string
from RLTest import Env
from redisgraph import Graph

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

GRAPH_ID = "index"
redis_graph = None

class testIndexCreationFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)

    # full-text index creation
    def test01_fulltext_index_creation(self):
        # create an index over L:v0
        result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex('L', 'v0')")
        self.env.assertEquals(result.indices_created, 1)

        # create an index over L:v1
        result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex('L', 'v1')")
        self.env.assertEquals(result.indices_created, 1)

        # create an index over L:v1 and L:v2
        result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex('L', 'v1', 'v2')")
        self.env.assertEquals(result.indices_created, 1)

        # create an index over L:v0, L:v1 and L:v2
        result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex('L', 'v0', 'v1', 'v2')")
        self.env.assertEquals(result.indices_created, 0)

        # create an index over L:v2, L:v1 and L:v0
        result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex('L', 'v2', 'v1', 'v0')")
        self.env.assertEquals(result.indices_created, 0)

        # create an index over L:v3 and L:v4
        result = redis_graph.query("CALL db.idx.fulltext.createNodeIndex('L', 'v3', 'v4')")
        self.env.assertEquals(result.indices_created, 2)

