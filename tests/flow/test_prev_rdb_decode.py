from base import FlowTestsBase
import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge

redis_con = None

class test_prev_rdb_decode(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_con
        redis_con = self.env.getConnection()

    def tearDown(self):
        self.env.flush()

    def test_v4_decode(self):
        graph_name = "v4_rdb_restore"
        # dump created with the following query (v4 supported property value: integer, double, boolean, string)
        #  graph.query g "CREATE (:L1 {val:1, strval: 'str', doubleval: 5.5, boolval: true})-[:E{val:2}]->(:L2{val:3})"
        #  graph.query g "CREATE INDEX ON :L1(val)"
        #  dump g
        v4_rdb = "\a\x81\x82\xb6\xa9\x85\xd6\xadh\x04\x05\x02g\x00\x02\x02\x02\x81\xff\xff\xff\xff\xff\xff\xff\xff\x05\x04ALL\x00\x02\x04\x05\aboolval\x05\tdoubleval\x05\x06strval\x05\x03val\x02\x00\x05\x03L1\x00\x02\x04\x05\aboolval\x05\tdoubleval\x05\x06strval\x05\x03val\x02\x01\x05\x03L2\x00\x02\x01\x05\x03val\x02\x01\x02\x81\xff\xff\xff\xff\xff\xff\xff\xff\x05\x04ALL\x00\x02\x01\x05\x03val\x02\x00\x05\x02E\x00\x02\x01\x05\x03val\x02\x02\x02\x00\x02\x01\x02\x00\x02\x04\x05\bboolval\x00\x02\x10\x02\x01\x05\ndoubleval\x00\x02@@\x04\x00\x00\x00\x00\x00\x00\x16@\x05\astrval\x00\x02A\x00\x05\x04str\x00\x05\x04val\x00\x02\x04\x02\x01\x02\x01\x02\x01\x02\x01\x02\x01\x05\x04val\x00\x02\x04\x02\x03\x02\x01\x02\x00\x02\x00\x02\x01\x02\x00\x02\x01\x05\x04val\x00\x02\x04\x02\x02\x02\x01\x05\x03L1\x00\x05\x04val\x00\x00\t\x00\xb38\x87\x01U^\x8b\xe3"
        redis_con.restore(graph_name, 0, v4_rdb, True)
        redis_graph = Graph(graph_name, redis_con)
        node0 = Node(node_id=0, label='L1', properties={'val':1, 'strval': 'str', 'doubleval': 5.5, 'boolval': True})
        node1 = Node(node_id=1, label='L2', properties={'val':3})
        edge01 = Edge(src_node=0, relation='E', dest_node=1, edge_id=0, properties={'val':2})
        results = redis_graph.query("MATCH (n)-[e]->(m) RETURN n, e, m")
        self.env.assertEqual(results.result_set, [[node0, edge01, node1]])
        plan = redis_graph.execution_plan("MATCH (n:L1 {val:1}) RETURN n")
        self.env.assertIn("Index Scan", plan)
        results = redis_graph.query("MATCH (n:L1 {val:1}) RETURN n")
        self.env.assertEqual(results.result_set, [[node0]])

    def test_v6_decode(self):
        graph_name = "v6_rdb_restore"
        # dump created with the following query (v6 supported property value: integer, double, boolean, string, null, array)
        #  graph.query g "CREATE (:L1 {val:1, strval: 'str', numval: 5.5, nullval: NULL, boolval: true, array: [1,2,3]})-[:E{val:2}]->(:L2{val:3})"
        #  graph.query g "CREATE INDEX ON :L1(val)"
        #  dump g
        v6_rdb = "\a\x81\x82\xb6\xa9\x85\xd6\xadh\x06\x05\x02g\x00\x02\x06\x05\x04val\x00\x05\astrval\x00\x05\anumval\x00\x05\bnullval\x00\x05\bboolval\x00\x05\x06array\x00\x02\x02\x02\x00\x05\x03L1\x00\x02\x01\x02\x00\x05\x04val\x00\x02\x01\x05\x03L2\x00\x02\x00\x02\x01\x02\x00\x05\x02E\x00\x02\x00\x02\x02\x02\x01\x02\x00\x02\x06\x05\x04val\x00\x02`\x00\x02\x01\x05\astrval\x00\x02H\x00\x05\x04str\x00\x05\anumval\x00\x02\x80\x00\x00@\x00\x04\x00\x00\x00\x00\x00\x00\x16@\x05\bnullval\x00\x02\x80\x00\x00\x80\x00\x05\bboolval\x00\x02P\x00\x02\x01\x05\x06array\x00\x02\b\x02\x03\x02`\x00\x02\x01\x02`\x00\x02\x02\x02`\x00\x02\x03\x02\x01\x02\x01\x02\x01\x05\x04val\x00\x02`\x00\x02\x03\x02\x01\x02\x00\x02\x01\x02\x00\x02\x01\x05\x04val\x00\x02`\x00\x02\x02\x00\t\x00\xd9\r\xb4c\xf2Z\xd9\xb3"
        redis_con.restore(graph_name, 0, v6_rdb, True)
        redis_graph = Graph(graph_name, redis_con)
        node0 = Node(node_id=0, label='L1', properties={'val':1, 'strval': 'str', 'doubleval': 5.5, 'boolval': True, 'nullval': None, 'array': [1,2,3]})
        node1 = Node(node_id=1, label='L2', properties={'val':3})
        edge01 = Edge(src_node=0, relation='E', dest_node=1, edge_id=0, properties={'val':2})
        results = redis_graph.query("MATCH (n)-[e]->(m) RETURN n, e, m")
        self.env.assertEqual(results.result_set, [[node0, edge01, node1]])
        plan = redis_graph.execution_plan("MATCH (n:L1 {val:1}) RETURN n")
        self.env.assertIn("Index Scan", plan)
        results = redis_graph.query("MATCH (n:L1 {val:1}) RETURN n")
        self.env.assertEqual(results.result_set, [[node0]])
