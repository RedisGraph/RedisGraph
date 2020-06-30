import os
import sys
import redis
from RLTest import Env
from redisgraph import Graph, Node

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
from base import FlowTestsBase
sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../')
from demo import QueryInfo

GRAPH_ID = "G"
redis_graph = None

class testParams(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)

    def setUp(self):
        self.env.flush()
    
    def test_simple_params(self):
        params = [1, 2.3, "str", True, False, None, [0, 1, 2]]
        query = "RETURN $param"
        for param in params:    
            expected_results = [[param]]
            query_info = QueryInfo(query = query, description="Tests simple params", expected_result = expected_results)
            self._assert_resultset_equals_expected(redis_graph.query(query, {'param': param}), query_info)

    def test_expression_on_param(self):
        params = {'param': 1}
        query = "RETURN $param + 1"
        expected_results = [[2]]
            
        query_info = QueryInfo(query = query, description="Tests expression on param", expected_result = expected_results)
        self._assert_resultset_equals_expected(redis_graph.query(query, params), query_info)

    def test_node_retrival(self):
        p0 = Node(node_id=0, label="Person", properties={'name': 'a'})
        p1 = Node(node_id=1, label="Person", properties={'name': 'b'})
        p2 = Node(node_id=2, label="NoPerson", properties={'name': 'a'})
        redis_graph.add_node(p0)
        redis_graph.add_node(p1)
        redis_graph.add_node(p2)
        redis_graph.flush()

        params = {'name': 'a'}
        query = "MATCH (n :Person {name:$name}) RETURN n"
        expected_results = [[p0]]
            
        query_info = QueryInfo(query = query, description="Tests expression on param", expected_result = expected_results)
        self._assert_resultset_equals_expected(redis_graph.query(query, params), query_info)

    def test_parameterized_skip_limit(self):
        params = {'skip': 1, 'limit': 1}
        query = "UNWIND [1,2,3] AS X RETURN X SKIP $skip LIMIT $limit"
        expected_results = [[2]]
            
        query_info = QueryInfo(query = query, description="Tests skip limit as params", expected_result = expected_results)
        self._assert_resultset_equals_expected(redis_graph.query(query, params), query_info)

        # Set one parameter to non-integer value
        params = {'skip': '1', 'limit': 1}
        try:
            redis_graph.query(query, params)
            assert(False)
        except redis.exceptions.ResponseError as e:
            pass

    def test_missing_parameter(self):
        # Make sure missing parameters are reported back as an error.
        query = "RETURN $missing"
        try:
            redis_graph.query(query)
            assert(False)
        except:
            # Expecting an error.
            pass

        query = "MATCH (a) WHERE a.v = $missing RETURN a"
        try:
            redis_graph.query(query)
            assert(False)
        except:
            # Expecting an error.
            pass

        query = "MATCH (a) SET a.v = $missing RETURN a"
        try:
            redis_graph.query(query)
            assert(False)
        except:
            # Expecting an error.
            pass

    def test_id_scan(self):
        redis_graph.query("CREATE ({val:1})")
        expected_results=[[1]]
        params = {'id' : 0}
        query = "MATCH (n) WHERE id(n)=$id return n.val"
        query_info = QueryInfo(query = query, description="Test id scan with params", expected_result = expected_results)
        self._assert_resultset_equals_expected(redis_graph.query(query, params), query_info)
        query = redis_graph.build_params_header(params) + query
        plan = redis_graph.execution_plan(query)
        self.env.assertIn('NodeByIdSeek', plan)

