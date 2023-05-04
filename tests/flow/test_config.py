from common import *

redis_con = None
redis_graph = None
# Number of options available.
NUMBER_OF_OPTIONS = 14

class testConfig(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_con
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(redis_con, "config")

    def test01_config_get(self):
        global redis_graph

        # Try reading 'QUERY_MEM_CAPACITY' from config
        config_name = "QUERY_MEM_CAPACITY"
        response = redis_con.execute_command("GRAPH.CONFIG GET " + config_name)
        expected_response = [config_name, 0] # capacity=QUERY_MEM_CAPACITY_UNLIMITED
        self.env.assertEqual(response, expected_response)

        # Try reading all configurations
        config_name = "*"
        response = redis_con.execute_command("GRAPH.CONFIG GET " + config_name)
        # 14 configurations should be reported
        self.env.assertEquals(len(response), NUMBER_OF_OPTIONS)

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
        config_value = 1<<20 # 1MB

        # Set configuration
        response = redis_con.execute_command("GRAPH.CONFIG SET %s %d" % (config_name, config_value))
        self.env.assertEqual(response, "OK")

        # Make sure config been updated.
        response = redis_con.execute_command("GRAPH.CONFIG GET " + config_name)
        expected_response = [config_name, config_value]
        self.env.assertEqual(response, expected_response)

    def test04_config_set_multi(self):
        # Set multiple configuration values
        response = redis_con.execute_command("GRAPH.CONFIG SET RESULTSET_SIZE 3 QUERY_MEM_CAPACITY 100")
        self.env.assertEqual(response, "OK")

        # Make sure both values been updated
        names = ["RESULTSET_SIZE", "QUERY_MEM_CAPACITY"]
        values = [3, 100]
        for name, val in zip(names, values):
            response = redis_con.execute_command("GRAPH.CONFIG GET %s" % name)
            expected_response = [name, val]
            self.env.assertEqual(response, expected_response)

    def test05_config_set_invalid_multi(self):
        # Get current configuration
        prev_conf = redis_con.execute_command("GRAPH.CONFIG GET *")

        try:
            # Set multiple configuration values, THREAD_COUNT is NOT
            # a runtime configuration, expecting this command to fail
            response = redis_con.execute_command("GRAPH.CONFIG SET QUERY_MEM_CAPACITY 150 THREAD_COUNT 40")
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("This configuration parameter cannot be set at run-time" in str(e))

        try:
            # Set multiple configuration values, FAKE_CONFIG_NAME is NOT a valid
            # configuration, expecting this command to fail
            response = redis_con.execute_command("GRAPH.CONFIG SET QUERY_MEM_CAPACITY 150 FAKE_CONFIG_NAME 40")
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("Unknown configuration field" in str(e))

        try:
            # Set multiple configuration values, -1 is not a valid value for
            # MAX_QUEUED_QUERIES, expecting this command to fail
            response = redis_con.execute_command("GRAPH.CONFIG SET QUERY_MEM_CAPACITY 150 MAX_QUEUED_QUERIES -1")
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("Failed to set config value" in str(e))

        # make sure configuration wasn't modified
        current_conf = redis_con.execute_command("GRAPH.CONFIG GET *")
        self.env.assertEqual(prev_conf, current_conf)

    def test06_config_set_invalid_name(self):

        # Ensure that setter fails on unknown configuration field
        fake_config_name = "FAKE_CONFIG_NAME"

        try:
            redis_con.execute_command("GRAPH.CONFIG SET " + fake_config_name + " 5")
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("Unknown configuration field" in str(e))
            pass

    def test07_config_invalid_subcommand(self):

        # Ensure failure on invalid sub-command, e.g. GRAPH.CONFIG DREP...
        config_name = "RESULTSET_SIZE"
        try:
            response = redis_con.execute_command("GRAPH.CONFIG DREP " + config_name + " 3")
            assert(False)
        except redis.exceptions.ResponseError as e:
            assert("Unknown subcommand for GRAPH.CONFIG" in str(e))
            pass

    def test08_config_reset_to_defaults(self):
        # Revert memory limit to default
        response = redis_con.execute_command("GRAPH.CONFIG SET QUERY_MEM_CAPACITY 0")
        self.env.assertEqual(response, "OK")

        # Change timeout value from default
        response = redis_con.execute_command("GRAPH.CONFIG SET TIMEOUT 5")
        self.env.assertEqual(response, "OK")

        # Make sure config been updated.
        response = redis_con.execute_command("GRAPH.CONFIG GET TIMEOUT")
        expected_response = ["TIMEOUT", 5]
        self.env.assertEqual(response, expected_response)

        # Revert timeout to unlimited
        response = redis_con.execute_command("GRAPH.CONFIG SET TIMEOUT 0")
        self.env.assertEqual(response, "OK")

        # Make sure config been updated.
        response = redis_con.execute_command("GRAPH.CONFIG GET TIMEOUT")
        expected_response = ["TIMEOUT", 0]
        self.env.assertEqual(response, expected_response)

        # Change timeout_default value from default
        response = redis_con.execute_command("GRAPH.CONFIG SET TIMEOUT_DEFAULT 5")
        self.env.assertEqual(response, "OK")

        # Make sure config been updated.
        response = redis_con.execute_command("GRAPH.CONFIG GET TIMEOUT_DEFAULT")
        expected_response = ["TIMEOUT_DEFAULT", 5]
        self.env.assertEqual(response, expected_response)

        # Revert timeout_default to unlimited
        response = redis_con.execute_command("GRAPH.CONFIG SET TIMEOUT_DEFAULT 0")
        self.env.assertEqual(response, "OK")

        # Make sure config been updated.
        response = redis_con.execute_command("GRAPH.CONFIG GET TIMEOUT_DEFAULT")
        expected_response = ["TIMEOUT_DEFAULT", 0]
        self.env.assertEqual(response, expected_response)

        # Change timeout_max value from default
        response = redis_con.execute_command("GRAPH.CONFIG SET TIMEOUT_MAX 5")
        self.env.assertEqual(response, "OK")

        # Make sure config been updated.
        response = redis_con.execute_command("GRAPH.CONFIG GET TIMEOUT_MAX")
        expected_response = ["TIMEOUT_MAX", 5]
        self.env.assertEqual(response, expected_response)

        # Revert timeout_max to unlimited
        response = redis_con.execute_command("GRAPH.CONFIG SET TIMEOUT_MAX 0")
        self.env.assertEqual(response, "OK")

        # Make sure config been updated.
        response = redis_con.execute_command("GRAPH.CONFIG GET TIMEOUT_MAX")
        expected_response = ["TIMEOUT_MAX", 0]
        self.env.assertEqual(response, expected_response)

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

    def test09_set_invalid_values(self):
        # The run-time configurations supported by RedisGraph are:
        # MAX_QUEUED_QUERIES
        # TIMEOUT
        # QUERY_MEM_CAPACITY
        # DELTA_MAX_PENDING_CHANGES
        # RESULTSET_SIZE

        # Validate that attempting to set these configurations to
        # invalid values fails
        try:
            # MAX_QUEUED_QUERIES must be a positive value
            redis_con.execute_command("GRAPH.CONFIG SET MAX_QUEUED_QUERIES 0")
            assert(False)
        except redis.exceptions.ResponseError as e:
            assert("Failed to set config value MAX_QUEUED_QUERIES to 0" in str(e))
            pass

        # TIMEOUT, QUERY_MEM_CAPACITY, and DELTA_MAX_PENDING_CHANGES must be
        # non-negative values, 0 resets to default
        for config in ["TIMEOUT", "QUERY_MEM_CAPACITY", "DELTA_MAX_PENDING_CHANGES"]:
            try:
                redis_con.execute_command("GRAPH.CONFIG SET %s -1" % config)
                assert(False)
            except redis.exceptions.ResponseError as e:
                assert("Failed to set config value %s to -1" % config in str(e))
                pass

        # No configuration can be set to a string
        for config in ["MAX_QUEUED_QUERIES", "TIMEOUT", "QUERY_MEM_CAPACITY",
                       "DELTA_MAX_PENDING_CHANGES", "RESULTSET_SIZE"]:
            try:
                redis_con.execute_command("GRAPH.CONFIG SET %s invalid" % config)
                assert(False)
            except redis.exceptions.ResponseError as e:
                assert(("Failed to set config value %s to invalid" % config) in str(e))

    def test10_set_get_vkey_max_entity_count(self):
        global redis_graph

        config_name = "VKEY_MAX_ENTITY_COUNT"
        config_value = 100

        # Set configuration
        response = redis_con.execute_command("GRAPH.CONFIG SET %s %d" % (config_name, config_value))
        self.env.assertEqual(response, "OK")

        # Make sure config been updated.
        response = redis_con.execute_command("GRAPH.CONFIG GET " + config_name)
        expected_response = [config_name, config_value]
        self.env.assertEqual(response, expected_response)

    def test11_set_get_node_creation_buffer(self):
        # flush and stop is needed for memcheck for clean shutdown
        self.env.flush()
        self.env.stop()

        self.env = Env(decodeResponses=True, moduleArgs='NODE_CREATION_BUFFER 0')
        global redis_con
        redis_con = self.env.getConnection()

        # values less than 128 (such as 0, which this module was loaded with)
        # will be increased to 128
        creation_buffer_size = redis_con.execute_command("GRAPH.CONFIG", "GET", "NODE_CREATION_BUFFER")
        expected_response = ["NODE_CREATION_BUFFER", 128]
        self.env.assertEqual(creation_buffer_size, expected_response)

        # restart the server with a buffer argument of 600
        self.env.flush()
        self.env = Env(decodeResponses=True, moduleArgs='NODE_CREATION_BUFFER 600')
        redis_con = self.env.getConnection()

        # the node creation buffer should be 1024, the next-greatest power of 2 of 600
        creation_buffer_size = redis_con.execute_command("GRAPH.CONFIG", "GET", "NODE_CREATION_BUFFER")
        expected_response = ["NODE_CREATION_BUFFER", 1024]
        self.env.assertEqual(creation_buffer_size, expected_response)

