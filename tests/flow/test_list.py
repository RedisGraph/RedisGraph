from RLTest import Env
import redis
from redisgraph import Graph, Node, Edge

from base import FlowTestsBase

redis_graph = None
GRAPH_ID = "G"


class testList(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)

    def test01_collect(self):
        for i in range(10):
            redis_graph.add_node(Node())
        redis_graph.commit()

        query = """MATCH (n) RETURN collect(n)"""
        result = redis_graph.query(query)
        result_set = result.result_set
        self.env.assertEquals(len(result_set), 1)
        self.env.assertTrue(all(isinstance(n, Node) for n in result_set[0][0]))

    def test02_unwind(self):
        query = """CREATE ()"""
        redis_graph.query(query)
        query = """unwind(range(0,10)) as x return x"""
        result_set = redis_graph.query(query).result_set
        expected_result = [[0], [1], [2], [3],
                           [4], [5], [6], [7], [8], [9], [10]]
        self.env.assertEquals(result_set, expected_result)

    # List functions should handle null inputs appropriately.
    def test03_null_list_function_inputs(self):
        expected_result = [[None]]

        # NULL list argument to subscript.
        query = """WITH NULL as list RETURN list[0]"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # NULL list argument to slice.
        query = """WITH NULL as list RETURN list[0..5]"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # NULL list argument to HEAD.
        query = """WITH NULL as list RETURN head(list)"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # NULL list argument to TAIL.
        query = """WITH NULL as list RETURN tail(list)"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # NULL list argument to IN.
        query = """WITH NULL as list RETURN 'val' in list"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # NULL list argument to SIZE.
        query = """WITH NULL as list RETURN size(list)"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # NULL subscript argument.
        query = """WITH ['a'] as list RETURN list[NULL]"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # NULL IN non-empty list should return NULL.
        query = """RETURN NULL in ['val']"""
        actual_result = redis_graph.query(query)
        expected_result = [[None]]
        self.env.assertEquals(actual_result.result_set, expected_result)

        # NULL arguments to slice.
        query = """WITH ['a'] as list RETURN list[0..NULL]"""
        actual_result = redis_graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)

        # NULL range argument should produce an error.
        query = """RETURN range(NULL, 5)"""
        try:
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

        # NULL IN empty list should return false.
        query = """RETURN NULL in []"""
        actual_result = redis_graph.query(query)
        expected_result = [[False]]
        self.env.assertEquals(actual_result.result_set, expected_result)
