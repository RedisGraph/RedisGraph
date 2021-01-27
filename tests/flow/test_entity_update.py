from RLTest import Env
from redisgraph import Graph, Node, Edge

from base import FlowTestsBase

graph = None

class testEntityUpdate(FlowTestsBase):
    def __init__(self):
        global graph
        self.env = Env(decodeResponses=True)
        graph = Graph('update', self.env.getConnection())

        # create a single node with attribute 'v'
        graph.query("CREATE ({v:1})")

    def test01_update_attribute(self):
        # update existing attribute 'v'
        result = graph.query("MATCH (n) SET n.v = 2")
        self.env.assertEqual(result.properties_set, 1)

    def test02_update_none_existing_attr(self):
        # introduce a new attribute 'x'
        result = graph.query("MATCH (n) SET n.x = 1")
        self.env.assertEqual(result.properties_set, 1)

    def test03_update_no_change(self):
        # setting 'x' to its current value
        result = graph.query("MATCH (n) SET n.x = 1")
        self.env.assertEqual(result.properties_set, 0)

        # setting both 'v' and 'x' to their current values
        result = graph.query("MATCH (n) SET n.v = 2, n.x = 1")
        self.env.assertEqual(result.properties_set, 0)

        # update 'v' to a new value, 'x' remains the same
        result = graph.query("MATCH (n) SET n.v = 1, n.x = 1")
        self.env.assertEqual(result.properties_set, 1)

        # update 'x' to a new value, 'v' remains the same
        result = graph.query("MATCH (n) SET n.v = 1, n.x = 2")
        self.env.assertEqual(result.properties_set, 1)

    def test04_update_remove_attribute(self):
        # remove the 'x' attribute
        result = graph.query("MATCH (n) SET n.x = NULL")
        self.env.assertEqual(result.properties_set, 1)

