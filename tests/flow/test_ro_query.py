from common import *
import time

slave_con = None
master_con = None

def checkSlaveSynced(env, slaveConn, graph_name, time_out=5):
    time.sleep(time_out)
    res = slaveConn.execute_command("keys", graph_name)
    env.assertEqual(res, [graph_name])

class test_read_only_query(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True, useSlaves=True)
        if self.env.envRunner.debugger is not None:
            self.env.skip() # valgrind is not working correctly with replication
        global master_con
        global slave_con
        master_con = self.env.getConnection()
        slave_con = self.env.getSlaveConnection()

    def test01_test_simple_read_only_command(self):
        # This test check graph.RO_QUERY to execute read only commands with success.
        graph_name = "Test_RO_QUERY_command"
        graph = Graph(master_con, graph_name)
        graph.query("UNWIND range(0,20) as i CREATE ()")
        raw_result_set = master_con.execute_command("GRAPH.RO_QUERY", graph_name, "MATCH (n) RETURN COUNT(n)", "--compact")
        result_set = query_result.QueryResult(graph, raw_result_set).result_set
        self.env.assertEqual(21, result_set[0][0])
        # Try execute write commands with RO_QUERY
        try:
            master_con.execute_command("GRAPH.RO_QUERY", graph_name, "CREATE()", "--compact")
            assert(False)
        except:
            # Expecting an error.
            pass
    
    def test02_test_RO_QUERY_fail_on_write_operations(self):
        # This test check graph.RO_QUERY to execute read only commands with success.
        graph_name = "Test_RO_QUERY_fail_on_write_command"
        graph = Graph(master_con, graph_name)
        # Try execute write commands with RO_QUERY
        try:
            master_con.execute_command("GRAPH.RO_QUERY", graph_name, "CREATE()", "--compact")
            assert(False)
        except:
            # Expecting an error.
            pass
        try:
            master_con.execute_command("GRAPH.RO_QUERY", graph_name, "MERGE()", "--compact")
            assert(False)
        except:
            # Expecting an error.
            pass
        try:
            master_con.execute_command("GRAPH.RO_QUERY", graph_name, "MATCH(n) DELETE n", "--compact")
            assert(False)
        except:
            # Expecting an error.
            pass
        try:
            master_con.execute_command("GRAPH.RO_QUERY", graph_name, "CREATE INDEX ON :person(age)", "--compact")
            assert(False)
        except:
            # Expecting an error.
            pass
        try:
            master_con.execute_command("GRAPH.RO_QUERY", graph_name, "DROP INDEX ON :Person(age)", "--compact")
            assert(False)
        except:
            # Expecting an error.
            pass

    def test03_test_replica_read_only(self):
        if OS == 'macos':
            self.env.skip()
        # This test checks that only RO_QUERY is valid on replicas.
        graph_name = "Test_RO_QUERY_command_on_replica"
        graph = Graph(master_con, graph_name)
        graph.query("UNWIND range(0,20) as i CREATE ()")
        slave_con.execute_command("REPLICAOF", "127.0.0.1", self.env.envRunner.port)
        checkSlaveSynced(self.env, slave_con, graph_name)
        raw_result_set = slave_con.execute_command("GRAPH.RO_QUERY", graph_name, "MATCH (n) RETURN COUNT(n)", "--compact")
        result_set = query_result.QueryResult(graph, raw_result_set).result_set
        self.env.assertEqual(21, result_set[0][0])
        try:
            # Every GRAPH.QUERY command is a write command, see that replica connection throws an exception.
            slave_con.execute_command("GRAPH.QUERY", graph_name, "MATCH (n) RETURN COUNT(n)", "--compact")
            assert(False)
        except:
            # Expecting an error.
            pass
