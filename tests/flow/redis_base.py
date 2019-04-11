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
        return cls.__name__

    # a single command specified as multiline string
    @classmethod
    def createCommand(cls):
        return ""

    # several commands specified as multiine string, one command per line
    @classmethod
    def createCommands(cls):
        return ""

    @classmethod
    def create_graph(cls):
        global redis_cli
        global graph
        
        redis_cli.execute_command("DEL", cls.graphId())  # delete previous graph if exists
        
        graph = Graph(cls.graphId(), redis_cli)

        cmd = " ".join(map(lambda x: x.strip(), cls.createCommand().split("\n")))
        if cmd != "":
            graph.query(cmd)

        for cmd in cls.createCommands().split("\n"):
            cmd = cmd.strip()
            if cmd != "":
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
        cls.create_graph()

    @classmethod
    def tearDownClass(cls):
        redis_server.stop()

    def execute(self, cmd):
        global redis_cli
        return redis_cli.execute_command(cmd)

    def query(self, cmd):
        global graph
        q = graph.query(cmd)
        try:
            return q.result_set[1:]
        except:
            return []

    def explain(cls, query):
        return graph.execution_plan(query)

    def multi(self):
        global redis_cli
        redis_cli.execute_command("MULTI")
    
    def exec_(self):
        return redis_cli.execute_command("EXEC")
