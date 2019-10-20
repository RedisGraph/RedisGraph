import os
import sys
from redisgraph import Graph, Node, Edge
from demo import QueryInfo
from collections import Counter

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

GRAPH_ID = "G"
redis_graph = None

class testPath(FlowTestsBase):
    def __init__(self):
        super(testPath, self).__init__()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)

    def path_to_string(self, path):
        str_path = ", ".join([str(obj) for obj in path])
        return str_path

    def setUp(self):
        self.env.flush()
    
    def test_outgoing_edge_array(self):
        q = "CREATE (a:L1)-[:R1 {value:1}]->(b:L1)-[:R1 {value:2}]->(c:L1)"
        redis_graph.query(q)

        q = "MATCH (:L1)-[e:R1*]->(:L1) RETURN e"
        edge01 = Edge(0, "R1", 1, edge_id = 0 , properties={'value':1})
        edge12 = Edge(1, "R1", 2, edge_id = 1 , properties={'value':2})

        path01 = [edge01]
        path12 = [edge12]
        path02 = [edge01, edge12]

        expected_results = [[path01], [path12], [path02]]

        query_info = QueryInfo(query = q, description="Tests outgoing variable length path edge array", expected_result = expected_results)

        self._assert_resultset_equals_expected(redis_graph.query(q), query_info)

    def test_incoming_edge_array(self):
        q = "CREATE (a:L1)-[:R1 {value:1}]->(b:L1)-[:R1 {value:2}]->(c:L1)"
        redis_graph.query(q)

        q = "MATCH (:L1)<-[e:R1*]-(:L1) RETURN e"
        edge01 = Edge(0, "R1", 1, edge_id = 0 , properties={'value':1})
        edge12 = Edge(1, "R1", 2, edge_id = 1 , properties={'value':2})

        path01 = [edge01]
        path12 = [edge12]
        path02 = [edge12, edge01]

        expected_results = [[path01], [path12], [path02]]

        query_info = QueryInfo(query = q, description = "Tests incoming variable length path edge array", expected_result = expected_results)

        self._assert_resultset_equals_expected(redis_graph.query(q), query_info)
        
