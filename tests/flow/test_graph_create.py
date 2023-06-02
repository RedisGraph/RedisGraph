from common import *

GRAPH_ID = "G"
redis_graph = None


class testGraphCreationFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(redis_con, GRAPH_ID)

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

    # def test05_create_with_property_reference(self):
        # Queries that reference properties before they have been created should emit an error.
        # try:
        #     query = """CREATE (a {val: 2}), (b {val: a.val})"""
        #     redis_graph.query(query)
        #     self.env.assertTrue(False)
        # except redis.exceptions.ResponseError as e:
        #     self.env.assertIn("undefined attribute", str(e))

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

    # Fail when a property is a complex type or complex type nested within an array type
    def test07_create_invalid_complex_type_in_array(self):
        # Test combinations of invalid types with nested and top-level arrays
        # Invalid types are NULL, maps, nodes, edges, and paths
        queries = ["CREATE (a), (b) SET a.v = [b]",
                   "CREATE (a {v: [[{k: 'v'}]]})",
                   "CREATE (a {v: ['str', [1, NULL]]})",
                   "CREATE (a:L)-[e:R]->(:L {v: [{}]})",
                   "CREATE (a), (b)-[:R {k:[{}]}]->(c)",
                ]
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

    # test creating a node with some alias, and then creating an edge that touches that node
    # "variable redeclared" error should return only if the relation alias was already declared
    # also, a node cannot be redeclared unless it is used to create new edges in a pattern
    def test09_create_use_alias_in_many_clauses(self):
        query = """CREATE (n1:Node1) CREATE (n2:Node1) CREATE (n1)-[r:Rel1]->(n2)"""
        result = redis_graph.query(query)
        self.env.assertEquals(result.nodes_created, 2)
        self.env.assertEquals(result.relationships_created, 1)

        # This is a valid query, even though 'n1' is redeclared in the CREATE clause
        query = """MATCH (n1:Node1) CREATE (n1)-[r:Rel1]->(n2)"""
        result = redis_graph.query(query)
        self.env.assertEquals(result.nodes_created, 2)
        self.env.assertEquals(result.relationships_created, 2)

        queries = ["CREATE (n1)-[r:Rel1]->(n2) CREATE (n2)-[r:Rel1]->(n1)",
                   "CREATE (n1)-[r:Rel1]->(n2), (n2)-[r:Rel1]->(n1)",
                   "CREATE (n1)-[r:Rel1]->(n2) CREATE (n2)-[r2:Rel1]->(n3), (n3)-[r:Rel1]->(n2)",
                   "MATCH (r) CREATE (r)"]
        for query in queries:
            try:
                redis_graph.query(query)
                self.env.assertTrue(False)
            except redis.exceptions.ResponseError as e:
                self.env.assertContains("The bound variable 'r' can't be redeclared in a CREATE clause", str(e))

    # test creating queries with matching relationship type :R|R
    # the results can't report duplicates
    def test10_match_duplicated_reltype(self):
        query = """CREATE (a:A)-[r1:R1]->(b:B), (a)-[r2:R2]->(b)"""
        result = redis_graph.query(query)
        self.env.assertEquals(result.nodes_created, 2)
        self.env.assertEquals(result.relationships_created, 2)
        self.env.assertEquals(result.labels_added, 2)

        # search for same relationship multiple times, should report one relationship
        queries = ["MATCH (a:A)-[r:R1]->(b:B) RETURN count(r)",
                   "MATCH (a:A)-[r:R1|R1]->(b:B) RETURN count(r)",
                   "MATCH (a:A)-[r:R1|R1|R1|R1]->(b:B) RETURN count(r)",
                   "MATCH (a:A)-[r:R2]->(b:B) RETURN count(r)",
                   "MATCH (a:A)-[r:R2|R2]->(b:B) RETURN count(r)",
                   "MATCH (a:A)-[r:R2|R2|R3]->(b:B) RETURN count(r)",
                   "MATCH (a:A)-[r:R2|R2|R3|R4|R4]->(b:B) RETURN count(r)",
                   ]
        for query in queries:
            result = redis_graph.query(query)
            expected_result = [[1]]
            self.env.assertEquals(result.result_set, expected_result)

        # search for two relationship multiple times, should report two relationships
        queries = ["MATCH (a:A)-[r:R1|R2]->(b:B) RETURN count(r)",
                   "MATCH (a:A)-[r:R1|R2|R1|R2]->(b:B) RETURN count(r)",
                   "MATCH (a:A)-[r:R1|R1|R2|R2]->(b:B) RETURN count(r)",
                   "MATCH (a:A)-[r:R1|R2|R2|R2]->(b:B) RETURN count(r)",
                   "MATCH (a:A)-[r:R2|R1|R1|R1]->(b:B) RETURN count(r)",
                   "MATCH (a:A)-[r:R2|R2|R1|R1]->(b:B) RETURN count(r)",
                   ]
        for query in queries:
            result = redis_graph.query(query)
            expected_result = [[2]]
            self.env.assertEquals(result.result_set, expected_result)

        # These queries should not report results
        queries = ["MATCH (a:A)<-[r:R1|R2]-(b:B) RETURN count(r)",
                   "MATCH (a:A)-[r:R3]->(b:B) RETURN count(r)",
                   "MATCH (a:A)-[r:R3|R3]->(b:B) RETURN count(r)",
                   "MATCH (a:A)-[r:R3|R3|R4|R4]->(b:B) RETURN count(r)",
                   ]
        for query in queries:
            result = redis_graph.query(query)
            expected_result = [[0]]
            self.env.assertEquals(result.result_set, expected_result)

    # test referencing entities defined previously in the same query
    # the "intermediate" entities and their properties will be evaluated as NULL
    def test11_use_intermediate_entities(self):
        # test using function with valid arguments
        # 'a' is an intermidate node, which doesn't have any attributes just yet

        queries = [
                "CREATE (a:A {v:0}), ()-[r:R {k:toJSON(a)}]->() RETURN r.k",
                "CREATE (a:A {v:0}), ()-[:R]->(c {k:toJSON(a)}) RETURN c.k",
        ]

        for query in queries:
            result = redis_graph.query(query)
            self.env.assertEquals(result.nodes_created, 3, depth=1)
            self.env.assertEquals(result.properties_set, 2, depth=1)
            self.env.assertEquals(result.relationships_created, 1, depth=1)
            self.env.assertContains("\"properties\": {}", result.result_set[0][0])

        queries = [
                "MERGE (a:A {v:2})-[r:R {k:toJSON(a)}]->() RETURN r.k",
                "MERGE (a:A {v:2})-[:R]->(b {k:toJSON(a)}) RETURN b.k",
        ]

        for query in queries:
            result = redis_graph.query(query)
            self.env.assertEquals(result.nodes_created, 2, depth=1)
            self.env.assertEquals(result.properties_set, 2, depth=1)
            self.env.assertEquals(result.relationships_created, 1, depth=1)
            self.env.assertContains("\"properties\": {}", result.result_set[0][0])

        # test using functions with invalid arguments
        queries = [
            # invalid argument to predicate functions, which expect a list,
            # but the function properties() returns a Map
            "CREATE (a), (b)-[:R]->(c {k:any(x IN properties(a) WHERE x = 0)})",
            "CREATE (a), (b)-[:R]->(c {k:none(x IN properties(a) WHERE x = 0)})",
            "CREATE (a), (b)-[:R {k:single(x IN properties(a) WHERE x = 0)}]->()",
            # invalid argument to function floor(), which expects an Integer,
            # Float, or Null but any() returns Boolean
            "CREATE (a:A {n:'A'}), (b:B {v:floor(any(v4 IN [2] WHERE b = [a IN keys(a)]))})"
        ]

        for query in queries:
            try:
                redis_graph.query(query)
                self.env.assertTrue(False)
            except redis.exceptions.ResponseError as e:
                self.env.assertContains("Type mismatch", str(e), depth=1)

        # test referencing intermediate entities
        error_invalid_input   = "Invalid input"
        error_undef_attribute = "Attempted to access undefined attribute"
        error_undef_node_edge = "not defined"
        error_unhandled_type  = "Encountered unhandled type in inlined properties"
        error_primitive_type  = "Property values can only be of primitive types or arrays of primitive types"
        error_merge_null      = "Cannot merge node using null property value"

        queries_with_errors = [
            # reference to intermediate node
            ("MERGE (a:A {v:a})", error_undef_node_edge),
            ("CREATE (a:A {v:a})", error_undef_node_edge),
            ("CREATE (a:A {v:1}), (b {v:a})", error_primitive_type),
            ("MERGE (a:A)-[:R]->(b:B {v:a})", error_primitive_type),
            ("CREATE (a:A)-[:R]->(b:B {v:a})", error_primitive_type),

            # reference to intermediate edge
            ("MERGE ()-[r:R {v:r}]->()", error_undef_node_edge),
            ("CREATE ()-[r:R {v:r}]->()", error_undef_node_edge),
            ("MERGE ()-[r:R]->(b:B {v:r})", error_primitive_type),
            ("CREATE ()-[r:R]->(b:B {v:r})", error_primitive_type),
            ("CREATE ()-[r1:R1]->(), ()-[r2:R2]->({v:r1})", error_primitive_type),

            # assign path pattern to property
            ("MATCH ({v:()}) RETURN 0", error_invalid_input),
            ("MATCH (a {v:()-[]->()}) RETURN a", error_unhandled_type),
            ("MERGE ({v:()}) RETURN 0", error_invalid_input),
            ("MERGE (a {v:()-[]->()}) return n", error_unhandled_type),
            ("CREATE ({v:()}) RETURN 0", error_invalid_input),
            ("CREATE (a {v:()-[]->()}) return n", error_unhandled_type),

            # MERGE referencing to property of intermediate node
            ("MERGE (a {v:3})-[:R]->(b:B {v:a.v})", error_merge_null),
            ("MERGE ()-[r:R {v:2}]->(b:B {v:r.v})", error_merge_null),
        ]

        for query, error in queries_with_errors:
            self._assert_exception(redis_graph, query, error)

        # test referencing to properties of intermediate entities
        # the query is valid, but the value of the property will be NULL
        queries = [
            # reference to property of intermediate node
            "CREATE (a {v:1}), (b {v:a.v}) RETURN b.v",
            "CREATE (a {v:0})-[:R]->(b:B {v:a.v}) RETURN b.v",

            # reference to property of intermediate edge
            "CREATE ()-[r:R {v:2}]->(b:B {v:r.v}) RETURN b.v",
            "CREATE ()-[r:R {v:1}]->(), (a {v:r.v}) RETURN a.v",
        ]

        for query in queries:
            result = redis_graph.query(query)
            expected_result = [[None]]
            self.env.assertEquals(result.result_set, expected_result)

    def test12_redeclaring_matched_vars(self):
        error_redeclare_node      = "The alias 'n' can't be redeclared as node"
        error_redeclare_in_merge  = "The bound variable 'n' can't be redeclared in a MERGE clause"
        error_redeclare_in_create = "The bound variable 'n' can't be redeclared in a CREATE clause"
        error_props_in_merge      = "The bound node 'n' can't be redeclared in a MERGE clause"
        error_props_in_create     = "The bound node 'n' can't be redeclared in a CREATE clause"
        queries = [
            # Redeclare with a different type
            ("MATCH ()-[n]->() CREATE (n)-[:R]->()", error_redeclare_node),
            ("MATCH (n)-[:R]->() CREATE ()-[n:R]->()", error_redeclare_in_create),
            ("MATCH ()-[n]->() MERGE (n)-[:R]->()", error_redeclare_node),
            ("MATCH (n)-[:R]->() MERGE ()-[n:R]->()", error_redeclare_in_merge),
            # Modify matched entity
            # ("MATCH (n)-[]->() CREATE (n:L)-[:R]->()", error_props_in_create),
            # ("MATCH (n)-[]->() CREATE (n {v:1})-[:R]->()", error_props_in_create),
            ("MATCH ()-[n]->() CREATE ()-[n:R]->()", error_redeclare_in_create),
            ("MATCH ()-[n]->() CREATE ()-[n {v:2}]->()", error_redeclare_in_create),
            ("MATCH (n)-[:R]->() MERGE (n:L)-[:R]->()", error_props_in_merge),
            ("MATCH (n)-[:R]->() MERGE (n {v:1})-[:R]->()", error_props_in_merge),
            ("MATCH ()-[n]->() MERGE ()-[n:R]->()", error_redeclare_in_merge),
            ("MATCH ()-[n]->() MERGE ()-[n {v:2}]->()", error_redeclare_in_merge),
        ]

        for query, error in queries:
            self._assert_exception(redis_graph, query, error)

