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

        # Try reading 'QUERY_MEM_CAPACITY' from config
        config_name = "QUERY_MEM_CAPACITY"
        response = redis_con.execute_command("GRAPH.CONFIG GET " + config_name)
        expected_response = [config_name, 0] # capacity=QUERY_MEM_CAPACITY_UNLIMITED
        self.env.assertEqual(response, expected_response)

        # Try reading all configurations
        config_name = "*"
        response = redis_con.execute_command("GRAPH.CONFIG GET " + config_name)
        # At least 9 configurations should be reported
        self.env.assertGreaterEqual(len(response), 9)

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
            # Set multiple configuration values, VKEY_MAX_ENTITY_COUNT is NOT
            # a runtime configuration, expecting this command to fail
            response = redis_con.execute_command("GRAPH.CONFIG SET QUERY_MEM_CAPACITY 150 VKEY_MAX_ENTITY_COUNT 40")
            assert(False)
        except redis.exceptions.ResponseError as e:
            # Expecting an error.
            assert("Field can not be re-configured" in str(e))
        
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

