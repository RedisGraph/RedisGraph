from common import *
from redis import ResponseError
from base import FlowTestsBase

graph = None
multiple_entity_graph = None

class testEntityUpdate(FlowTestsBase):
    def __init__(self):
        global graph
        global multiple_entity_graph
        self.env = Env(decodeResponses=True)
        # create a graph with a single node with attribute 'v'
        graph = Graph(self.env.getConnection(), 'update')
        graph.query("CREATE ({v:1})")

        # create a graph with a two nodes connected by an edge
        multiple_entity_graph = Graph(self.env.getConnection(), 'multiple_entity_update')
        multiple_entity_graph.query("CREATE (:L {v1: 1})-[:R {v1: 3}]->(:L {v2: 2})")
        multiple_entity_graph.query("CREATE INDEX ON :L(v1)")
        multiple_entity_graph.query("CREATE INDEX ON :L(v2)")

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

    def test05_update_from_projection(self):
        result = graph.query("MATCH (n) UNWIND ['Calgary'] as city_name SET n.name = city_name RETURN n.v, n.name")
        expected_result = [[1, 'Calgary']]
        self.env.assertEqual(result.properties_set, 1)
        self.env.assertEqual(result.result_set, expected_result)

    # Set the entity's properties to an empty map
    def test06_replace_property_map(self):
        empty_node = Node()
        result = graph.query("MATCH (n) SET n = {} RETURN n")
        expected_result = [[empty_node]]
        # The node originally had 2 properties, 'name' and 'city_name'
        self.env.assertEqual(result.properties_set, 2)
        self.env.assertEqual(result.result_set, expected_result)

    # Update the entity's properties by setting a specific property and merging property maps
    def test07_update_property_map(self):
        node = Node(properties={"v": 1, "v2": 2})
        result = graph.query("MATCH (n) SET n.v = 1, n += {v2: 2} RETURN n")
        expected_result = [[node]]
        self.env.assertEqual(result.properties_set, 2)
        self.env.assertEqual(result.result_set, expected_result)

    # Replacement maps overwrite existing properties and previous SETs but do not modify subsequent non-replacement SETs
    def test08_multiple_updates_to_property_map(self):
        node = Node(properties={"v": 1, "v2": 2, "v4": 4})
        result = graph.query("MATCH (n) SET n.v3 = 3, n = {v: 1}, n += {v2: 2}, n.v4 = 4 RETURN n")
        expected_result = [[node]]
        self.env.assertEqual(result.result_set, expected_result)

    # MERGE updates should support the same operations as SET updates
    def test09_merge_update_map(self):
        node = Node(properties={"v": 5})
        result = graph.query("MERGE (n {v: 1}) ON MATCH SET n = {}, n.v = 5 RETURN n")
        expected_result = [[node]]
        self.env.assertEqual(result.result_set, expected_result)

    # Update properties with a map retrieved by alias
    def test10_property_map_from_identifier(self):
        # Overwrite existing properties
        node = Node(properties={"v2": 10})
        result = graph.query("WITH {v2: 10} as props MATCH (n) SET n = props RETURN n")
        expected_result = [[node]]
        self.env.assertEqual(result.result_set, expected_result)

        # Merge property maps
        node = Node(properties={"v1": True, "v2": 10})
        result = graph.query("WITH {v1: True} as props MATCH (n) SET n += props RETURN n")
        expected_result = [[node]]
        self.env.assertEqual(result.result_set, expected_result)

    # Update properties with a map retrieved from a parameter
    def test11_property_map_from_parameter(self):
        # Overwrite existing properties
        node = Node(properties={"v2": 10})
        result = graph.query("CYPHER props={v2: 10} MATCH (n) SET n = $props RETURN n")
        expected_result = [[node]]
        self.env.assertEqual(result.result_set, expected_result)

        # Merge property maps
        node = Node(properties={"v1": True, "v2": 10})
        result = graph.query("CYPHER props={v1: true} MATCH (n) SET n += $props RETURN n")
        expected_result = [[node]]
        self.env.assertEqual(result.result_set, expected_result)

    # Fail update an entity property when left hand side is not alias
    def test12_fail_update_property_of_non_alias_entity(self):
        try:
            graph.query("MATCH P=() SET nodes(P).prop = 1 RETURN nodes(P)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("RedisGraph does not currently support non-alias references on the left-hand side of SET expressions", str(e))

    # Fail when a property is a complex type nested within an array type
    def test13_invalid_complex_type_in_array(self):
        # Test combinations of invalid types with nested and top-level arrays
        # Invalid types are NULL, maps, nodes, edges, and paths
        queries = ["MATCH (a) SET a.v = [a]",
                   "MATCH (a) SET a = {v: ['str', [1, NULL]]}",
                   "MATCH (a) SET a += [[{k: 'v'}]]",
                   "CREATE (a:L)-[e:R]->(:L) SET a.v = [e]"]
        for query in queries:
            try:
                graph.query(query)
                self.env.assertTrue(False)
            except ResponseError as e:
                self.env.assertContains("Property values can only be of primitive types or arrays of primitive types", str(e))

    # fail when attempting to perform invalid map assignment
    def test14_invalid_map_assignment(self):
        try:
            graph.query("MATCH (a) SET a.v = {f: true}")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Property values can only be of primitive types or arrays of primitive types", str(e))

    # update properties by attribute set reassignment
    def test15_assign_entity_properties(self):
        # merge attribute set of a node with existing properties
        node = Node(label="L", properties={"v1": 1, "v2": 2})
        result = multiple_entity_graph.query("MATCH (n1 {v1: 1}), (n2 {v2: 2}) SET n1 += n2 RETURN n1")
        expected_result = [[node]]
        self.env.assertEqual(result.result_set, expected_result)
        # validate index updates
        result = multiple_entity_graph.query("MATCH (n:L) WHERE n.v1 > 0 RETURN n.v1 ORDER BY n.v1")
        expected_result = [[1]]
        self.env.assertEqual(result.result_set, expected_result)
        result = multiple_entity_graph.query("MATCH (n:L) WHERE n.v2 > 0 RETURN n.v2 ORDER BY n.v2")
        expected_result = [[2],
                           [2]]
        self.env.assertEqual(result.result_set, expected_result)

        # overwrite attribute set of node with attribute set of edge
        node = Node(label="L", properties={"v1": 3})
        result = multiple_entity_graph.query("MATCH (n {v1: 1})-[e]->() SET n = e RETURN n")
        expected_result = [[node]]
        self.env.assertEqual(result.result_set, expected_result)
        # validate index updates
        result = multiple_entity_graph.query("MATCH (n:L) WHERE n.v1 > 0 RETURN n.v1 ORDER BY n.v1")
        expected_result = [[3]]
        self.env.assertEqual(result.result_set, expected_result)
        result = multiple_entity_graph.query("MATCH (n:L) WHERE n.v2 > 0 RETURN n.v2 ORDER BY n.v2")
        expected_result = [[2]]
        self.env.assertEqual(result.result_set, expected_result)

    # repeated attribute set reassignment
    def test16_assign_entity_properties(self):
        # repeated merges to the attribute set of a node
        node = Node(label="L", properties={"v1": 3, "v2": 2})
        result = multiple_entity_graph.query("MATCH (n), (x) WHERE ID(n) = 0 WITH n, x ORDER BY ID(x) SET n += x RETURN n")
        expected_result = [[node],
                           [node]]
        self.env.assertEqual(result.result_set, expected_result)
        # validate index updates
        result = multiple_entity_graph.query("MATCH (n:L) WHERE n.v1 > 0 RETURN n.v1 ORDER BY n.v1")
        expected_result = [[3]]
        self.env.assertEqual(result.result_set, expected_result)
        result = multiple_entity_graph.query("MATCH (n:L) WHERE n.v2 > 0 RETURN n.v2 ORDER BY n.v2")
        expected_result = [[2],
                           [2]]
        self.env.assertEqual(result.result_set, expected_result)

        # repeated updates to the attribute set of a node
        node = Node(label="L", properties={"v2": 2})
        result = multiple_entity_graph.query("MATCH (n), (x) WHERE ID(n) = 0 WITH n, x ORDER BY ID(x) SET n = x RETURN n")
        expected_result = [[node],
                           [node]]
        self.env.assertEqual(result.result_set, expected_result)
        # validate index updates
        result = multiple_entity_graph.query("MATCH (n:L) WHERE n.v1 > 0 RETURN n.v1 ORDER BY n.v1")
        expected_result = []
        self.env.assertEqual(result.result_set, expected_result)
        result = multiple_entity_graph.query("MATCH (n:L) WHERE n.v2 > 0 RETURN n.v2 ORDER BY n.v2")
        expected_result = [[2],
                           [2]]
        self.env.assertEqual(result.result_set, expected_result)

        # repeated multiple updates to the attribute set of a node
        node = Node(label="L", properties={"v2": 2})
        result = multiple_entity_graph.query("MATCH (n), (x) WHERE ID(n) = 0 WITH n, x ORDER BY ID(x) SET n = x, n += x RETURN n")
        expected_result = [[node],
                           [node]]
        self.env.assertEqual(result.result_set, expected_result)
        # validate index updates
        result = multiple_entity_graph.query("MATCH (n:L) WHERE n.v1 > 0 RETURN n.v1 ORDER BY n.v1")
        expected_result = []
        self.env.assertEqual(result.result_set, expected_result)
        result = multiple_entity_graph.query("MATCH (n:L) WHERE n.v2 > 0 RETURN n.v2 ORDER BY n.v2")
        expected_result = [[2],
                           [2]]
        self.env.assertEqual(result.result_set, expected_result)

    # fail when attempting to perform invalid entity assignment
    def test17_invalid_entity_assignment(self):
        queries = ["MATCH (a) SET a.v = [a]",
                   "MATCH (a) SET a = a.v",
                   "MATCH (a) SET a = NULL"]
        for query in queries:
            try:
                graph.query(query)
                self.env.assertTrue(False)
            except ResponseError as e:
                self.env.assertContains("Property values can only be of primitive types or arrays of primitive types", str(e))
