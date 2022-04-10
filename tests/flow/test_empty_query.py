from common import *

graph = None


class testEmptyQuery(FlowTestsBase):
    def __init__(self):
        global graph
        self.env = Env(decodeResponses=True)
        graph = Graph(self.env.getConnection(), 'G')

    def test01_empty_query(self):
        try:
            # execute empty query
            graph.query("")
        except ResponseError as e:
            self.env.assertIn("Error: empty query.", str(e))

    #def test02_query_with_only_params(self):
    #    try:
    #        graph.query("CYPHER v=1")
    #    except ResponseError as e:
    #        self.env.assertIn("Error: could not parse query", str(e))
