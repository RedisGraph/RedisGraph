import os
import sys
import redis
from RLTest import Env
from redisgraph import Graph, Node, Edge

from base import FlowTestsBase

GRAPH_ID = "G"
redis_graph = None

class testGraphCreationFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
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

        query = """UNWIND ['Vancouver', 'Portland', 'Calgary'] AS city CREATE (p:person {birthplace: city}) RETURN p.birthplace ORDER BY p.birthplace"""
        result = redis_graph.query(query)
        expected_result = [['Calgary'], ['Portland'], ['Vancouver']]
        self.env.assertEquals(result.nodes_created, 3)
        self.env.assertEquals(result.properties_set, 3)
        self.env.assertEquals(result.result_set, expected_result)

    def test04_create_with_null_properties(self):
        query = """CREATE (a:L {v1: NULL, v2: 'prop'}) RETURN a"""
        result = redis_graph.query(query)
        node = Node(label="L", properties={"v2": "prop"})
        expected_result = [[node]]

        self.env.assertEquals(result.labels_added, 1)
        self.env.assertEquals(result.nodes_created, 1)
        self.env.assertEquals(result.properties_set, 1)
        self.env.assertEquals(result.result_set, expected_result)

        # Create 2 new nodes, one with no properties and one with a property 'v'
        query = """CREATE (:M), (:M {v: 1})"""
        redis_graph.query(query)

        # Verify that a MATCH...CREATE accesses the property correctly.
        query = """MATCH (m:M) WITH m ORDER BY m.v DESC CREATE ({v: m.v})"""
        result = redis_graph.query(query)
        self.env.assertEquals(result.nodes_created, 2)
        self.env.assertEquals(result.properties_set, 1)

    def test05_create_with_property_reference(self):
        # Skip this test if running under Valgrind, as it causes a memory leak.
        if self.env.envRunner.debugger is not None:
            self.env.skip()

        # Queries that reference properties before they have been created should emit an error.
        try:
            query = """CREATE (a {val: 2}), (b {val: a.val})"""
            redis_graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertIn("undefined attribute-set", str(e))

    def test06_create_project_volatile_value(self):
        # The path e is volatile; verify that it can be projected after entity creation.
        query = """MATCH ()-[e*]->() CREATE (:L) WITH e RETURN 5"""
        result = redis_graph.query(query)
        expected_result = [[5], [5]]

        self.env.assertEquals(result.nodes_created, 2)
        self.env.assertEquals(result.result_set, expected_result)

        query = """UNWIND [1, 2] AS val WITH collect(val) AS arr CREATE (:L) RETURN arr"""
        result = redis_graph.query(query)
        expected_result = [[[1, 2]]]

        self.env.assertEquals(result.nodes_created, 1)
        self.env.assertEquals(result.result_set, expected_result)

    # Fail when a property is a complex type nested within an array type
    def test07_create_invalid_complex_type_in_array(self):
        # Test combinations of invalid types with nested and top-level arrays
        # Invalid types are NULL, maps, nodes, edges, and paths
        queries = ["CREATE (a), (b) SET a.v = [b]",
                   "CREATE (a {v: ['str', [1, NULL]]})",
                   "CREATE (a {v: [[{k: 'v'}]]})",
                   "CREATE (a:L {v: [e]})-[e:R]->(:L)"]
        for query in queries:
            try:
                redis_graph.query(query)
                self.env.assertTrue(False)
            except redis.exceptions.ResponseError as e:
                self.env.assertContains("Property values can only be of primitive types or arrays of primitive types", str(e))

    # test creating a node with multiple attributes with the same name
    # expecting node with single attribute 'name' with the last mentioned value 'B'
    def test08_create_node_with_2_attr_same_name(self):
        query = """CREATE (a:N {name:'A', name:'B'})"""
        result = redis_graph.query(query)
        self.env.assertEquals(result.nodes_created, 1)
        self.env.assertEquals(result.properties_set, 1)

        query = """MATCH (a:N) RETURN a.name"""
        result = redis_graph.query(query)
        expected_result = [['B']]
        self.env.assertEquals(result.result_set, expected_result)

