import os
import sys
import threading
from redisgraph import Graph, Node, Edge

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
