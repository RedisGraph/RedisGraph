import os
import sys
from redisgraph import Graph, Node, Edge
from collections import Counter

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase
sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../')
from demo import QueryInfo

GRAPH_ID = "G"
redis_graph = None

class testPathFilter(FlowTestsBase):
    def __init__(self):
        super(testPathFilter, self).__init__()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)



    def setUp(self):
        self.env.flush()

    def test00_semi_apply(self):
        node0 = Node(node_id=0, label="L")
        node1 = Node(node_id=1, label="L", properties={'x':1})
        edge01 = Edge(src_node=node0, dest_node=node1, relation="R")
        redis_graph.add_node(node0)
        redis_graph.add_node(node1)
        redis_graph.add_edge(edge01)
        redis_graph.flush()

        query = "MATCH(n:L) WHERE (n)-[:R]->(:L) RETURN n"
        result_set = redis_graph.query(query)
        expected_results = [[node0]]
        query_info = QueryInfo(query = query, description="Tests simple path filter", expected_result = expected_results)
        self._assert_resultset_equals_expected(result_set, query_info)




