import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge

import redis
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

redis_con = None
redis_graph = None

class testQueryValidationFlow(FlowTestsBase):

    def __init__(self):
        self.env = Env()
        global redis_con
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph("G", redis_con)
        self.populate_graph()
    
    def populate_graph(self):
         # Create a single graph.
        global redis_graph
        node = Node(properties={"age": 34})
        redis_graph.add_node(node)
        redis_graph.commit()

    # Expect an error when trying to use a function which does not exists.
    def test01_none_existing_function(self):
        query = """MATCH (n) RETURN noneExistingFunc(n.age) AS cast"""
        try:
            redis_graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    # Make sure function validation is type case insensitive.
    def test02_case_insensitive_function_name(self):
        try:
            query = """MATCH (n) RETURN mAx(n.age)"""
            redis_graph.query(query)
        except redis.exceptions.ResponseError:
            # function validation should be case insensitive.
            self.env.assertTrue(False)
    
    def test03_edge_missing_relation_type(self):
        try:
            query = """CREATE (n:Person {age:32})-[]->(:person {age:30})"""
            redis_graph.query(query)
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    def test04_escaped_quotes(self):
       query = r"CREATE (:escaped{prop1:'single \' char', prop2: 'double \" char', prop3: 'mixed \' and \" chars'})"
       actual_result = redis_graph.query(query)
       self.env.assertEquals(actual_result.nodes_created, 1)
       self.env.assertEquals(actual_result.properties_set, 3)

       query = r"MATCH (a:escaped) RETURN a.prop1, a.prop2, a.prop3"
       actual_result = redis_graph.query(query)
       expected_result = [["single ' char", 'double " char', 'mixed \' and " chars']]
       self.env.assertEquals(actual_result.result_set, expected_result)

    def test05_invalid_entity_references(self):
        try:
            query = """MATCH (a) RETURN e"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

        try:
            query = """MATCH (a) RETURN a ORDER BY e"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    def test06_where_references(self):
        try:
            query = """MATCH (a) WHERE fake = true RETURN a"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    def test07_with_references(self):
        try:
            query = """MATCH (a) WITH e RETURN e"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    def test08_count_distinct_star(self):
        try:
            query = """MATCH (a) RETURN COUNT(DISTINCT *)"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    def test09_invalid_apply_all(self):
        try:
            query = """MATCH (a) RETURN SUM(*)"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    def test10_missing_params(self):
        try:
            query = """MATCH (a {name:$name}) RETURN a"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass
    
    def test11_param_error(self):
        try:
            query = """CYPHER name=({name:'a'}) MATCH (a {name:$name}) RETURN a"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    def test12_invalid_query_order(self):
        try:
            query = """MERGE (a) MATCH (a)-[]->(b) RETURN b"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    def test13_create_bound_variables(self):
        try:
            query = """MATCH (a)-[e]->(b) CREATE (a)-[e]->(b)"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    def test14_treat_path_as_entity(self):
        redis_graph.query("CREATE ()-[:R]->()")
        try:
            query= """MATCH x=()-[]->() RETURN x.name"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    def test15_dont_crash_on_multiple_errors(self):
        try:
            query = """MATCH (a) where id(a) IN range(0) OR id(a) in range(1)"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    # Run a query in which a parsed parameter introduces a type in an unsupported context.
    def test16_param_introduces_unhandled_type(self):
        try:
            query = """CYPHER props={a:1,b:2} CREATE (a:A $props)"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("Encountered unhandled type" in e.message)
            pass

    # Validate that the module fails properly with incorrect argument counts.
    def test17_query_arity(self):
        # Call GRAPH.QUERY with a missing query argument.
        try:
            res = redis_con.execute_command("GRAPH.QUERY", "G")
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("wrong number of arguments" in e.message)
            pass

    # Run queries in which compile-time variables are accessed but not defined.
    def test18_undefined_variable_access(self):
        try:
            query = """CREATE (:person{name:bar[1]})"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("not defined" in e.message)
            pass

        try:
            query = """MATCH (a {val: undeclared}) RETURN a"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("not defined" in e.message)
            pass

        try:
            query = """UNWIND [fake] AS ref RETURN ref"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("not defined" in e.message)
            pass

    def test19_invalid_cypher_options(self):
        query = "EXPLAIN MATCH (p:president)-[:born]->(:state {name:'Hawaii'}) RETURN p"
        try:
            redis_graph.query(query)
            assert(False)
        except:
            # Expecting an error.
            pass

        query = "PROFILE MATCH (p:president)-[:born]->(:state {name:'Hawaii'}) RETURN p"
        try:
            redis_graph.query(query)
            assert(False)
        except:
            # Expecting an error.
            pass

        query = "CYPHER val=1 EXPLAIN MATCH (p:president)-[:born]->(:state {name:'Hawaii'}) RETURN p"
        try:
            redis_graph.query(query)
            assert(False)
        except:
            # Expecting an error.
            pass

        query = "CYPHER val=1 PROFILE MATCH (p:president)-[:born]->(:state {name:'Hawaii'}) RETURN p"
        try:
            redis_graph.query(query)
            assert(False)
        except:
            # Expecting an error.
            pass

    # Undirected edges are not allowed in CREATE clauses.
    def test20_undirected_edge_creation(self):
        try:
            query = """CREATE (:Endpoint)-[:R]-(:Endpoint)"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("Only directed relationships" in e.message)
            pass

    # Applying a filter for non existing entity.
    def test20_non_existing_graph_entity(self):
        try:
            query = """MATCH p=() WHERE p.name='value' RETURN p"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("Type mismatch: expected Node but was Path" in e.message)
            pass

    # Comments should not affect query functionality.
    def test21_ignore_query_comments(self):
        query = """MATCH (n)  // This is a comment
                   /* This is a block comment */
                   WHERE EXISTS(n.age)
                   RETURN n.age /* Also a block comment*/"""
        actual_result = redis_graph.query(query)
        expected_result = [[34]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """/* A block comment*/ MATCH (n)  // This is a comment
                /* This is a block comment */
                WHERE EXISTS(n.age)
                RETURN n.age /* Also a block comment*/"""
        actual_result = redis_graph.query(query)
        expected_result = [[34]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """// This is a comment
                MATCH (n)  // This is a comment
                /* This is a block comment */
                WHERE EXISTS(n.age)
                RETURN n.age /* Also a block comment*/"""
        actual_result = redis_graph.query(query)
        expected_result = [[34]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        query = """MATCH (n)  /* This is a block comment */ WHERE EXISTS(n.age)
                RETURN n.age /* Also a block comment*/"""
        actual_result = redis_graph.query(query)
        expected_result = [[34]]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # Validate procedure call refrences and definitions
    def test22_procedure_validations(self):
        try:
            # procedure call refering to a none existing alias 'n'
            query = """CALL db.idx.fulltext.queryNodes(n, 'B') YIELD node RETURN node"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("not defined" in e.message)
            pass

        # refer to procedure call original output when output is aliased.
        try:
            query = """CALL db.idx.fulltext.queryNodes('A', 'B') YIELD node AS n RETURN node"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("not defined" in e.message)
            pass

        # valid procedure call, no output aliasing
        query = """CALL db.idx.fulltext.queryNodes('A', 'B') YIELD node RETURN node"""
        redis_graph.query(query)

        # valid procedure call, output aliasing
        query = """CALL db.idx.fulltext.queryNodes('A', 'B') YIELD node AS n RETURN n"""
        redis_graph.query(query)

    # Applying a filter for a non-boolean constant should raise a compile-time error.
    def test23_invalid_constant_filter(self):
        try:
            query = """MATCH (a) WHERE 1 RETURN a"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            assert("Expected boolean predicate" in e.message)
            pass

    # Referencing a variable before defining it should raise a compile-time error.
    def test24_reference_before_definition(self):
        try:
            query = """MATCH ({prop: reference}) MATCH (reference) RETURN *"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("not defined" in e.message)
            pass

    # Invalid filters in cartesian products should raise errors.
    def test25_cartesian_product_invalid_filter(self):
        try:
            query = """MATCH p1=(), (n), ({prop: p1.path_val}) RETURN *"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("Type mismatch: expected Node but was Path" in e.message)
            pass

    # Scalar predicates in filters should raise errors.
    def test26_invalid_filter_predicate(self):
        try:
            query = """WITH 1 AS a WHERE '' RETURN a"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("Expected boolean predicate" in e.message)
            pass

    # Conditional filters with non-boolean scalar predicate children should raise errors.
    def test27_invalid_filter_predicate_child(self):
        try:
            # 'Amor' is an invalid construct for the RHS of 'OR'.
            query = """MATCH (a:Author) WHERE a.name CONTAINS 'Ernest' OR 'Amor' RETURN a"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("Expected boolean predicate" in e.message)
            pass

    # The NOT operator does not compare left and right side expressions.
    def test28_invalid_filter_binary_not(self):
        try:
            # Query should have been:
            # MATCH (u) where u.v IS NOT NULL RETURN u
            query = """MATCH (u) where u.v NOT NULL RETURN u"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("Invalid usage of 'NOT' filter" in e.message)
            pass

    def test29_invalid_filter_non_boolean_constant(self):
        try:
            query = """MATCH (a) WHERE a RETURN a"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("expected Boolean but was Node" in e.message)
            pass

        try:
            query = """MATCH (a) WHERE 1+rand() RETURN a"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("expected Boolean but was Float" in e.message)
            pass

        try:
            query = """CYPHER p=3 WITH 1 AS a WHERE $p RETURN a"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("expected Boolean but was Integer" in e.message)
            pass

        # 'val' is a boolean, so this query is valid.
        query = """WITH true AS val WHERE val return val"""
        redis_graph.query(query)

        # Non-existent properties are treated as NULLs, which are boolean in Cypher's 3-valued logic.
        query = """MATCH (a) WHERE a.fakeprop RETURN a"""
        redis_graph.query(query)

    def test31_set_invalid_property_type(self):
        try:
            query = """MATCH (a) CREATE (:L {v: a})"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("Property values can only be of primitive types" in e.message)
            pass

        try:
            query = """MATCH (a)-[]->(b) SET b.age = a"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("Property values can only be of primitive types" in e.message)
            pass
