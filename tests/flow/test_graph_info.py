from common import *

graph = None
redis_con = None
GRAPH_ID = 'test_graph_info'

INFO_QUERIES_COMMAND = 'GRAPH.INFO QUERIES'
INFO_QUERIES_CURRENT_COMMAND = 'GRAPH.INFO QUERIES CURRENT'
INFO_QUERIES_CURRENT_PREV_COMMAND_1 = 'GRAPH.INFO QUERIES CURRENT PREV 1'
INFO_QUERIES_CURRENT_PREV_COMMAND_5 = 'GRAPH.INFO QUERIES CURRENT PREV 5'

def list_to_dict(lst):
    """Recursively converts a list to a dictionary.
    This is useful when we convert the response of the `execute_command` method
    to a dictionary, since the response is a list of alternating keys and
    values."""

    d = {}
    for i in range(0, len(lst), 2):
        k = lst[i]
        if isinstance(k, list):
            return k
        if i + 1 >= len(lst):
            d[k] = None
            break
        v = lst[i + 1]
        if isinstance(v, list) and len(v) > 0 and isinstance(v[0], str):
            v = list_to_dict(v)
        d[k] = v
    return d

class testGraphInfo(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        self.conn = self.env.getConnection()
        global graph
        global redis_con
        redis_con = self.env.getConnection()
        graph = Graph(redis_con, GRAPH_ID)

    def execute_get_dict(self, query):
        """Executes a query and returns the result as a dictionary"""
        res = graph.execute_command(query)
        return list_to_dict(res)

    def assertEquals(self, a, b):
        self.env.assertEquals(a, b)

    def test01_empty_info(self):
        """Tests that when no queries were executed, the 'CURRENT' info command
        returns an empty list"""

        info = self.conn.execute_command(INFO_QUERIES_CURRENT_COMMAND)
        res_dict = list_to_dict(info)
        self.env.assertEquals(res_dict['Queries'], [])

    def test02_last_query(self):
        """Tests that the last query is returned by the `PREV 1` command"""

        # issue a query
        query = "MATCH (n) RETURN n"
        graph.query(query)
        info = self.execute_get_dict(INFO_QUERIES_CURRENT_PREV_COMMAND_1)
        first_query = list_to_dict(info['Queries'][0])
        self.env.assertEquals(first_query['Query'], query)

        # issue another query
        query = "MATCH (n) RETURN n LIMIT 1"
        graph.query(query)
        info = self.execute_get_dict(INFO_QUERIES_CURRENT_PREV_COMMAND_1)
        first_query = list_to_dict(info['Queries'][0])
        self.assertEquals(first_query['Query'], query)

    def test03_current_query(self):
        """Tests that the current query is returned by the `CURRENT` command"""

        # issue a "heavy" query
        query = "UNWIND range(1, 10000000) as x CREATE (:Node {val: x})"
        graph.query(query)
        info = self.execute_get_dict(INFO_QUERIES_CURRENT_COMMAND)
        first_query = list_to_dict(info['Queries'][0])
        self.env.assertEquals(first_query['Query'], query)
