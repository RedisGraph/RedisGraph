import os
import sys
import time
import threading
from time import sleep
from RLTest import Env
from redisgraph import Graph
from base import FlowTestsBase

GRAPH_ID = "G"                      # Graph identifier.
CLIENT_COUNT = 100                  # Number of concurrent connections.
conn = None                         # Connection to redis.
graphs = None                       # One graph object per client.

def query_crud(graph, threadID):
    for i in range(10):
        create_query = "CREATE (n:node {v:'%s'}), (n)-[:have]->({value:'%s'}), (n)-[:have]->({value:'%s'})" % (threadID, threadID, threadID)
        read_query = "MATCH (n0:node {v:'%s'})<-[:have]-(n:node)-[:have]->(n1:node) return n1.v" % threadID
        update_query = "MATCH (n:node {v: '%s'}) SET n.x = '%s'" % (threadID, threadID)
        delete_query = "MATCH (n:node {v: '%s'})-[:have*]->(n1:node) DELETE n, n1" % threadID

        try:
            graph.query(create_query)
            graph.query(read_query)
            graph.query(update_query)
            graph.query(delete_query)
        except:
            return

# Run n_iterations and create n_nodes each iteration
def query_create_nodes(graph, n_iterations, n_nodes):
    for i in range(0, n_iterations):
        try:
            graph.query("UNWIND range(%d,%d) as j CREATE (:Node {val:j})" % (i*n_nodes, i*n_nodes + n_nodes))
        except:
            return

# Run n_iterations and update all nodes each iteration
def query_update_nodes(graph, n_iterations):
    for i in range(0, n_iterations):
        try:
            graph.query("MATCH (n:Node {v}) SET n.v = %d" % (i))
        except:
            return

# Run n_iterations and execute a read query each iteration
def query_read_nodes(graph, n_iterations):
    for i in range(0, n_iterations):
            try:
                graph.query("MATCH (n:Node {v}) return n")
            except:
                return

# Calls BGSAVE every 0.2 second
def query_bgsave_loop(env, conn, n_iterations):
    start = time.time()
    results = conn.execute_command("INFO", "persistence")
    cur_bgsave_time = prev_bgsave_time = results['rdb_last_save_time']
    for i in range(0, n_iterations):
        try:
            conn.execute_command("BGSAVE")
        except:
            return

        while(cur_bgsave_time == prev_bgsave_time):
            # Assert and return if the timeout of 15 seconds took place
            if(time.time() - start > 15):
                env.assertTrue(False)
                return
            results = conn.execute_command("INFO", "persistence")
            cur_bgsave_time = results['rdb_last_save_time']
            sleep(1) # sleep for 1 second, allowing threads to kick in

        prev_bgsave_time = cur_bgsave_time
        env.assertEqual(results['rdb_last_bgsave_status'], "ok")

class testStressFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global conn
        global graphs

        graphs = []
        conn = self.env.getConnection()

        for i in range(0, CLIENT_COUNT):
            graphs.append(Graph(GRAPH_ID, self.env.getConnection()))

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
        conn.ping()

    def test01_bgsave_stress(self):
        graph = graphs[0]
        n_nodes = 20
        n_iterations = 10
        graph.query("CREATE INDEX ON :Node(v)")
        t1 = threading.Thread(target=query_create_nodes, args=(graph, n_iterations, n_nodes))
        t1.setDaemon(True)
        t2 = threading.Thread(target=query_read_nodes, args=(graph, n_iterations))
        t2.setDaemon(True)
        t3 = threading.Thread(target=query_bgsave_loop, args=(self.env, conn, 3))
        t3.setDaemon(True)
        t4 = threading.Thread(target=query_update_nodes(graph, n_iterations))

        t1.start()
        t2.start()
        t3.start()
        t4.start()

        # Wait for threads to return.
        t1.join()
        t2.join()
        t3.join()
        t4.join()

        # Make sure we did not crashed.
        conn.ping()

    def test02_clean_shutdown(self):
        # TODO: enable
        return
        # issue SHUTDOWN while traffic is generated
        threads = []
        for i in range(CLIENT_COUNT):
            graph = graphs[i]
            t = threading.Thread(target=query_crud, args=(graph, i))
            t.setDaemon(True)
            threads.append(t)
            t.start()

        # sleep for half a second, allowing threads to kick in
        sleep(0.2)

        conn.shutdown()

        # wait for threads to return
        for i in range(CLIENT_COUNT):
            t = threads[i]
            t.join()

        # TODO: exit code doesn't seems to work
        # self.env.assertTrue(self.env.checkExitCode())

