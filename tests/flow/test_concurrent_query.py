import os
import sys
import threading
from RLTest import Env
from redisgraph import Graph, Node, Edge
from redis import ResponseError

from base import FlowTestsBase

GRAPH_ID = "G"                      # Graph identifier.
CLIENT_COUNT = 16                   # Number of concurrent connections.
graphs = None                       # One graph object per client.
assertions = [True] * CLIENT_COUNT  # Each thread places its verdict at position threadID.
exceptions = [None] * CLIENT_COUNT  # Each thread which fails sets its exception content ar position threadID.
people = ["Roi", "Alon", "Ailon", "Boaz", "Tal", "Omri", "Ori"]

def query_aggregate(graph, query, threadID):
    global assertions
    assertions[threadID] = True

    for i in range(10):
        actual_result = graph.query(query)
        person_count = actual_result.result_set[0][0]
        if person_count != len(people):
            assertions[threadID] = False
            break

def query_neighbors(graph, query, threadID):
    global assertions
    assertions[threadID] = True

    # Fully connected graph + header row.
    expected_resultset_size = len(people) * (len(people)-1)

    for i in range(10):
        actual_result = graph.query(query)
        if len(actual_result.result_set) is not expected_resultset_size:
            assertions[threadID] = False
            break

def query_write(graph, query, threadID):
    global assertions
    assertions[threadID] = True
    
    for i in range(10):
        actual_result = graph.query(query)        
        if actual_result.nodes_created != 1 or actual_result.properties_set != 1:
            assertions[threadID] = False
            break

def thread_run_query(graph, query, threadID):
    global assertions
    try:
        assertions[threadID] = graph.query(query)
    except ResponseError as e:
        exceptions[threadID] = str(e)

def delete_graph(graph, threadID):
    global assertions
    assertions[threadID] = True

    # Try to delete graph.
    try:
        graph.delete()
    except:
        # Graph deletion failed.
        assertions[threadID] = False

class testConcurrentQueryFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env()
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
        threads = []
        for i in range(CLIENT_COUNT):
            graph = graphs[i]
            t = threading.Thread(target=query_aggregate, args=(graph, q, i))
            t.setDaemon(True)
            threads.append(t)
            t.start()

        # Wait for threads to return.
        for i in range(CLIENT_COUNT):
            t = threads[i]
            t.join()
            self.env.assertTrue(assertions[i])
    
    # Concurrently get neighbors of every node.
    def test02_retrieve_neighbors(self):
        q = """MATCH (p:person)-[know]->(n:person) RETURN n.name"""
        threads = []
        for i in range(CLIENT_COUNT):
            graph = graphs[i]
            t = threading.Thread(target=query_neighbors, args=(graph, q, i))
            t.setDaemon(True)
            threads.append(t)
            t.start()

        # Wait for threads to return.
        for i in range(CLIENT_COUNT):
            t = threads[i]
            t.join()
            self.env.assertTrue(assertions[i])

    # Concurrent writes
    def test_03_concurrent_write(self):        
        threads = []
        for i in range(CLIENT_COUNT):
            graph = graphs[i]
            q = """CREATE (c:country {id:"%d"})""" % i            
            t = threading.Thread(target=query_write, args=(graph, q, i))
            t.setDaemon(True)
            threads.append(t)
            t.start()

        # Wait for threads to return.
        for i in range(CLIENT_COUNT):
            t = threads[i]
            t.join()
            self.env.assertTrue(assertions[i])
    
    # Try to delete graph multiple times.
    def test_04_concurrent_delete(self):
        threads = []
        for i in range(CLIENT_COUNT):
            graph = graphs[i]
            t = threading.Thread(target=delete_graph, args=(graph, i))
            t.setDaemon(True)
            threads.append(t)
            t.start()
        
        # Wait for threads to return.
        for i in range(CLIENT_COUNT):
            t = threads[i]
            t.join()

        # Exactly one thread should have successfully deleted the graph.
        self.env.assertEquals(assertions.count(True), 1)

    # Try to delete a graph while multiple queries are executing.
    def test_05_concurrent_read_delete(self):
        redis_con = self.env.getConnection()
        
        ##############################################################################################
        # Delete graph via Redis DEL key.
        ##############################################################################################
        self.populate_graph()
        q = """UNWIND (range(0, 10000)) AS x WITH x AS x WHERE (x / 900) = 1 RETURN x"""
        threads = []
        for i in range(CLIENT_COUNT):
            graph = graphs[i]
            t = threading.Thread(target=thread_run_query, args=(graph, q, i))
            t.setDaemon(True)
            threads.append(t)
            t.start()

        redis_con.delete(GRAPH_ID)

        # Wait for threads to return.
        for i in range(CLIENT_COUNT):
            t = threads[i]
            t.join()            
            self.env.assertEquals(assertions[i].result_set[0][0], 900)

        # Make sure Graph is empty, e.g. graph was deleted.
        resultset = graphs[0].query("MATCH (n) RETURN count(n)").result_set
        self.env.assertEquals(resultset[0][0], 0)

        ##############################################################################################        
        # Delete graph via Redis FLUSHALL.
        ##############################################################################################
        self.populate_graph()
        q = """UNWIND (range(0, 10000)) AS x WITH x AS x WHERE (x / 900) = 1 RETURN x"""
        threads = []
        for i in range(CLIENT_COUNT):
            graph = graphs[i]
            t = threading.Thread(target=thread_run_query, args=(graph, q, i))
            t.setDaemon(True)
            threads.append(t)
            t.start()

        redis_con.flushall()

        # Wait for threads to return.
        for i in range(CLIENT_COUNT):
            t = threads[i]
            t.join()            
            self.env.assertEquals(assertions[i].result_set[0][0], 900)

        # Make sure Graph is empty, e.g. graph was deleted.
        resultset = graphs[0].query("MATCH (n) RETURN count(n)").result_set
        self.env.assertEquals(resultset[0][0], 0)

        ##############################################################################################        
        # Delete graph via GRAPH.DELETE.
        ##############################################################################################
        self.populate_graph()
        q = """UNWIND (range(0, 10000)) AS x WITH x AS x WHERE (x / 900) = 1 RETURN x"""
        threads = []
        for i in range(CLIENT_COUNT):
            graph = graphs[i]
            t = threading.Thread(target=thread_run_query, args=(graph, q, i))
            t.setDaemon(True)
            threads.append(t)
            t.start()

        graphs[i].delete()

        # Wait for threads to return.
        for i in range(CLIENT_COUNT):
            t = threads[i]
            t.join()            
            self.env.assertEquals(assertions[i].result_set[0][0], 900)

        # Make sure Graph is empty, e.g. graph was deleted.
        resultset = graphs[0].query("MATCH (n) RETURN count(n)").result_set
        self.env.assertEquals(resultset[0][0], 0)

    def test_06_concurrent_write_delete(self):
        # Test setup - validate that graph exists and possible results are None
        graphs[0].query("MATCH (n) RETURN n")
        assertions[0] = None
        exceptions[0] = None

        redis_con = self.env.getConnection()
        heavy_write_query = """UNWIND(range(0,999999)) as x CREATE(n)"""
        writer = threading.Thread(target=thread_run_query, args=(graphs[0], heavy_write_query, 0))
        writer.setDaemon(True)
        writer.start()
        redis_con.delete(GRAPH_ID)
        writer.join()
        if exceptions[0] is not None:
            self.env.assertEquals(exceptions[0], "Encountered an empty key when opened key " + GRAPH_ID)
        else:
            self.env.assertEquals(1000000, assertions[0].nodes_created)       
    
    def test_07_concurrent_write_rename(self):
        # Test setup - validate that graph exists and possible results are None
        graphs[0].query("MATCH (n) RETURN n")
        assertions[0] = None
        exceptions[0] = None

        redis_con = self.env.getConnection()
        new_graph = GRAPH_ID + "2"
        # Create new empty graph with id GRAPH_ID + "2"
        redis_con.execute_command("GRAPH.QUERY",new_graph, """MATCH (n) return n""", "--compact")
        heavy_write_query = """UNWIND(range(0,999999)) as x CREATE(n)"""
        writer = threading.Thread(target=thread_run_query, args=(graphs[0], heavy_write_query, 0))
        writer.setDaemon(True)
        writer.start()
        redis_con.rename(new_graph, GRAPH_ID)
        writer.join()
        # Possible scenarios:
        # 1. Rename is done before query is sent. The name in the graph context is new_graph, so when upon commit, when trying to open new_graph key, it will encounter an empty key since new_graph is not a valid key. 
        #    Note: As from https://github.com/RedisGraph/RedisGraph/pull/820 this may not be valid since the rename event handler might actually rename the graph key, before the query execution.    
        # 2. Rename is done during query executing, so when commiting and comparing stored graph context name (GRAPH_ID) to the retrived value graph context name (new_graph), the identifiers are not the same, since new_graph value is now stored at GRAPH_ID value.

        possible_exceptions = ["Encountered different graph value when opened key " + GRAPH_ID, 
        "Encountered an empty key when opened key " + new_graph]
        if exceptions[0] is not None:
            self.env.assertContains(exceptions[0], possible_exceptions)
        else:
            self.env.assertEquals(1000000, assertions[0].nodes_created)
        
    def test_08_concurrent_write_replace(self):
        # Test setup - validate that graph exists and possible results are None
        graphs[0].query("MATCH (n) RETURN n")
        assertions[0] = None
        exceptions[0] = None

        redis_con = self.env.getConnection()
        heavy_write_query = """UNWIND(range(0,999999)) as x CREATE(n)"""
        writer = threading.Thread(target=thread_run_query, args=(graphs[0], heavy_write_query, 0))
        writer.setDaemon(True)
        writer.start()
        set_result = redis_con.set(GRAPH_ID, "1")
        writer.join()
        if exceptions[0] is not None:
            # If the SET command attempted to execute while the CREATE query was running,
            # an exception should have been issued.
            self.env.assertEquals(exceptions[0], "Encountered a non-graph value type when opened key " + GRAPH_ID)
        else:
            # Otherwise, both the CREATE query and the SET command should have succeeded.
            self.env.assertEquals(1000000, assertions[0].nodes_created)
            self.env.assertEquals(set_result, True)

    def test_09_concurrent_multiple_readers_after_big_write(self):
        # Test issue #890
        global assertions
        global exceptions
        redis_con = self.env.getConnection()
        redis_graph = Graph("G890", redis_con)
        redis_graph.query("""UNWIND(range(0,999)) as x CREATE()-[:R]->()""")
        read_query = """MATCH (n)-[r:R]->(m) RETURN n, r, m"""
        assertions = [True] * CLIENT_COUNT
        exceptions = [None] * CLIENT_COUNT
        threads = []
        for i in range(CLIENT_COUNT):
            t = threading.Thread(target=thread_run_query, args=(redis_graph, read_query, i))
            t.setDaemon(True)
            threads.append(t)
            t.start()
        
        for i in range(CLIENT_COUNT):
            t = threads[i]
            t.join()
        
        for i in range(CLIENT_COUNT):
            self.env.assertIsNone(exceptions[i])
            self.env.assertEquals(1000, len(assertions[i].result_set))
