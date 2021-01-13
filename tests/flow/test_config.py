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

