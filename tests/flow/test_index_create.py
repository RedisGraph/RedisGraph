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

    def test02_multi_prop_index_creation(self):
        # create an index over person:age and person:name
        result = redis_graph.query("CREATE INDEX ON :person(age, name)")
        self.env.assertEquals(result.indices_created, 2)

        # try to create an index over person:age and person:name, index shouldn't be created as it already exist
        result = redis_graph.query("CREATE INDEX ON :person(age, name)")
        self.env.assertEquals(result.indices_created, 0)

        # try to create an index over person:age and person:name and person:height, index for height should be created as the rest already exist
        result = redis_graph.query("CREATE INDEX ON :person(age, age, name, height)")
        self.env.assertEquals(result.indices_created, 1)

        # try to create an index over person:gender and person:name and person:height, index for gender should be created as the rest already exist
        result = redis_graph.query("CREATE INDEX ON :person(gender, gender, name, height)")
        self.env.assertEquals(result.indices_created, 1)