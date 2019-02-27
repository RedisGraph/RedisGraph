
import os
import sys
import unittest
from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
from disposableredis import DisposableRedis

from base import FlowTestsBase

def redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

redis_server = None
redis_cli = None
graph = None

class RedisGraphTestBase(FlowTestsBase):
    @classmethod
    def graphId(cls):
        return "g"

    @classmethod
    def createCommands(cls):
        return ""

    @classmethod
    def create_graph(cls, cmd):
        global redis_cli
        redis_cli.execute_command("DEL", cls.graphId())  # delete previous graph if exists
        for cmd in cmd.strip().split("\n"):
            graph.query(cmd)
    
    @classmethod
    def setUpClass(cls):
        global redis_server
        global redis_cli
        global graph

        print "\n### " + cls.__name__
        redis_server = redis()
        redis_server.start()
        redis_cli = redis_server.client()
        
        graph = Graph(cls.graphId(), redis_cli)
        cls.create_graph(cls.createCommands())

    @classmethod
    def tearDownClass(cls):
        redis_server.stop()

    def execute(self, cmd):
        global redis_cli
        return redis_cli.execute_command(cmd, self.graphId())

    def query(self, cmd):
        global redis_cli
        q = redis_cli.execute_command("GRAPH.QUERY", self.graphId(), cmd)
        try:
            return q[0][1:]
        except:
            return []

    def begin(self):
        global redis_cli
        redis_cli.execute_command("MULTI")
    
    def commit(self):
        return redis_cli.execute_command("EXEC")
