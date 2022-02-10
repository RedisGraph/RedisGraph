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
                           [],   # collect,
                           None, # percentileDisc
                           None  # percentileCont
                           ]

        query = """MATCH (n) WHERE n.v = 'noneExisting'
                   RETURN count(n), min(n.v), max(n.v), sum(n.v), avg(n.v),
                   stDev(n.v), stDevP(n.v), collect(n),
                   percentileDisc(n.v, 0.5), percentileCont(n.v, 0.5)"""
        result = graph.query(query)
        self.env.assertEquals(result.result_set[0], expected_result)

        # issue a similar query only perform aggregations within a WITH clause
        query = """MATCH (n) WHERE n.v = 'noneExisting'
                   WITH count(n) as A, min(n.v) as B, max(n.v) as C, sum(n.v) as D,
                   avg(n.v) as E, stDev(n.v) as F,  stDevP(n.v) as G,
                   collect(n) as H, percentileDisc(n.v, 0.5) as I,
                   percentileCont(n.v, 0.5) as J
                   RETURN *"""

        result = graph.query(query)
        self.env.assertEquals(result.result_set[0], expected_result)

