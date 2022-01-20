import os
import sys
import redis
from RLTest import Env
from redisgraph import Graph
from base import FlowTestsBase

redis_con = None
redis_graph = None

class testConfig(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_con
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph("config", redis_con)

    def test01_config_get(self):
        global redis_graph

        # Try reading 'MAINTAIN_TRANSPOSED_MATRICES' from config
        config_name = "MAINTAIN_TRANSPOSED_MATRICES"
        response = redis_con.execute_command("GRAPH.CONFIG GET " + config_name)
        expected_response = [config_name, 1]
        self.env.assertEqual(response, expected_response)

        # Try reading 'QUERY_MEM_CAPACITY' from config
        config_name = "QUERY_MEM_CAPACITY"
        response = redis_con.execute_command("GRAPH.CONFIG GET " + config_name)
        expected_response = [config_name, 0] # capacity=QUERY_MEM_CAPACITY_UNLIMITED
        self.env.assertEqual(response, expected_response)

        # Try reading all configurations
        config_name = "*"
        response = redis_con.execute_command("GRAPH.CONFIG GET " + config_name)
        # At least 10 configurations should be reported
        self.env.assertGreaterEqual(len(response), 10)

    def test02_config_get_invalid_name(self):
        global redis_graph

        # Ensure that getter fails on invalid parameters appropriately
        fake_config_name = "FAKE_CONFIG_NAME"

        try:
            redis_con.execute_command("GRAPH.CONFIG GET " + fake_config_name)
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("Unknown configuration field" in str(e))
            pass

    def test03_config_set(self):
        global redis_graph

        config_name = "RESULTSET_SIZE"
        config_value = 3

        # Set configuration
        response = redis_con.execute_command("GRAPH.CONFIG SET %s %d" % (config_name, config_value))
        self.env.assertEqual(response, "OK")

        # Make sure config been updated.
        response = redis_con.execute_command("GRAPH.CONFIG GET " + config_name)
        expected_response = [config_name, config_value]
        self.env.assertEqual(response, expected_response)


        config_name = "QUERY_MEM_CAPACITY"
        config_value = 1<<30 # 1GB

        # Set configuration
        response = redis_con.execute_command("GRAPH.CONFIG SET %s %d" % (config_name, config_value))
        self.env.assertEqual(response, "OK")

        # Make sure config been updated.
        response = redis_con.execute_command("GRAPH.CONFIG GET " + config_name)
        expected_response = [config_name, config_value]
        self.env.assertEqual(response, expected_response)

    def test04_config_set_invalid_name(self):
        global redis_graph

        # Ensure that getter fails on invalid parameters appropriately
        fake_config_name = "FAKE_CONFIG_NAME"

        try:
            redis_con.execute_command("GRAPH.CONFIG SET " + fake_config_name + " 5")
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("Unknown configuration field" in str(e))
            pass

    def test05_config_invalid_subcommand(self):
        global redis_graph

        # Ensure failure on invalid sub-command, e.g. GRAPH.CONFIG DREP...
        config_name = "RESULTSET_SIZE"
        try:
            response = redis_con.execute_command("GRAPH.CONFIG DREP " + config_name + " 3")
            assert(False)
        except redis.exceptions.ResponseError as e:
            assert("Unknown subcommand for GRAPH.CONFIG" in str(e))
            pass

    def test08_config_restore_timeout_to_default(self):
        # Revert memory limit to default
        response = redis_con.execute_command("GRAPH.CONFIG SET QUERY_MEM_CAPACITY 0")
        self.env.assertEqual(response, "OK")

        # Change timeout value from default
        response = redis_con.execute_command("GRAPH.CONFIG SET TIMEOUT 10")
        self.env.assertEqual(response, "OK")

        # Make sure config been updated.
        response = redis_con.execute_command("GRAPH.CONFIG GET TIMEOUT")
        expected_response = ["TIMEOUT", 10]
        self.env.assertEqual(response, expected_response)

        # Revert timeout to unlimited
        response = redis_con.execute_command("GRAPH.CONFIG SET TIMEOUT 0")
        self.env.assertEqual(response, "OK")

        # Make sure config been updated.
        response = redis_con.execute_command("GRAPH.CONFIG GET TIMEOUT")
        expected_response = ["TIMEOUT", 0]
        self.env.assertEqual(response, expected_response)

        # Issue long-running query to validate the reconfiguration
        result = redis_graph.query("UNWIND range(1,1000000) AS v RETURN COUNT(v)")
        self.env.assertEqual(result.result_set[0][0], 1000000)

        # Change resultset_size from default
        response = redis_con.execute_command("GRAPH.CONFIG SET RESULTSET_SIZE 2")
        self.env.assertEqual(response, "OK")

        # Validate modified resultset_size
        result = redis_graph.query("UNWIND range(1, 10) AS v RETURN v")
        self.env.assertEqual(len(result.result_set), 2)

        # Revert resultset_size to unlimited with a negative argument
        response = redis_con.execute_command("GRAPH.CONFIG SET RESULTSET_SIZE -100")
        self.env.assertEqual(response, "OK")

        # Make sure resultset_size has been updated to unlimited.
        response = redis_con.execute_command("GRAPH.CONFIG GET RESULTSET_SIZE")
        expected_response = ["RESULTSET_SIZE", -1]
        self.env.assertEqual(response, expected_response)

        response = redis_con.execute_command("GRAPH.CONFIG", "GET", "NODE_CREATION_BUFFER")
        expected_response = ["NODE_CREATION_BUFFER", 16384]
        self.env.assertEqual(response, expected_response)

    def test09_set_get_node_creation_buffer(self):
        global redis_con
        redis_con.execute_command("FLUSHALL")
        self.env = Env(decodeResponses=True, moduleArgs='NODE_CREATION_BUFFER 0')
        redis_con = self.env.getConnection()

        # values less than 128 (such as 0, which this module was loaded with)
        # will be increased to 128
        creation_buffer_size = redis_con.execute_command("GRAPH.CONFIG", "GET", "NODE_CREATION_BUFFER")
        expected_response = ["NODE_CREATION_BUFFER", 128]
        self.env.assertEqual(creation_buffer_size, expected_response)

        # restart the server with a buffer argument of 600
        self.env = Env(decodeResponses=True, moduleArgs='NODE_CREATION_BUFFER 600')
        redis_con = self.env.getConnection()

        # the node creation buffer should be 1024, the next-greatest power of 2 of 600
        creation_buffer_size = redis_con.execute_command("GRAPH.CONFIG", "GET", "NODE_CREATION_BUFFER")
        expected_response = ["NODE_CREATION_BUFFER", 1024]
        self.env.assertEqual(creation_buffer_size, expected_response)

