import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge, Path
from collections import Counter

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase
sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../')
from demo import QueryInfo

GRAPH_ID = "G"
redis_graph = None

class testPath(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)

    def path_to_string(self, path):
        str_path = ", ".join([str(obj) for obj in path])
        return str_path

    def setUp(self):
        self.env.flush()
    
    def test_simple_path(self):
        node0 = Node(node_id=0, label="L1")
        node1 = Node(node_id=1, label="L1")
        node2 = Node(node_id=2, label="L1")
        edge01 = Edge(node0, "R1", node1, edge_id=0, properties={'value': 1})
        edge12 = Edge(node1, "R1", node2, edge_id=1, properties={'value': 2})

        redis_graph.add_node(node0)
        redis_graph.add_node(node1)
        redis_graph.add_node(node2)
        redis_graph.add_edge(edge01)
        redis_graph.add_edge(edge12)

        redis_graph.flush()

        path01 = Path.new_empty_path().add_node(node0).add_edge(edge01).add_node(node1)
        path12 = Path.new_empty_path().add_node(node1).add_edge(edge12).add_node(node2)
        expected_results = [[path01], [path12]]

        query = "MATCH p=(:L1)-[:R1]->(:L1) RETURN p"
        query_info = QueryInfo(query = query, description="Tests simple paths", expected_result = expected_results)
        self._assert_resultset_and_expected_mutually_included(redis_graph.query(query), query_info)

    def test_variable_length_path(self):
        node0 = Node(node_id=0, label="L1")
        node1 = Node(node_id=1, label="L1")
        node2 = Node(node_id=2, label="L1")
        edge01 = Edge(node0, "R1", node1, edge_id=0, properties={'value': 1})
        edge12 = Edge(node1, "R1", node2, edge_id=1, properties={'value': 2})

        redis_graph.add_node(node0)
        redis_graph.add_node(node1)
        redis_graph.add_node(node2)
        redis_graph.add_edge(edge01)
        redis_graph.add_edge(edge12)

        redis_graph.flush()
        

        path01 = Path.new_empty_path().add_node(node0).add_edge(edge01).add_node(node1)
        path12 = Path.new_empty_path().add_node(node1).add_edge(edge12).add_node(node2)
        path02 = Path.new_empty_path().add_node(node0).add_edge(edge01).add_node(node1).add_edge(edge12).add_node(node2)
        expected_results=[[path01], [path12], [path02]]

        query = "MATCH p=(:L1)-[:R1*]->(:L1) RETURN p"
        query_info = QueryInfo(query = query, description="Tests variable length paths", expected_result = expected_results)
        self._assert_resultset_and_expected_mutually_included(redis_graph.query(query), query_info)

    def test_bi_directional_path(self):
        node0 = Node(node_id=0, label="L1")
        node1 = Node(node_id=1, label="L1")
        node2 = Node(node_id=2, label="L1")
        edge01 = Edge(node0, "R1", node1, edge_id=0, properties={'value': 1})
        edge12 = Edge(node1, "R1", node2, edge_id=1, properties={'value': 2})

        redis_graph.add_node(node0)
        redis_graph.add_node(node1)
        redis_graph.add_node(node2)
        redis_graph.add_edge(edge01)
        redis_graph.add_edge(edge12)

        redis_graph.flush()

        # Reverse direction edges which are not part of the graph. Read only values.
        edge10 = Edge(1, "R1", 0, edge_id = 0 , properties={'value':1})
        edge21 = Edge(2, "R1", 1, edge_id = 1 , properties={'value':2})

        path010 = Path.new_empty_path().add_node(node0).add_edge(edge01).add_node(node1).add_edge(edge10).add_node(node0)
        path0121 = Path.new_empty_path().add_node(node0).add_edge(edge01).add_node(node1).add_edge(edge12) \
                                        .add_node(node2).add_edge(edge21).add_node(node1)
        path01210 = Path.new_empty_path().add_node(node0).add_edge(edge01).add_node(node1).add_edge(edge12) \
                                        .add_node(node2).add_edge(edge21).add_node(node1).add_edge(edge10).add_node(node0)
        path121 = Path.new_empty_path().add_node(node1).add_edge(edge12).add_node(node2).add_edge(edge21).add_node(node1)
        path1210 = Path.new_empty_path().add_node(node1).add_edge(edge12).add_node(node2).add_edge(edge21) \
                                        .add_node(node1).add_edge(edge10).add_node(node0)
        expected_results=[[path010], [path0121], [path01210], [path121], [path1210]]
       
        query = "MATCH p=(:L1)-[:R1*]->(:L1)<-[:R1*]-() RETURN p"

        query_info = QueryInfo(query = query, description="Tests bi directional variable length paths", \
                                expected_result = expected_results)
        self._assert_resultset_and_expected_mutually_included(redis_graph.query(query), query_info)

    def test_bi_directional_path_functions(self):
        node0 = Node(node_id=0, label="L1")
        node1 = Node(node_id=1, label="L1")
        node2 = Node(node_id=2, label="L1")
        edge01 = Edge(node0, "R1", node1, edge_id=0, properties={'value': 1})
        edge12 = Edge(node1, "R1", node2, edge_id=1, properties={'value': 2})

        redis_graph.add_node(node0)
        redis_graph.add_node(node1)
        redis_graph.add_node(node2)
        redis_graph.add_edge(edge01)
        redis_graph.add_edge(edge12)

        redis_graph.flush()

        # Reverse direction edges which are not part of the graph. Read only values.
        edge10 = Edge(1, "R1", 0, edge_id = 0 , properties={'value':1})
        edge21 = Edge(2, "R1", 1, edge_id = 1 , properties={'value':2})

        expected_results=[[[node0, node1, node0], [edge01, edge10], 2],
                        [[node0, node1, node2, node1], [edge01, edge12, edge21], 3], 
                        [[node0, node1, node2, node1, node0], [edge01, edge12, edge21, edge10], 4], 
                        [[node1, node2, node1], [edge12, edge21], 2], 
                        [[node1, node2, node1, node0], [edge12, edge21, edge10], 3]]

        query = "MATCH p=(:L1)-[:R1*]->(:L1)<-[:R1*]-() RETURN nodes(p), relationships(p), length(p)"

        query_info = QueryInfo(query = query, description="Tests path functions over bi directional variable length paths", \
                                        expected_result = expected_results)
        self._assert_resultset_and_expected_mutually_included(redis_graph.query(query), query_info)

    def test_zero_length_path(self):
        node0 = Node(node_id=0, label="L1")
        node1 = Node(node_id=1, label="L2")
        edge01 = Edge(node0, "R1", node1, edge_id=0, properties={'value': 1})

        redis_graph.add_node(node0)
        redis_graph.add_node(node1)
        redis_graph.add_edge(edge01)

        redis_graph.flush()

        path01 = Path.new_empty_path().add_node(node0).add_edge(edge01).add_node(node1)
        expected_results=[[path01]]

        query = "MATCH p=(:L1)-[*0..]->()-[]->(:L2) RETURN p"

        query_info = QueryInfo(query = query, description="Tests path with zero length variable length paths", \
                                        expected_result = expected_results)
        self._assert_resultset_and_expected_mutually_included(redis_graph.query(query), query_info)

    def test_path_comparison(self):
        node0 = Node(node_id=0, label="L1")
        node1 = Node(node_id=1, label="L1")
        node2 = Node(node_id=2, label="L1")
        edge01 = Edge(node0, "R1", node1, edge_id=0, properties={'value': 1})
        edge12 = Edge(node1, "R1", node2, edge_id=1, properties={'value': 2})

        redis_graph.add_node(node0)
        redis_graph.add_node(node1)
        redis_graph.add_node(node2)
        redis_graph.add_edge(edge01)
        redis_graph.add_edge(edge12)

        redis_graph.flush()

        path01 = Path.new_empty_path().add_node(node0).add_edge(edge01).add_node(node1)
        path12 = Path.new_empty_path().add_node(node1).add_edge(edge12).add_node(node2)

        # Test a path equality filter
        query = "MATCH p1 = (:L1)-[:R1]->(:L1) MATCH p2 = (:L1)-[:R1]->(:L1) WHERE p1 = p2 RETURN p1"
        expected_results = [[path01],
                            [path12]]

        query_info = QueryInfo(query=query, description="Test path equality", expected_result=expected_results)
        self._assert_resultset_and_expected_mutually_included(redis_graph.query(query), query_info)

        # Test a path inequality filter
        query = "MATCH p1 = (:L1)-[:R1]->(:L1) MATCH p2 = (:L1)-[:R1]->(:L1) WHERE p1 <> p2 RETURN DISTINCT p1, p2"
        expected_results = [[path01, path12],
                            [path12, path01]]

        query_info = QueryInfo(query=query, description="Test path inequality", expected_result=expected_results)
        self._assert_resultset_and_expected_mutually_included(redis_graph.query(query), query_info)
