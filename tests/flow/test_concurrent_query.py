import os
import sys
import utils.multiproc as mlp
from utils.multiproc import *
from RLTest import Env
from redisgraph import Graph, Node, Edge, query_result
from redis import ResponseError
import inspect

from base import FlowTestsBase

GRAPH_ID = "G"                      # Graph identifier.
CLIENT_COUNT = 16                   # Number of concurrent connections.
graphs = None                       # One graph object per client.
people = ["Roi", "Alon", "Ailon", "Boaz", "Tal", "Omri", "Ori"]

class testConcurrentQueryFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global graphs
        graphs = []
        for i in range(0, CLIENT_COUNT):
            redis_con = self.env.getConnection()
            graphs.append(Graph(GRAPH_ID, redis_con))
        self.populate_graph()

    def populate_graph(self):
        nodes = {}
        graph = graphs[0]

        # Create entities
        for p in people:
            node = Node(label="person", properties={"name": p})
            graph.add_node(node)
            nodes[p] = node

        # Fully connected graph
        for src in nodes:
            for dest in nodes:
                if src != dest:
                    edge = Edge(nodes[src], "know", nodes[dest])
                    graph.add_edge(edge)

        graph.commit()

    # Count number of nodes in the graph
    def test01_concurrent_aggregation(self):
        q = """MATCH (p:person) RETURN count(p)"""
        graph_ids = [(graphs[i].name,) for i in range(CLIENT_COUNT)]
        res = mlp.run_queries_multiproc(self.env, [(q,)]*CLIENT_COUNT, graph_ids, 10)
        for r in res:
            if isinstance(r, Exception):
                raise r
            else:
                person_count = r.result_set[0][0]
                self.env.assertEqual(person_count, len(people))
    
    # Concurrently get neighbors of every node.
    def test02_retrieve_neighbors(self):
        q = """MATCH (p:person)-[know]->(n:person) RETURN n.name"""
        graph_ids = [(graphs[i].name,) for i in range(CLIENT_COUNT)]
        res = mlp.run_queries_multiproc(self.env, [(q,)]*CLIENT_COUNT, graph_ids, 10)
        for r in res:
            if isinstance(r, Exception):
                raise r
            else:
                # Fully connected graph + header row.
                expected_resultset_size = len(people) * (len(people)-1)
                self.env.assertEqual(len(r.result_set), expected_resultset_size)

    # Concurrent writes
    def test_03_concurrent_write(self):
        q = "CREATE (c:country {id:'%d'})"
        graph_ids = [(graphs[i].name,) for i in range(CLIENT_COUNT)]
        res = mlp.run_queries_multiproc(self.env, [(q % i,) for i in range (CLIENT_COUNT)], graph_ids, 10)
        for r in res:
            if isinstance(r, Exception):
                raise r
            else:
                self.env.assertTrue(r.nodes_created == 1 and r.properties_set == 1)
    
    # Try to delete graph multiple times.
    def test_04_concurrent_delete(self):
        res = mlp.run_commands_multiproc(self.env, [(("GRAPH.DELETE", graphs[0].name),)]*CLIENT_COUNT, 1)
        n_succeeded = 0
        for r in res:
            if not isinstance(r, Exception):
                n_succeeded += 1

        # Exactly one thread should have successfully deleted the graph.
        self.env.assertEquals(n_succeeded, 1)

    # Try to delete a graph while multiple queries are executing.
    def test_05_concurrent_read_delete(self):
        redis_con = self.env.getConnection()
        
        ##############################################################################################
        # Delete graph via Redis DEL key.
        ##############################################################################################
        self.populate_graph()
        q = """UNWIND (range(0, 10000)) AS x WITH x AS x WHERE (x / 900) = 1 RETURN x"""
        with Multiproc(self.env, CLIENT_COUNT) as mp:
            async_calls = mp.run_queries_multiproc_no_wait([(q,)]*CLIENT_COUNT, [(graphs[0].name,)]*CLIENT_COUNT, 1)
            redis_con.delete(GRAPH_ID)
            for res in async_calls:
                try:
                    r = res.get()
                    self.env.assertEquals(r.result_set[0][0], 900)
                except Exception as e:
                    pass

        # Make sure Graph is empty, e.g. graph was deleted.
        resultset = graphs[0].query("MATCH (n) RETURN count(n)").result_set
        self.env.assertEquals(resultset[0][0], 0)

        ##############################################################################################        
        # Delete graph via Redis FLUSHALL.
        ##############################################################################################
        self.populate_graph()
        q = """UNWIND (range(0, 10000)) AS x WITH x AS x WHERE (x / 900) = 1 RETURN x"""
        with Multiproc(self.env, CLIENT_COUNT) as mp:
            async_calls = mp.run_queries_multiproc_no_wait([(q,)]*CLIENT_COUNT, [(graphs[0].name,)]*CLIENT_COUNT, 1)
            redis_con.flushall()
            for res in async_calls:
                try:
                    r = res.get()
                    self.env.assertEquals(r.result_set[0][0], 900)
                except Exception as e:
                    pass

        # Make sure Graph is empty, e.g. graph was deleted.
        resultset = graphs[0].query("MATCH (n) RETURN count(n)").result_set
        self.env.assertEquals(resultset[0][0], 0)

        ##############################################################################################        
        # Delete graph via GRAPH.DELETE.
        ##############################################################################################
        self.populate_graph()
        q = """UNWIND (range(0, 10000)) AS x WITH x AS x WHERE (x / 900) = 1 RETURN x"""
        with Multiproc(self.env, CLIENT_COUNT) as mp:
            async_calls = mp.run_queries_multiproc_no_wait([(q,)]*CLIENT_COUNT, [(graphs[0].name,)]*CLIENT_COUNT, 1)
            graphs[0].delete()
            for res in async_calls:
                try:
                    r = res.get()
                    self.env.assertEquals(r.result_set[0][0], 900)
                except Exception as e:
                    pass

        # Make sure Graph is empty, e.g. graph was deleted.
        resultset = graphs[0].query("MATCH (n) RETURN count(n)").result_set
        self.env.assertEquals(resultset[0][0], 0)

    def test_06_concurrent_write_delete(self):
        # Test setup - validate that graph exists and possible results are None
        graphs[0].query("MATCH (n) RETURN n")

        redis_con = self.env.getConnection()
        heavy_write_query = """UNWIND(range(0,999999)) as x CREATE(n)"""
        with Multiproc(self.env, 1) as mp:
            async_calls = mp.run_queries_multiproc_no_wait([(heavy_write_query,)], [(graphs[0].name,)], 1)
            redis_con.delete(GRAPH_ID)
            for res in async_calls:
                try:
                    r = res.get()
                    self.env.assertEquals(1000000, r[0].nodes_created)
                except ResponseError as e:
                    possible_exceptions = ["Encountered different graph value when opened key " + GRAPH_ID,
                                           "Encountered an empty key when opened key " + GRAPH_ID]
                    self.env.assertContains(str(e), possible_exceptions)
 
    
    def test_07_concurrent_write_rename(self):
        # Test setup - validate that graph exists and possible results are None
        graphs[0].query("MATCH (n) RETURN n")

        redis_con = self.env.getConnection()
        new_graph = GRAPH_ID + "2"
        # Create new empty graph with id GRAPH_ID + "2"
        redis_con.execute_command("GRAPH.QUERY",new_graph, """MATCH (n) return n""", "--compact")
        heavy_write_query = """UNWIND(range(0,999999)) as x CREATE(n)"""
        with Multiproc(self.env, 1) as mp:
            async_calls = mp.run_queries_multiproc_no_wait([(heavy_write_query,)], [(graphs[0].name,)], 1)
            redis_con.rename(new_graph, GRAPH_ID)
            # Possible scenarios:
            # 1. Rename is done before query is sent. The name in the graph context is new_graph, so when upon commit, when trying to open new_graph key, it will encounter an empty key since new_graph is not a valid key. 
            #    Note: As from https://github.com/RedisGraph/RedisGraph/pull/820 this may not be valid since the rename event handler might actually rename the graph key, before the query execution.    
            # 2. Rename is done during query executing, so when commiting and comparing stored graph context name (GRAPH_ID) to the retrived value graph context name (new_graph), the identifiers are not the same, since new_graph value is now stored at GRAPH_ID value.

            for res in async_calls:
                try:
                    r = res.get()
                    self.env.assertEquals(1000000, r[0].nodes_created)
                except ResponseError as e:
                    possible_exceptions = ["Encountered different graph value when opened key " + GRAPH_ID, 
                    "Encountered an empty key when opened key " + new_graph]
                    self.env.assertContains(str(e), possible_exceptions)

    def test_08_concurrent_write_replace(self):
        # Test setup - validate that graph exists and possible results are None
        graphs[0].query("MATCH (n) RETURN n")

        redis_con = self.env.getConnection()
        heavy_write_query = """UNWIND(range(0,999999)) as x CREATE(n)"""
        with Multiproc(self.env, 1) as mp:
            async_calls = mp.run_queries_multiproc_no_wait([(heavy_write_query,)], [(graphs[0].name,)], 1)
            set_result = redis_con.set(GRAPH_ID, "1")

            for res in async_calls:
                try:
                    r = res.get()
                    self.env.assertEquals(1000000, r[0].nodes_created)
                    self.env.assertEquals(set_result, True)
                except ResponseError as e:
                    possible_exceptions = ["Encountered a non-graph value type when opened key " + GRAPH_ID,
                        "WRONGTYPE Operation against a key holding the wrong kind of value"]
                    # If the SET command attempted to execute while the CREATE query was running,
                    # an exception should have been issued.
                    self.env.assertContains(str(e), possible_exceptions)

    def test_09_concurrent_multiple_readers_after_big_write(self):
        # Test issue #890
        redis_con = self.env.getConnection()
        redis_graph = Graph("G890", redis_con)
        redis_graph.query("""UNWIND(range(0,999)) as x CREATE()-[:R]->()""")
        read_query = """MATCH (n)-[r:R]->(m) RETURN n, r, m"""

        res = mlp.run_queries_multiproc(self.env, [(read_query,)]*CLIENT_COUNT, [(redis_graph.name,)]*CLIENT_COUNT, 1)
        for r in res:
            if isinstance(r, Exception):
                raise r
            else:
                self.env.assertEquals(1000, len(r.result_set))
