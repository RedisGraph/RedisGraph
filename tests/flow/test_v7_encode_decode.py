from base import FlowTestsBase
import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge

redis_con = None

class test_v7_encode_decode(FlowTestsBase):
    def __init__(self):
        self.env = Env(moduleArgs='ENTITIES_THRESHOLD 10')
        global redis_con
        redis_con = self.env.getConnection()
        info = redis_con.execute_command("INFO")
        version = info['redis_version'].split('.')
        major = int(version[0])
        minor = int(version[1])
        # Look for redis 6 RC versions and up
        if major < 5 or major == 5 and minor < 9:
            self.env.skip()

    def test01_nodes_over_multiple_keys(self):
        graph_name = "nodes_over_multiple_keys"
        redis_graph = Graph(graph_name, redis_con)
        # Create 3 nodes meta keys
        redis_graph.query("UNWIND range(0,20) as i CREATE ({val:i})")
        # Return all the nodes, before and after saving & loading the RDB, and check equality
        query = "MATCH (n) return n"
        expected = redis_graph.query(query)
        # Save RDB & Load from RDB
        redis_con.execute_command("DEBUG", "RELOAD")
        actual = redis_graph.query(query)
        self.env.assertEquals(expected.result_set, actual.result_set)
    
    def test02_no_compaction_on_nodes_delete(self):
        graph_name = "no_compaction_on_nodes_delete"
        redis_graph = Graph(graph_name, redis_con)
        # Create 3 nodes meta keys
        redis_graph.query("UNWIND range(0,20) as i CREATE ()")
        # Return all the nodes, before and after saving & loading the RDB, and check equality
        query = "MATCH (n) WITH n ORDER by id(n) return COLLECT(id(n))"
        expected_full_graph_nodes_id = redis_graph.query(query)
        # Delete 3 nodes.
        redis_graph.query("MATCH (n) WHERE id(n) IN [7,14,20] DELETE n")
        expected_nodes_id_after_delete = redis_graph.query(query)
        # Save RDB & Load from RDB
        redis_con.execute_command("DEBUG", "RELOAD")
        actual = redis_graph.query(query)
        # Validate no compaction, all IDs are the same
        self.env.assertEquals(expected_nodes_id_after_delete.result_set, actual.result_set)
        # Validate reuse of node ids - create 3 nodes.
        redis_graph.query("UNWIND range (0,2) as i CREATE ()")
        actual = redis_graph.query(query)
        self.env.assertEquals(expected_full_graph_nodes_id.result_set, actual.result_set)

    def test03_edges_over_multiple_keys(self):
        graph_name = "edges_over_multiple_keys"
        redis_graph = Graph(graph_name, redis_con)
        # Create 3 edges meta keys
        redis_graph.query("UNWIND range(0,20) as i CREATE ()-[:R {val:i}]->()")
        # Return all the edges, before and after saving & loading the RDB, and check equality
        query = "MATCH ()-[e]->() return e"
        expected = redis_graph.query(query)
        # Save RDB & Load from RDB
        redis_con.execute_command("DEBUG", "RELOAD")
        actual = redis_graph.query(query)
        self.env.assertEquals(expected.result_set, actual.result_set)

    def test04_no_compaction_on_edges_delete(self):
        graph_name = "no_compaction_on_edges_delete"
        redis_graph = Graph(graph_name, redis_con)
        # Create 3 nodes meta keys
        redis_graph.query("UNWIND range(0,20) as i CREATE ()-[:R]->()")
        # Return all the edges, before and after saving & loading the RDB, and check equality
        query = "MATCH ()-[e]->() WITH e ORDER by id(e) return COLLECT(id(e))"
        expected_full_graph_nodes_id = redis_graph.query(query)
        # Delete 3 edges.
        redis_graph.query("MATCH ()-[e]->() WHERE id(e) IN [7,14,20] DELETE e")
        expected_nodes_id_after_delete = redis_graph.query(query)
        # Save RDB & Load from RDB
        redis_con.execute_command("DEBUG", "RELOAD")
        actual = redis_graph.query(query)
        # Validate no compaction, all IDs are the same
        self.env.assertEquals(expected_nodes_id_after_delete.result_set, actual.result_set)
        # Validate reuse of edges ids - create 3 edges.
        redis_graph.query("UNWIND range (0,2) as i CREATE ()-[:R]->()")
        actual = redis_graph.query(query)
        self.env.assertEquals(expected_full_graph_nodes_id.result_set, actual.result_set)

    def test05_multiple_edges_over_multiple_keys(self):
        graph_name = "multiple_edges_over_multiple_keys"
        redis_graph = Graph(graph_name, redis_con)
        # Create 3 edges meta keys
        redis_graph.query("CREATE (n1 {val:1}), (n2 {val:2}) WITH n1, n2 UNWIND range(0,20) as i CREATE (n1)-[:R {val:i}]->(n2)")
        # Return all the edges, before and after saving & loading the RDB, and check equality
        query = "MATCH ()-[e]->() return e"
        expected = redis_graph.query(query)
        # Save RDB & Load from RDB
        redis_con.execute_command("DEBUG", "RELOAD")
        actual = redis_graph.query(query)
        self.env.assertEquals(expected.result_set, actual.result_set)

    def test06_no_compaction_on_multiple_edges_delete(self):
        graph_name = "no_compaction_on_multiple_edges_delete"
        redis_graph = Graph(graph_name, redis_con)
        # Create 3 nodes meta keys
        redis_graph.query("CREATE (n1 {val:1}), (n2 {val:2}) WITH n1, n2 UNWIND range(0,20) as i CREATE (n1)-[:R]->(n2)")
        # Return all the edges, before and after saving & loading the RDB, and check equality
        query = "MATCH ()-[e]->() WITH e ORDER by id(e) return COLLECT(id(e))"
        expected_full_graph_nodes_id = redis_graph.query(query)
        # Delete 3 edges.
        redis_graph.query("MATCH ()-[e]->() WHERE id(e) IN [7,14,20] DELETE e")
        expected_nodes_id_after_delete = redis_graph.query(query)
        # Save RDB & Load from RDB
        redis_con.execute_command("DEBUG", "RELOAD")
        actual = redis_graph.query(query)
        # Validate no compaction, all IDs are the same
        self.env.assertEquals(expected_nodes_id_after_delete.result_set, actual.result_set)
        # Validate reuse of edges ids - create 3 edges.
        redis_graph.query("MATCH (n1 {val:1}), (n2 {val:2}) WITH n1, n2 UNWIND range (0,2) as i CREATE ()-[:R]->()")
        actual = redis_graph.query(query)
        self.env.assertEquals(expected_full_graph_nodes_id.result_set, actual.result_set)

    def test07_index_after_encode_decode_in_v7(self):
        graph_name = "index_after_encode_decode_in_v7"
        redis_graph = Graph(graph_name, redis_con)
        redis_graph.query("CREATE INDEX ON :N(val)")
        # Verify indices exists.
        plan = redis_graph.execution_plan(
            "MATCH (n:N {val:1}) RETURN n")
        self.env.assertIn("Index Scan", plan)
        # Save RDB & Load from RDB
        redis_con.execute_command("DEBUG", "RELOAD")
        # Verify indices exists after loading RDB.
        plan = redis_graph.execution_plan(
            "MATCH (n:N {val:1}) RETURN n")
        self.env.assertIn("Index Scan", plan)
