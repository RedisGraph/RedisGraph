from RLTest import Env
from redisgraph import Graph, Node, Edge

graph = None
GRAPH_ID = "aggregations"

class testAggregations():
    def __init__(self):
        global graph
        self.env = Env(decodeResponses=True)
        graph = Graph(GRAPH_ID, self.env.getConnection())

    # test aggregation default values
    # default values should be returned when the aggregation operation
    # was not given any data to process
    # and the aggregation doesn't specify any keys
    def test01_empty_aggregation(self):
        # default aggregation values
        expected_result = [0,    # count
                           None, # min
                           None, # max
                           0,    # sum
                           None, # avg
                           0,    # stDev
                           0,    # stDevP
                           []    # collect
                           ]

        query = """MATCH (n) WHERE n.v = 'noneExisting'
                   RETURN count(n), min(n), max(n), sum(n.v), avg(n.v),
                   stDev(n.v), stDevP(n.v), collect(n)"""

        result = graph.query(query)
        self.env.assertEquals(result.result_set[0], expected_result)

        # issue a similar query only perform aggregations within a WITH clause
        query = """MATCH (n) WHERE n.v = 'noneExisting'
                   WITH count(n) as, min(n), max(n), sum(n.v), avg(n.v),
                   stDev(n.v), stDevP(n.v), collect(n)
                   RETURN *"""
        result = graph.query(query)
        self.env.assertEquals(result.result_set[0], expected_result)

