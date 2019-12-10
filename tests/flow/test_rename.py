import os
import sys
from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../')
from demo import QueryInfo

graph = None
redis_con = None
GRAPH_ID = "G"
NEW_GRAPH_ID = "G2"

class testGraphRename(FlowTestsBase):
    def __init__(self):
        super(testGraphRename, self).__init__()
        global graph
        global redis_con
        redis_con = self.env.getConnection()
        graph = Graph(GRAPH_ID, redis_con)
    
    def test00_test_data_valid_after_rename(self):
        global graph
        node0 = Node(node_id=0, label="L", properties={'name':'x', 'age':1})
        graph.add_node(node0)
        graph.flush()
        redis_con.rename(GRAPH_ID, NEW_GRAPH_ID)

        graph = Graph(NEW_GRAPH_ID, redis_con)

        query = "MATCH (n) return n"
        expected_results = [[node0]]
        query_info = QueryInfo(query = query, description="Tests data is valid after renaming", expected_result = expected_results)
        self._assert_resultset_equals_expected(graph.query(query), query_info)
