import os
import re
import sys
import redis
from RLTest import Env
from redisgraph import Graph, Node, Edge
from base import FlowTestsBase

redis_graph = None
graph_2 = None

class testGraphMergeFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_graph
        global graph_2
        redis_con = self.env.getConnection()
        redis_graph = Graph("G", redis_con)
        graph_2 = Graph("H", redis_con)

    def test17_complex_merge_queries(self):
        # Beginning with an empty graph
        global graph_2
        # Create a new pattern
        query = """MERGE (a:Person {name: 'a'}) MERGE (b:Person {name: 'b'}) MERGE (a)-[e:FRIEND {val: 1}]->(b) RETURN a.name, e.val, b.name"""
        result = graph_2.query(query)
        expected = [['a', 1, 'b']]

        # Verify the results
        self.env.assertEquals(result.labels_added, 1)
        self.env.assertEquals(result.nodes_created, 2)
        self.env.assertEquals(result.relationships_created, 1)
        self.env.assertEquals(result.properties_set, 3)
        self.env.assertEquals(result.result_set, expected)

        # Repeat the query and verify that no changes were introduced
        result = graph_2.query(query)
        self.env.assertEquals(result.labels_added, 0)
        self.env.assertEquals(result.nodes_created, 0)
        self.env.assertEquals(result.relationships_created, 0)
        self.env.assertEquals(result.properties_set, 0)
        self.env.assertEquals(result.result_set, expected)

        # Verify that these entities are accessed properly with MATCH...MERGE queries
        query = """MATCH (a:Person {name: 'a'}), (b:Person {name: 'b'}) MERGE (a)-[e:FRIEND {val: 1}]->(b) RETURN a.name, e.val, b.name"""
        result = graph_2.query(query)
        self.env.assertEquals(result.labels_added, 0)
        self.env.assertEquals(result.nodes_created, 0)
        self.env.assertEquals(result.relationships_created, 0)
        self.env.assertEquals(result.properties_set, 0)
        self.env.assertEquals(result.result_set, expected)

        # Verify that we can bind entities properly in variable-length traversals
        query = """MATCH (a)-[*]->(b) MERGE (a)-[e:FRIEND {val: 1}]->(b) RETURN a.name, e.val, b.name"""
        result = graph_2.query(query)
        self.env.assertEquals(result.labels_added, 0)
        self.env.assertEquals(result.nodes_created, 0)
        self.env.assertEquals(result.relationships_created, 0)
        self.env.assertEquals(result.properties_set, 0)
        self.env.assertEquals(result.result_set, expected)

        # Verify UNWIND...MERGE does not recreate existing entities
        query = """UNWIND ['a', 'b'] AS names MERGE (a:Person {name: names}) RETURN a.name"""
        expected = [['a'], ['b']]

        result = graph_2.query(query)
        self.env.assertEquals(result.labels_added, 0)
        self.env.assertEquals(result.nodes_created, 0)
        self.env.assertEquals(result.relationships_created, 0)
        self.env.assertEquals(result.properties_set, 0)
        self.env.assertEquals(result.result_set, expected)

        # Merging entities from an UNWIND list
        query = """UNWIND ['a', 'b', 'c'] AS names MERGE (a:Person {name: names}) ON MATCH SET a.set_by = 'match' ON CREATE SET a.set_by = 'create' RETURN a.name, a.set_by ORDER BY a.name"""
        expected = [['a', 'match'],
                    ['b', 'match'],
                    ['c', 'create']]

        result = graph_2.query(query)
        self.env.assertEquals(result.labels_added, 0)
        self.env.assertEquals(result.nodes_created, 1)
        self.env.assertEquals(result.properties_set, 4)
        self.env.assertEquals(result.result_set, expected)

        # Connect 'c' to both 'a' and 'b' via a Friend relation
        # One thing to note here is that both `c` and `x` are bounded, which means
        # our current merge distinct validation inspect the created edge only using its relationship, properties and bounded
        # nodes! as such the first created edge is different from the second one (due to changes in the destination node).
        query = """MATCH (c:Person {name: 'c'}) MATCH (x:Person) WHERE x.name in ['a', 'b'] WITH c, x MERGE(c)-[:FRIEND]->(x)"""
        result = graph_2.query(query)
        self.env.assertEquals(result.labels_added, 0)
        self.env.assertEquals(result.nodes_created, 0)
        self.env.assertEquals(result.properties_set, 0)
        self.env.assertEquals(result.relationships_created, 2)

        # Verify function calls in MERGE do not recreate existing entities
        query = """UNWIND ['A', 'B'] AS names MERGE (a:Person {name: toLower(names)}) RETURN a.name"""
        expected = [['a'], ['b']]

        result = graph_2.query(query)
        self.env.assertEquals(result.labels_added, 0)
        self.env.assertEquals(result.nodes_created, 0)
        self.env.assertEquals(result.relationships_created, 0)
        self.env.assertEquals(result.properties_set, 0)
        self.env.assertEquals(result.result_set, expected)

        query = """MERGE (a:Person {name: 'a'}) ON MATCH SET a.set_by = 'match' ON CREATE SET a.set_by = 'create' MERGE (b:Clone {name: a.name + '_clone'}) ON MATCH SET b.set_by = 'match' ON CREATE SET b.set_by = 'create' RETURN a.name, a.set_by, b.name, b.set_by"""
        result = graph_2.query(query)
        expected = [['a', 'match', 'a_clone', 'create']]

        # Verify the results
        self.env.assertEquals(result.labels_added, 1)
        self.env.assertEquals(result.nodes_created, 1)
        self.env.assertEquals(result.properties_set, 2)
        self.env.assertEquals(result.result_set, expected)

    def test18_merge_unique_creations(self):
        global graph_2
        # Create a new pattern with non-unique entities.
        query = """UNWIND ['newprop1', 'newprop2'] AS x MERGE ({v:x})-[:e]->(n {v:'newprop1'})"""
        result = graph_2.query(query)

        # Verify that every entity was created in both executions.
        self.env.assertEquals(result.nodes_created, 4)
        self.env.assertEquals(result.relationships_created, 2)
        self.env.assertEquals(result.properties_set, 4)

        # Repeat the query.
        result = graph_2.query(query)

        # Verify that no data was modified.
        self.env.assertEquals(result.nodes_created, 0)
        self.env.assertEquals(result.relationships_created, 0)
        self.env.assertEquals(result.properties_set, 0)

    def test19_merge_dependency(self):
        redis_con = self.env.getConnection()
        graph = Graph("M", redis_con)

        # Starting with an empty graph.
        # Create 2 nodes and connect them to one another.
        self.env.flush()
        query = """MERGE (a:Person {name: 'a'}) MERGE (b:Person {name: 'b'}) MERGE (a)-[:FRIEND]->(b) MERGE (b)-[:FRIEND]->(a)"""
        result = graph.query(query)

        # Verify that every entity was created.
        self.env.assertEquals(result.nodes_created, 2)
        self.env.assertEquals(result.relationships_created, 2)
        self.env.assertEquals(result.properties_set, 2)

        # Repeat the query.
        result = graph.query(query)

        # Verify that no data was modified.
        self.env.assertEquals(result.nodes_created, 0)
        self.env.assertEquals(result.relationships_created, 0)
        self.env.assertEquals(result.properties_set, 0)

    def test20_merge_edge_dependency(self):
        redis_con = self.env.getConnection()
        graph = Graph("M", redis_con)

        # Starting with an empty graph.
        # Make sure the pattern ()-[]->()-[]->()-[]->() exists.
        self.env.flush()
        query = """MERGE (a {v:1}) MERGE (b {v:2}) MERGE (a)-[:KNOWS]->(b) MERGE ()-[:KNOWS]->()-[:KNOWS]->()"""
        result = graph.query(query)

        # Verify that every entity was created.
        self.env.assertEquals(result.nodes_created, 5)
        self.env.assertEquals(result.relationships_created, 3)
        self.env.assertEquals(result.properties_set, 2)

        # Repeat the query.
        result = graph.query(query)

        # Verify that no data was modified.
        self.env.assertEquals(result.nodes_created, 0)
        self.env.assertEquals(result.relationships_created, 0)
        self.env.assertEquals(result.properties_set, 0)

    def test21_merge_scan(self):
        redis_con = self.env.getConnection()
        graph = Graph("M", redis_con)

        # Starting with an empty graph.
        # All node scan should see created nodes.
        self.env.flush()
        query = """MERGE (a {v:1}) WITH a MATCH (n) MERGE (n)-[:KNOWS]->(m)"""
        result = graph.query(query)

        # Verify that every entity was created.
        self.env.assertEquals(result.nodes_created, 2)
        self.env.assertEquals(result.relationships_created, 1)
        self.env.assertEquals(result.properties_set, 1)

        # Starting with an empty graph.
        # Label scan should see created nodes.
        self.env.flush()
        query = """MERGE (a:L {v:1}) WITH a MATCH (n:L) MERGE (n)-[:KNOWS]->(m)"""
        result = graph.query(query)

        # Verify that every entity was created.
        self.env.assertEquals(result.nodes_created, 2)
        self.env.assertEquals(result.relationships_created, 1)
        self.env.assertEquals(result.properties_set, 1)

    def test22_merge_label_scan(self):
        redis_con = self.env.getConnection()
        graph = Graph("M", redis_con)

        # Starting with an empty graph.
        # Make sure the pattern ()-[]->()-[]->()-[]->() exists.
        self.env.flush()
        query = """MERGE (a {v:1}) MERGE (b {v:2}) MERGE (a)-[:KNOWS]->(b) WITH a AS c, b AS d MATCH (c)-[:KNOWS]->(d) MERGE (c)-[:LIKES]->(d)"""
        result = graph.query(query)

        # Verify that every entity was created.
        self.env.assertEquals(result.nodes_created, 2)
        self.env.assertEquals(result.relationships_created, 2)
        self.env.assertEquals(result.properties_set, 2)

        # Repeat the query.
        result = graph.query(query)

        # Verify that no data was modified.
        self.env.assertEquals(result.nodes_created, 0)
        self.env.assertEquals(result.relationships_created, 0)
        self.env.assertEquals(result.properties_set, 0)

    def test23_merge_var_traverse(self):
        redis_con = self.env.getConnection()
        graph = Graph("M", redis_con)

        # Starting with an empty graph.
        # Make sure the pattern ()-[]->()-[]->()-[]->() exists.
        self.env.flush()
        query = """MERGE (a {v:1}) MERGE (b {v:2}) MERGE (a)-[:KNOWS]->(b) WITH a AS c, b AS d MATCH (c)-[:KNOWS*]->(d) MERGE (c)-[:LIKES]->(d)"""
        result = graph.query(query)

        # Verify that every entity was created.
        self.env.assertEquals(result.nodes_created, 2)
        self.env.assertEquals(result.relationships_created, 2)
        self.env.assertEquals(result.properties_set, 2)

        # Repeat the query.
        result = graph.query(query)

        # Verify that no data was modified.
        self.env.assertEquals(result.nodes_created, 0)
        self.env.assertEquals(result.relationships_created, 0)
        self.env.assertEquals(result.properties_set, 0)

    def test24_merge_merge_delete(self):
        redis_con = self.env.getConnection()
        graph = Graph("M", redis_con)

        # Merge followed by an additional merge and ending with a deletion
        # which doesn't have any data to operate on,
        # this used to trigger force lock release, as the delete didn't tried to acquire/release the lock
        self.env.flush()
        query = """MERGE (user:User {name:'Sceat'}) WITH user UNWIND [1,2,3] AS sessionHash MERGE (user)-[:HAS_SESSION]->(newSession:Session {hash:sessionHash}) WITH DISTINCT user, collect(newSession.hash) as newSessionHash MATCH (user)-->(s:Session) WHERE NOT s.hash IN newSessionHash DELETE s"""
        result = graph.query(query)

        # Verify that every entity was created.
        self.env.assertEquals(result.nodes_created, 4)
        self.env.assertEquals(result.properties_set, 4)
        self.env.assertEquals(result.relationships_created, 3)

        # Repeat the query.
        result = graph.query(query)

        # Verify that no data was modified.
        self.env.assertEquals(result.nodes_created, 0)
        self.env.assertEquals(result.properties_set, 0)
        self.env.assertEquals(result.relationships_created, 0)

    def test25_merge_with_where(self):
        redis_con = self.env.getConnection()
        graph = Graph("M", redis_con)

        # Index the "L:prop) combination so that the MERGE tree will not have a filter op.
        query = """CREATE INDEX ON :L(prop)"""
        graph.query(query)

        query = """MERGE (n:L {prop:1}) WITH n WHERE n.prop < 1 RETURN n.prop"""
        result = graph.query(query)
        plan = graph.execution_plan(query)

        # Verify that the Filter op follows a Project op.
        self.env.assertTrue(re.search('Project\s+Filter', plan))

        # Verify that there is no Filter op after the Merge op.
        self.env.assertFalse(re.search('Merge\s+Filter', plan))

        # Verify that the entity was created and no results were returned.
        self.env.assertEquals(result.nodes_created, 1)
        self.env.assertEquals(result.properties_set, 1)

        # Repeat the query.
        result = graph.query(query)

        # Verify that no data was modified and no results were returned.
        self.env.assertEquals(result.nodes_created, 0)
        self.env.assertEquals(result.properties_set, 0)

    def test26_merge_set_invalid_property(self):
        redis_con = self.env.getConnection()
        graph = Graph("M", redis_con)

        query = """MATCH p=() MERGE () ON MATCH SET p.prop4 = 5"""
        result = graph.query(query)
        self.env.assertEquals(result.properties_set, 0)

    def test27_merge_create_invalid_entity(self):
        # Skip this test if running under Valgrind, as it causes a memory leak.
        if Env().envRunner.debugger is not None:
            Env().skip()

        redis_con = self.env.getConnection()
        graph = Graph("N", redis_con) # Instantiate a new graph.

        try:
            # Try to create a node with an invalid NULL property.
            query = """MERGE (n {v: NULL})"""
            graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("Cannot merge node using null property value" in str(e))
            pass

        # Verify that no entities were created.
        query = """MATCH (a) RETURN a"""
        result = graph.query(query)
        self.env.assertEquals(result.result_set, [])

        try:
            # Try to merge a node with a self-referential property.
            query = """MERGE (a:L {v: a.v})"""
            graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            self.env.assertIn("undefined property", str(e))
