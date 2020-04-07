import os
import sys
import threading
from RLTest import Env
from redisgraph import Graph
from base import FlowTestsBase

GRAPH_ID = "G"                      # Graph identifier.
CLIENT_COUNT = 100                  # Number of concurrent connections.
graphs = None                       # One graph object per client.

def query_crud(graph, threadID):
    for i in range(10):
        create_query = "CREATE (n:node {v:'%s'}), (n)-[:have]->({value:'%s'}), (n)-[:have]->({value:'%s'})" % (threadID, threadID, threadID)
        read_query = "MATCH (n0:node {v:'%s'})<-[:have]-(n:node)-[:have]->(n1:node) return n1.v" % threadID
        update_query = "MATCH (n:node {v: '%s'}) SET n.x = '%s'" % (threadID, threadID)
        delete_query = "MATCH (n:node {v: '%s'})-[:have*]->(n1:node) DELETE n, n1" % threadID

        graph.query(create_query)
        graph.query(read_query)
        graph.query(update_query)
        graph.query(delete_query)

class testStressFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global graphs
        graphs = []
        for i in range(0, CLIENT_COUNT):
            redis_con = self.env.getConnection()
            graphs.append(Graph(GRAPH_ID, redis_con))

    # Count number of nodes in the graph
    def test00_stress(self):
        threads = []
        for i in range(CLIENT_COUNT):
            graph = graphs[i]
            t = threading.Thread(target=query_crud, args=(graph, i))
            t.setDaemon(True)
            threads.append(t)
            t.start()

        # Wait for threads to return.
        for i in range(CLIENT_COUNT):
            t = threads[i]
            t.join()

        # Make sure we did not crashed.
        redis_con = graphs[0].redis_con
        redis_con.ping()
