from common import *
from index_utils import *

graph = None
multiple_entity_graph = None

class testEntityUpdate():
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
        create_node_exact_match_index(multiple_entity_graph, 'L', 'v1', 'v2', sync=True)

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
        self.env.assertEqual(result.properties_set, 0)
        self.env.assertEqual(result.properties_removed, 1)

        # remove null attribute using MERGE...ON CREATE SET
        result = graph.query("UNWIND [{id: 1, aField: 'withValue', andOneWithout: null}] AS item MERGE (m:X{id: item.id}) ON CREATE SET m += item RETURN properties(m)")
        self.env.assertEqual(result.labels_added, 1)
        self.env.assertEqual(result.nodes_created, 1)
        self.env.assertEqual(result.properties_set, 2)
        expected_result = [[{'id': 1, 'aField': 'withValue'}]]
        self.env.assertEqual(result.result_set, expected_result)
        result = graph.query("MATCH (m:X) DELETE(m)")
        self.env.assertEqual(result.nodes_deleted, 1)

        # remove the 'x' attribute using MERGE...ON MATCH SET
        result = graph.query("CREATE (n:N {x:5})")
        result = graph.query("MERGE (n:N) ON MATCH SET n.x=null RETURN n")
        self.env.assertEqual(result.properties_set, 0)
        self.env.assertEqual(result.properties_removed, 1)
        result = graph.query("MATCH (n:N) DELETE(n)")
        self.env.assertEqual(result.nodes_deleted, 1)

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
        self.env.assertEqual(result.properties_set, 0)
        self.env.assertEqual(result.properties_removed, 2)
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

        try:
            graph.query("MERGE (n:N) ON CREATE SET n.a.b=3 RETURN n")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("RedisGraph does not currently support non-alias references on the left-hand side of SET expressions", str(e))

        try:
            graph.query("MERGE (n:N) ON CREATE SET n = {v: 1}, n.a.b=3 RETURN n")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("RedisGraph does not currently support non-alias references on the left-hand side of SET expressions", str(e))

        try:
            graph.query("MERGE (n:N) ON MATCH SET n.a.b=3 RETURN n")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("RedisGraph does not currently support non-alias references on the left-hand side of SET expressions", str(e))

        try:
            graph.query("MERGE (n:N) ON MATCH SET n = {v: 1}, n.a.b=3 RETURN n")
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


    def validate_node_labels(self, graph, labels, expected_count):
        for label in labels:
            result = graph.query(f"MATCH (n:{label}) RETURN n")
            self.env.assertEqual(len(result.result_set), expected_count)
            if expected_count > 0:
                for record in result.result_set:
                    self.env.assertTrue(label in record[0].labels)


    def test18_update_node_label(self):
        labels = ["TestLabel"]
        
        self.validate_node_labels(graph, labels, 0)
        result = graph.query(f"MATCH (n) SET n:{labels[0]}")
        self.env.assertEqual(result.labels_added, 1)
        self.validate_node_labels(graph, labels, 1)

        # multiple node updates
        self.validate_node_labels(multiple_entity_graph, labels, 0)
        result = multiple_entity_graph.query(f"MATCH (n) SET n:{labels[0]}")
        self.env.assertEqual(result.labels_added, 2)
        self.validate_node_labels(multiple_entity_graph, labels, 2)


    def test19_update_node_multiple_label(self):
        labels = ["TestLabel2", "TestLabel3"]

        self.validate_node_labels(graph, labels, 0)   
        result = graph.query(f"MATCH (n) SET n:{':'.join(labels)}")
        self.env.assertEqual(result.labels_added, 2)
        self.validate_node_labels(graph, labels, 1)

        # multiple node updates
        self.validate_node_labels(multiple_entity_graph, labels, 0)   
        result = multiple_entity_graph.query(f"MATCH (n) SET n:{':'.join(labels)}")
        self.env.assertEqual(result.labels_added, 4)
        self.validate_node_labels(multiple_entity_graph, labels, 2)
    

    def test20_update_node_comma_separated_labels(self):
        labels = ["TestLabel4", "TestLabel5"]

        self.validate_node_labels(graph, labels, 0)
        result = graph.query(f"MATCH (n) SET n:{labels[0]}, n:{labels[1]}")
        self.env.assertEqual(result.labels_added, 2)
        self.validate_node_labels(graph, labels, 1)

        # multiple node updates
        self.validate_node_labels(multiple_entity_graph, labels, 0)
        result = multiple_entity_graph.query(f"MATCH (n) SET n:{labels[0]}, n:{labels[1]}")
        self.env.assertEqual(result.labels_added, 4)
        self.validate_node_labels(multiple_entity_graph, labels, 2)


    def test21_update_node_label_and_property(self):
        labels = ["TestLabel6"]
       
        self.validate_node_labels(graph, labels, 0)
        result = graph.query("MATCH (n {testprop:'testvalue'}) RETURN n")
        self.env.assertEqual(len(result.result_set), 0)
        result = graph.query(f"MATCH (n) SET n:{labels[0]}, n.testprop='testvalue'")
        self.env.assertEqual(result.labels_added, 1)
        self.env.assertEqual(result.properties_set, 1)
        self.validate_node_labels(graph, labels, 1)
        result = graph.query("MATCH (n {testprop:'testvalue'}) RETURN n")
        self.env.assertEqual(len(result.result_set), 1)

        # multiple node updates
        self.validate_node_labels(multiple_entity_graph, labels, 0)
        result = multiple_entity_graph.query("MATCH (n {testprop:'testvalue'}) RETURN n")
        self.env.assertEqual(len(result.result_set), 0)
        result = multiple_entity_graph.query(f"MATCH (n) SET n:{labels[0]}, n.testprop='testvalue'")
        self.env.assertEqual(result.labels_added, 2)
        self.env.assertEqual(result.properties_set, 2)
        self.validate_node_labels(multiple_entity_graph, labels, 2)
        result = multiple_entity_graph.query("MATCH (n {testprop:'testvalue'}) RETURN n")
        self.env.assertEqual(len(result.result_set), 2)
    

    def test22_update_cp_nodes_labels_and_properties(self):
        labels = ["TestLabel7", "TestLabel8"]
        self.validate_node_labels(multiple_entity_graph, labels, 0)
        result = multiple_entity_graph.query("MATCH (n {testprop2:'testvalue'}) RETURN n")
        self.env.assertEqual(len(result.result_set), 0)

        result = multiple_entity_graph.query(f"MATCH (n), (m) SET n:{labels[0]}, n.testprop2='testvalue', m:{labels[1]}")
        self.env.assertEqual(result.labels_added, 4)
        self.env.assertEqual(result.properties_set, 2)
        self.validate_node_labels(multiple_entity_graph, labels, 2)
        result = multiple_entity_graph.query("MATCH (n {testprop2:'testvalue'}) RETURN n")
        self.env.assertEqual(len(result.result_set), 2)


    def test23_update_connected_nodes_labels_and_properties(self):
        labels = ["TestLabel9", "TestLabel10"]
        self.validate_node_labels(multiple_entity_graph, labels, 0)
        result = multiple_entity_graph.query("MATCH (n {testprop3:'testvalue'}) RETURN n")
        self.env.assertEqual(len(result.result_set), 0)

        result = multiple_entity_graph.query(f"MATCH (n)-->(m) SET n:{labels[0]}, n.testprop3='testvalue', m:{labels[1]}")
        self.env.assertEqual(result.labels_added, 2)
        self.env.assertEqual(result.properties_set, 1)
        self.validate_node_labels(multiple_entity_graph, labels, 1)
        result = multiple_entity_graph.query("MATCH (n {testprop3:'testvalue'}) RETURN n")
        self.env.assertEqual(len(result.result_set), 1)


    def test_24_fail_update_non_matched_nodes(self):
        queries = ["MATCH (n) SET x:L", "MATCH (n) SET x:L:L:L"]
        for query in queries:
            try:
                graph.query(query)
                self.env.assertTrue(False)
            except ResponseError as e:
                self.env.assertContains("x not defined", str(e))


    def test_25_fail_update_labels_for_edge(self):
        queries = ["MATCH ()-[r]->() SET r:L", "MATCH (n)-[r]->(m) WITH n, r, m UNWIND [n, r, m] AS x SET x:L"]
        for query in queries:
            try:
                multiple_entity_graph.query(query)
                self.env.assertTrue(False)
            except ResponseError as e:
                self.env.assertContains("Type mismatch: expected Node but was Relationship", str(e))
    

    def test_26_fail_update_label_for_constant(self):
        queries = ["WITH 1 AS x SET x:L"]
        for query in queries:
            try:
                graph.query(query)
                self.env.assertTrue(False)
            except ResponseError as e:
                self.env.assertContains("Update error: alias 'x' did not resolve to a graph entity", str(e))
    

    def test_27_set_label_on_merge(self):
        # on match
        labels = ["Trigger", "TestLabel11", "TestLabel12"]
        self.validate_node_labels(graph, labels, 0)
        # This will create a node with Trigger label, and set the label TestLabel11
        result = graph.query(f"MERGE(n:{labels[0]}) ON CREATE SET n:{labels[1]} ON MATCH SET n:{labels[2]}")
        self.env.assertEqual(result.labels_added, 2)
        self.validate_node_labels(graph, labels[0:1], 1)
        # This will find a node with Trigger label and set the label TestLabel12
        result = graph.query(f"MERGE(n:{labels[0]}) ON CREATE SET n:{labels[1]} ON MATCH SET n:{labels[2]}")
        self.env.assertEqual(result.labels_added, 1)
        self.validate_node_labels(graph, labels, 1)

    
    def test_28_remove_node_labels(self):
        graph.delete()
        graph.query("CREATE ()")
        labels = ["Foo", "Bar"]
        self.validate_node_labels(graph, labels, 0)

        result = graph.query(f"MATCH (n) SET n:{':'.join(labels)}")
        self.env.assertEqual(result.labels_added, 2)
        self.validate_node_labels(graph, labels, 1)
        for label in labels:
            graph.query(f"MATCH (n:{label}) REMOVE n:{label} RETURN 1")
            self.validate_node_labels(graph, [label], 0)
        self.validate_node_labels(graph, labels, 0)

    def test_29_mix_add_and_remove_node_labels(self):
        graph.delete()
        graph.query("CREATE (:Foo)")
        labels_to_add = ["Bar"]
        labels_to_remove = ["Foo"]
        self.validate_node_labels(graph, labels_to_remove, 1)

        # call set prior to remove
        result = graph.query(f"MATCH (n:Foo) SET n:{':'.join(labels_to_add)} REMOVE n:{':'.join(labels_to_remove)} RETURN 1")
        self.env.assertEqual(result.labels_added, 1)
        self.validate_node_labels(graph, labels_to_remove, 0)
        self.validate_node_labels(graph, labels_to_add, 1)

        graph.delete()
        graph.query("CREATE (:Foo)")
        self.validate_node_labels(graph, labels_to_remove, 1)

        # call remove prior to set
        result = graph.query(f"MATCH (n:Foo) REMOVE n:{':'.join(labels_to_remove)} SET n:{':'.join(labels_to_add)} RETURN 1")
        self.env.assertEqual(result.labels_added, 1)
        self.validate_node_labels(graph, labels_to_remove, 0)
        self.validate_node_labels(graph, labels_to_add, 1)

    def test_30_mix_add_and_remove_same_labels(self):
        graph.delete()
        graph.query("CREATE ()")
        labels = ["Foo"]
        self.validate_node_labels(graph, labels, 0)

        # call set prior to remove
        result = graph.query(f"MATCH (n) SET n:{':'.join(labels)} REMOVE n:{':'.join(labels)} RETURN 1")
        self.env.assertEqual(result.labels_added, 1)
        self.env.assertEqual(result.labels_removed, 1)
        self.validate_node_labels(graph, labels, 0)

        graph.delete()
        graph.query("CREATE ()")
        self.validate_node_labels(graph, labels, 0)

        # call remove prior to set
        result = graph.query(f"MATCH (n) REMOVE n:{':'.join(labels)} SET n:{':'.join(labels)} RETURN 1")
        self.env.assertEqual(result.labels_added, 1)
        self.env.assertEqual(result.labels_removed, 0)
        self.validate_node_labels(graph, labels, 1)

    def test_32_mix_merge_and_remove_node_labels(self):
        graph.delete()
        labels_to_remove = ["Foo"]
        self.validate_node_labels(graph, labels_to_remove, 0)

        result = graph.query(f"MERGE (n:{':'.join(labels_to_remove)})  REMOVE n:{':'.join(labels_to_remove)} RETURN 1")
        self.env.assertEqual(result.labels_added, 1)
        self.env.assertEqual(result.labels_removed, 1)
        self.validate_node_labels(graph, labels_to_remove, 0)

    def test_33_syntax_error_remove_labels_on_match_on_create(self):
        queries = ["MERGE (n) ON MATCH REMOVE n:Foo RETURN 1", "MERGE (n) ON CREATE REMOVE n:Foo RETURN 1"]
        for query in queries:
            try:
                graph.query(query)
                self.env.assertTrue(False)
            except ResponseError as e:
                self.env.assertContains("Invalid input 'R':", str(e))

    def test_34_fail_remove_labels_for_edge(self):
        queries = ["MATCH ()-[r]->() REMOVE r:L RETURN 1", "MATCH (n)-[r]->(m) WITH n, r, m UNWIND [n, r, m] AS x REMOVE x:L RETURN 1"]
        for query in queries:
            try:
                multiple_entity_graph.query(query)
                self.env.assertTrue(False)
            except ResponseError as e:
                self.env.assertContains("Type mismatch: expected Node but was Relationship", str(e))
    
    def test_35_fail_remove_label_for_constant(self):
        queries = ["WITH 1 AS x REMOVE x:L RETURN x"]
        for query in queries:
            try:
                graph.query(query)
                self.env.assertTrue(False)
            except ResponseError as e:
                self.env.assertContains("Update error: alias 'x' did not resolve to a graph entity", str(e))

    def test_36_mix_add_and_remove_node_properties(self):
        graph.delete()
        graph.query("CREATE ({v:1})")
        result = graph.query("MATCH (n {v:1}) REMOVE n.v SET n.v=1")
        self.env.assertEqual(result.properties_set, 1)
        self.env.assertEqual(result.properties_removed, 1)

    def test_37_set_property_null(self):
        graph.delete()
        graph.query("CREATE ()")
        result = graph.query("MATCH (v) SET v.p1 = v.p8, v.p1 = v.p5, v.p2 = v.p4")
        result = graph.query("MATCH (v) RETURN v")
        self.env.assertEqual(result.header, [[1, 'v']])

