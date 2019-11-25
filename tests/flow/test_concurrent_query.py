import os
import sys
import threading
from redisgraph import Graph, Node, Edge
from redis import ResponseError

from base import FlowTestsBase

GRAPH_ID = "G"                      # Graph identifier.
CLIENT_COUNT = 16                   # Number of concurrent connections.
graphs = None                       # One graph object per client.
assertions = [True] * CLIENT_COUNT  # Each thread places its verdict at position threadID.
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
    assertions[threadID] = graph.query(query)

def thread_try_run_query(graph, query, threadID):
    global assertions
    try:
        assertions[threadID] = graph.query(query)
    except ResponseError as e:
        assertions[threadID] = str(e)

def thread_delete_graph(redis_con, graph_id):
    redis_con.delete(graph_id)

def thread_replace_graph(redis_con, graph_id):
    redis_con.set(graph_id, "1")

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
        super(testConcurrentQueryFlow, self).__init__()
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
        redis_con = self.env.getConnection()
        heavy_write_query = """UNWIND(range(0,999999)) as x CREATE(n)"""
        threads = []
        t = threading.Thread(target=thread_try_run_query, args=(graphs[0], heavy_write_query, 0))
        t.setDaemon(True)
        threads.append(t)
        t = threading.Thread(target=thread_delete_graph, args=(redis_con, GRAPH_ID))
        t.setDaemon(True)
        threads.append(t)
        for thread in threads:
            thread.start()
        for thread in threads:
            thread.join()
        self.env.assertEquals(assertions[0], "Encountered an empty key when opened key " + GRAPH_ID)
    
    def test_07_concurrent_write_replace(self):
        redis_con = self.env.getConnection()
        heavy_write_query = """UNWIND(range(0,999999)) as x CREATE(n)"""
        threads = []
        t = threading.Thread(target=thread_try_run_query, args=(graphs[0], heavy_write_query, 0))
        t.setDaemon(True)
        threads.append(t)
        t = threading.Thread(target=thread_replace_graph, args=(redis_con, GRAPH_ID))
        t.setDaemon(True)
        threads.append(t)
        for thread in threads:
            thread.start()
        for thread in threads:
            thread.join()
        self.env.assertEquals(assertions[0], "Encountered a non-graph value type when opened key " + GRAPH_ID)





