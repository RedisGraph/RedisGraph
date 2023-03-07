from common import *

GRAPH_ID = "graph_memory"

class testGraphMemUsage():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        self.redis_con = self.env.getConnection()
        self.graph = Graph(self.redis_con, GRAPH_ID)

    def graph_memory_usage(self):
        return self.redis_con.execute_command("MEMORY USAGE " + GRAPH_ID)

    def test_graph_memory_usage(self):
        # create an empty graph
        query = "RETURN 1"
        result = self.graph.query(query)

        # get empty graph memory usage
        mem_usage = self.graph_memory_usage()

        # introduce a number of entities
        # expecting memory usage to increase
        query = """UNWIND range(0, 1000) as x CREATE (:N{v:x})"""
        self.graph.query(query)
        self.env.assertGreater(self.graph_memory_usage(), mem_usage)
        mem_usage = self.graph_memory_usage()

        # form a few edges
        # expecting memory usage to increase
        query = """MATCH (a), (b) WITH a, b LIMIT 100 CREATE (a)-[:R]->(b)"""
        self.graph.query(query)
        self.env.assertGreater(self.graph_memory_usage(), mem_usage)
        mem_usage = self.graph_memory_usage()

        # add a number of attributes
        # expecting memory usage to increase
        query = """MATCH (a) WITH a LIMIT 100 SET a.wishlist = 'mud flaps'"""
        self.graph.query(query)
        self.env.assertGreater(self.graph_memory_usage(), mem_usage)
        mem_usage = self.graph_memory_usage()

        # make sure indices effect graph's memory usage
        # expecting memory usage to increase
        query = """CREATE INDEX FOR (n:N) ON (n.v)"""
        self.graph.query(query)
        self.env.assertGreater(self.graph_memory_usage(), mem_usage)
        mem_usage = self.graph_memory_usage()

        # removing an index should reduce graph's memory usage
        query = """DROP INDEX ON :N(v)"""
        self.graph.query(query)
        self.env.assertLess(self.graph_memory_usage(), mem_usage)
        mem_usage = self.graph_memory_usage()

        # delete all graph entities
        # expecting memory usage to decrease
        query = """MATCH (a) DELETE a"""
        self.graph.query(query)
        self.env.assertLess(self.graph_memory_usage(), mem_usage)
        mem_usage = self.graph_memory_usage()

