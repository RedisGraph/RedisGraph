import os
import sys
from redisgraph import Graph, Node, Edge, Path
from demo import QueryInfo
from collections import Counter

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase
sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../')
from demo import QueryInfo

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
    
    def test_edge_array(self):
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

        q = "MATCH (:L1)<-[e:R1*]-(:L1) RETURN e"
        query_info = QueryInfo(query = q, description = "Tests incoming variable length path edge array", expected_result = expected_results)
        self._assert_resultset_equals_expected(redis_graph.query(q), query_info)

    def test_simple_path(self):
        q = "CREATE (a:L1)-[:R1 {value:1}]->(b:L1)-[:R1 {value:2}]->(c:L1)"
        redis_graph.query(q)

        q = "MATCH p=(:L1)-[:R1]->(:L1) RETURN p"
        node0 = Node(node_id=0, label="L1")
        edge01 = Edge(0, "R1", 1, edge_id = 0 , properties={'value':1})
        node1 = Node(node_id=1, label="L1")
        edge12 = Edge(1, "R1", 2, edge_id = 1 , properties={'value':2})
        node2 = Node(node_id=2, label="L1")

        path01 = Path([node0, node1], [edge01])
        path12 = Path([node1, node2], [edge12])
        expected_results=[[path01], [path12]]

        query_info = QueryInfo(query = q, description="Tests simple paths", expected_result = expected_results)
        self._assert_resultset_equals_expected(redis_graph.query(q), query_info)

    def test_variable_length_path(self):
        q = "CREATE (a:L1)-[:R1 {value:1}]->(b:L1)-[:R1 {value:2}]->(c:L1)"
        redis_graph.query(q)

        q = "MATCH p=(:L1)-[:R1*]->(:L1) RETURN p"
        node0 = Node(node_id=0, label="L1")
        edge01 = Edge(0, "R1", 1, edge_id = 0 , properties={'value':1})
        node1 = Node(node_id=1, label="L1")
        edge12 = Edge(1, "R1", 2, edge_id = 1 , properties={'value':2})
        node2 = Node(node_id=2, label="L1")

        path01 = Path([node0, node1], [edge01])
        path12 = Path([node1, node2], [edge12])
        path02 = Path([node0, node1, node2], [edge01, edge12])
        expected_results=[[path01], [path12], [path02]]

        query_info = QueryInfo(query = q, description="Tests variable length paths", expected_result = expected_results)
        self._assert_resultset_equals_expected(redis_graph.query(q), query_info)

    def test_bi_directional_path(self):
        q = "CREATE (a:L1)-[:R1 {value:1}]->(b:L1)-[:R1 {value:2}]->(c:L1)"
        redis_graph.query(q)

        q = "MATCH p=(:L1)-[:R1*]->(:L1)<-[:R1*]-() RETURN p"
        node0 = Node(node_id=0, label="L1")
        node1 = Node(node_id=1, label="L1")
        node2 = Node(node_id=2, label="L1")
        edge01 = Edge(0, "R1", 1, edge_id = 0 , properties={'value':1})
        edge10 = Edge(1, "R1", 0, edge_id = 0 , properties={'value':1})
        edge12 = Edge(1, "R1", 2, edge_id = 1 , properties={'value':2})
        edge21 = Edge(2, "R1", 1, edge_id = 1 , properties={'value':2})

        path010 = Path([node0, node1, node0], [edge01, edge10])
        path0121 = Path([node0, node1, node2, node1], [edge01, edge12, edge21])
        path01210 = Path([node0, node1, node2, node1, node0], [edge01, edge12, edge21, edge10])
        path121 = Path([node1, node2, node1], [edge12, edge21])
        path1210 = Path([node1, node2, node1, node0], [edge12, edge21, edge10])
        expected_results=[[path010], [path0121], [path01210], [path121], [path1210]]

        query_info = QueryInfo(query = q, description="Tests bi directional variable length paths", expected_result = expected_results)
        self._assert_resultset_equals_expected(redis_graph.query(q), query_info)

    def test_bi_directional_path_functions(self):
        q = "CREATE (a:L1)-[:R1 {value:1}]->(b:L1)-[:R1 {value:2}]->(c:L1)"
        redis_graph.query(q)

        q = "MATCH p=(:L1)-[:R1*]->(:L1)<-[:R1*]-() RETURN nodes(p), relationships(p), length(p)"
        node0 = Node(node_id=0, label="L1")
        node1 = Node(node_id=1, label="L1")
        node2 = Node(node_id=2, label="L1")
        edge01 = Edge(0, "R1", 1, edge_id = 0 , properties={'value':1})
        edge10 = Edge(1, "R1", 0, edge_id = 0 , properties={'value':1})
        edge12 = Edge(1, "R1", 2, edge_id = 1 , properties={'value':2})
        edge21 = Edge(2, "R1", 1, edge_id = 1 , properties={'value':2})

        expected_results=[[[node0, node1, node0], [edge01, edge10], 2],
                        [[node0, node1, node2, node1], [edge01, edge12, edge21], 3], 
                        [[node0, node1, node2, node1, node0], [edge01, edge12, edge21, edge10], 4], 
                        [[node1, node2, node1], [edge12, edge21], 2], 
                        [[node1, node2, node1, node0], [edge12, edge21, edge10], 3]]

        query_info = QueryInfo(query = q, description="Tests path functions over bi directional variable length paths", expected_result = expected_results)
        self._assert_resultset_equals_expected(redis_graph.query(q), query_info)
