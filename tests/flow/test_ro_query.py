from base import FlowTestsBase
import os
import sys
import time
from RLTest import Env
from redisgraph import Graph, Node, Edge, query_result

master_con = None
slave_con = None


class test_read_only_query(FlowTestsBase):
    def __init__(self):
        self.env = Env(moduleArgs='VKEY_MAX_ENTITY_COUNT 3', useSlaves=True)
        global master_con
        global slave_con
        master_con = self.env.getConnection()
        slave_con = self.env.getSlaveConnection()

    def test01_test_simple_read_only_command(self):
        graph_name = "Test_RO_QUERY_command"
        graph = Graph(graph_name, master_con)
        graph.query("UNWIND range(0,20) as i CREATE ()")
        raw_result_set = master_con.execute_command("GRAPH.RO_QUERY", graph_name, "MATCH (n) RETURN COUNT(n)", "--compact")
        result_set = query_result.QueryResult(graph, raw_result_set).result_set
        self.env.assertEqual(21, result_set[0][0])
        try:
            raw_result_set = master_con.execute_command("GRAPH.RO_QUERY", graph_name, "CREATE()", "--compact")
            result_set = query_result.QueryResult(graph, raw_result_set).result_set
            assert(False)
        except:
            # Expecting an error.
            pass

    def test02_test_replica_read_only(self):
        graph_name = "Test_RO_QUERY_command_on_replica"
        graph = Graph(graph_name, master_con)
        graph.query("UNWIND range(0,20) as i CREATE ()")
        slave_con.execute_command("REPLICAOF", "localhost", "6379")
        time.sleep(2)
        raw_result_set = slave_con.execute_command("GRAPH.RO_QUERY", graph_name, "MATCH (n) RETURN COUNT(n)", "--compact")
        result_set = query_result.QueryResult(graph, raw_result_set).result_set
        self.env.assertEqual(21, result_set[0][0])
        try:
            # Every normal command is write command, see that replica connection throws an exception.
            raw_result_set = slave_con.execute_command("GRAPH.QUERY", graph_name, "MATCH (n) RETURN COUNT(n)", "--compact")
            result_set = query_result.QueryResult(graph, raw_result_set).result_set
            assert(False)
        except:
            # Expecting an error.
            pass
