from common import *

redis_graph = None
GRAPH_ID = "merge_test"


class testMerge():
    def __init__(self):
        global redis_graph
        self.env = Env(decodeResponses=True)
        redis_con = self.env.getConnection()
        redis_graph = Graph(redis_con, GRAPH_ID)

    def test01_merge_resue(self):
        query = "CREATE (:L2 {v: 'x'})"
        redis_graph.query(query)

        query = "CREATE (:L2 {v: 'y'})"
        redis_graph.query(query)
        redis_graph.query(query)

        query = """
        MERGE (m:L1 {v: "abc"})
        SET m.v = "abc"
        WITH m
        MATCH (u:L2 {v: "x"})
        MATCH (n:L2 {v: "y"})
        MERGE (u)-[:matched]->(m)<-[:matched]-(n)
        RETURN m, u, n"""

        redis_graph.query(query)

        redis_graph.query(query)

