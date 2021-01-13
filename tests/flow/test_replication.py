import os
import sys
import time
from RLTest import Env
from redisgraph import Graph, Node, Edge

from base import FlowTestsBase

GRAPH_ID = "replication"

# test to see if replication works as expected
# RedisGraph should replicate all write queries which had an effect on the
# underline graph, e.g. CREATE, DELETE, UPDATE operations as well as
# index creation and removal, read queries shouldn't be replicated.

class testReplication(FlowTestsBase):
    def __init__(self):
        # skip test if we're running under Valgrind
        if Env().envRunner.debugger is not None:
            Env().skip() # valgrind is not working correctly with replication

        self.env = Env(decodeResponses=True, env='oss', useSlaves=True)

    def test_CRUD_replication(self):
        # create a simple graph
        env = self.env
        source_con = env.getConnection()
        replica_con = env.getSlaveConnection()

        # enable write commands on slave, required as all RedisGraph
        # commands are registered as write commands
        replica_con.config_set("slave-read-only", "no")

        # perform CRUD operations
        # create a simple graph
        graph = Graph(GRAPH_ID, source_con)
        replica = Graph(GRAPH_ID, replica_con)
        s = Node(label='L', properties={'id': 0, 'name': 'a'})
        t = Node(label='L', properties={'id': 1, 'name': 'b'})
        e = Edge(s, 'R', t)

        graph.add_node(s)
        graph.add_node(t)
        graph.add_edge(e)
        graph.flush()

        # create index
        q = "CREATE INDEX ON :L(id)"
        graph.query(q)

        # update entity
        q = "MATCH (n:L {id:0}) SET n.id = 2, n.name = 'c'"
        graph.query(q)

        # delete entity
        q = "MATCH (n:L {id:1}) DELETE n"
        graph.query(q)

        # give replica some time to catch up
        time.sleep(1)

        # make sure index is available on replica
        q = "MATCH (s:L {id:2}) RETURN s.name"
        plan = graph.execution_plan(q)
        replica_plan = replica.execution_plan(q)
        env.assertIn("Index Scan", plan)
        self.env.assertEquals(replica_plan, plan)

        # issue query on both source and replica
        # make sure results are the same
        result = graph.query(q).result_set
        replica_result = replica.query(q).result_set
        self.env.assertEquals(replica_result, result)

