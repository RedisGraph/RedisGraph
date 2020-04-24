from base import FlowTestsBase
import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge

redis_con = None
# dump created with
#  graph.query g "CREATE (:L1{val:1})-[:E{val:2}]->(:L2{val:3})"
#  graph.query g "CREATE INDEX ON :L1(val)"
#  dump g

v4_rdb = "\a\x81\x82\xb6\xa9\x85\xd6\xadh\x04\x05\x02g\x00\x02\x02\x02\x81\xff\xff\xff\xff\xff\xff\xff\xff\x05\x04ALL\x00\x02\x01\x05\x03val\x02\x00\x05\x03L1\x00\x02\x01\x05\x03val\x02\x01\x05\x03L2\x00\x02\x01\x05\x03val\x02\x01\x02\x81\xff\xff\xff\xff\xff\xff\xff\xff\x05\x04ALL\x00\x02\x01\x05\x03val\x02\x00\x05\x02E\x00\x02\x01\x05\x03val\x02\x02\x02\x00\x02\x01\x02\x00\x02\x01\x05\x04val\x00\x02\x04\x02\x01\x02\x01\x02\x01\x02\x01\x02\x01\x05\x04val\x00\x02\x04\x02\x03\x02\x01\x02\x00\x02\x00\x02\x01\x02\x00\x02\x01\x05\x04val\x00\x02\x04\x02\x02\x02\x01\x05\x03L1\x00\x05\x04val\x00\x00\t\x00\xe3\x82\x0f\xb4\xd4\xc2\xc8V"
v6_rdb = "\a\x81\x82\xb6\xa9\x85\xd6\xadh\x06\x05\x02g\x00\x02\x01\x05\x04val\x00\x02\x02\x02\x00\x05\x03L1\x00\x02\x01\x02\x00\x05\x04val\x00\x02\x01\x05\x03L2\x00\x02\x00\x02\x01\x02\x00\x05\x02E\x00\x02\x00\x02\x02\x02\x01\x02\x00\x02\x01\x05\x04val\x00\x02`\x00\x02\x01\x02\x01\x02\x01\x02\x01\x05\x04val\x00\x02`\x00\x02\x03\x02\x01\x02\x00\x02\x01\x02\x00\x02\x01\x05\x04val\x00\x02`\x00\x02\x02\x00\t\x00\xf0\xc8<\b\xed\xbb\xa3\x90"

class test_prev_rdb_decode(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_con
        redis_con = self.env.getConnection()

    def validate_graph(self, redis_graph):
        node0 = Node(0, label='L1', properties={'val':1})
        node1 = Node(1, label='L2', properties={'val':3})
        edge01 = Edge(0, 'E', 1, 0, {'val':2})
        results = redis_graph.query("MATCH (n)-[e]->(m) RETURN n, e, m")
        self.env.assertEqual(results.result_set, [[node0, edge01, node1]])
        plan = redis_graph.execution_plan(
            "MATCH (n:L1 {val:1}) RETURN n")
        self.env.assertIn("Index Scan", plan)

    def test_v4_decode(self):
        graph_name = "v4_rdb_restore"
        redis_con.restore(graph_name, 0, v4_rdb, True)
        redis_graph = Graph(graph_name, redis_con)
        self.validate_graph(redis_graph)

    def test_v6_decode(self):
        graph_name = "v6_rdb_restore"
        redis_con.restore(graph_name, 0, v6_rdb, True)
        redis_graph = Graph(graph_name, redis_con)
        self.validate_graph(redis_graph)
