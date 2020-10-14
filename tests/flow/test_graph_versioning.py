import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

graph = None

GRAPH_ID = "GraphVersion"

class testGraphVersioning(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global graph
        redis_con = self.env.getConnection()
        graph = Graph(GRAPH_ID, redis_con)

    def graph_version(self):
        q = "RETURN 1"
        result = graph.query(q)
        return result.graph_version

    # Make sure graph version changes once a new label is created
    def test01_version_update_on_label_creation(self):
        v = self.graph_version()

        # Adding a node without a label shouldn't update graph version
        q = """CREATE ()"""
        res = graph.query(q)
        self.env.assertEquals(v, res.graph_version)
        v = res.graph_version

        # Adding a labeled node should update graph version
        q = """CREATE (:L)"""
        res = graph.query(q)
        self.env.assertNotEqual(v, res.graph_version)
        v = res.graph_version

        # Adding a node with an existing label shouldn't update graph version
        q = """CREATE (:L)"""
        res = graph.query(q)
        self.env.assertEquals(v, res.graph_version)
        v = res.graph_version

    # Make sure graph version changes once a new relationship type is created
    def test02_version_update_on_relation_creation(self):
        v = self.graph_version()

        # Adding edge with a new relationship type should update graph version
        q = """CREATE ()-[:R]->()"""
        res = graph.query(q)
        self.env.assertNotEqual(v, res.graph_version)
        v = res.graph_version

        # Adding edge with existing relationship type shouldn't update graph version
        q = """CREATE ()-[:R]->()"""
        res = graph.query(q)
        self.env.assertEquals(v, res.graph_version)
        v = res.graph_version

    # Make sure graph version changes once a new attribute is created
    def test03_version_update_on_attribute_creation(self):
        v = self.graph_version()

        # Adding a new attribute should update graph version
        q = """CREATE ({v:1})"""
        res = graph.query(q)
        self.env.assertNotEqual(v, res.graph_version)
        v = res.graph_version

        # Adding a new node with existing attribute shouldn't update graph version
        q = """CREATE ({v:1})"""
        res = graph.query(q)
        self.env.assertEquals(v, res.graph_version)
        v = res.graph_version

        # Adding a new edge with a new attribute should update graph version
        q = """CREATE ()-[:R {q:1}]->()"""
        res = graph.query(q)
        self.env.assertNotEqual(v, res.graph_version)
        v = res.graph_version

        # Adding a new edge with existing attribute shouldn't update graph version
        q = """CREATE ()-[:R {v:1}]->()"""
        res = graph.query(q)
        self.env.assertEquals(v, res.graph_version)
        v = res.graph_version

