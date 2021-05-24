import os
import sys
import time
import threading
from time import sleep
from RLTest import Env
from redisgraph import Graph
from base import FlowTestsBase

graphs       = None  # one graph object per client
GRAPH_ID     = "G"   # graph identifier
CLIENT_COUNT = 100   # number of concurrent connections

def query_crud(graph, threadID):
    for i in range(10):
        create_query = "CREATE (n:node {v:'%s'}), (n)-[:have]->({value:'%s'}), (n)-[:have]->({value:'%s'})" % (threadID, threadID, threadID)
        read_query   = "MATCH (n0:node {v:'%s'})<-[:have]-(n:node)-[:have]->(n1:node) return n1.v" % threadID
        update_query = "MATCH (n:node {v: '%s'}) SET n.x = '%s'" % (threadID, threadID)
        delete_query = "MATCH (n:node {v: '%s'})-[:have*]->(n1:node) DELETE n, n1" % threadID

        try:
            graph.query(create_query)
            graph.query(read_query)
            graph.query(update_query)
            graph.query(delete_query)
        except:
            return

# run n_iterations and create n node in each iteration
def create_nodes(graph, n_iterations, n):
    for i in range(n_iterations):
        graph.query("UNWIND range(0, %d) AS x CREATE (:Node {val: x})" % n)

# run n_iterations and delete n in each iteration
def delete_nodes(graph, n_iterations, n):
    for i in range(n_iterations):
        graph.query("MATCH (n) WITH n LIMIT %d DELETE n" % n)

# run n_iterations and update all nodes in each iteration
def update_nodes(graph, n_iterations):
    for i in range(n_iterations):
        graph.query("MATCH (n) SET n.v = 1")

# run n_iterations and execute a read query in each iteration
def read_nodes(graph, n_iterations):
    for i in range(n_iterations):
        graph.query("MATCH (n) return n")

# calls BGSAVE every 0.2 second
def BGSAVE_loop(env, conn, n_iterations):
    results = conn.execute_command("INFO", "persistence")
    cur_bgsave_time = prev_bgsave_time = results['rdb_last_save_time']

    for i in range(n_iterations):
        conn.execute_command("BGSAVE")
        start = time.time()

        while(cur_bgsave_time == prev_bgsave_time):
            # assert and return if the timeout of 5 seconds took place
            if(time.time() - start > 5):
                env.assertTrue(False)
                return

            results = conn.execute_command("INFO", "persistence")
            cur_bgsave_time = results['rdb_last_save_time']
            if cur_bgsave_time == prev_bgsave_time:
                sleep(1) # sleep for 1 second

        prev_bgsave_time = cur_bgsave_time
        env.assertEqual(results['rdb_last_bgsave_status'], "ok")

class testStressFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global graphs
        graphs = []

        for i in range(0, CLIENT_COUNT):
            graphs.append(Graph(GRAPH_ID, self.env.getConnection()))

    def __del__(self):
        for i in range(0, CLIENT_COUNT):
            g = graphs[0]
            g.redis_con.close()

    # Count number of nodes in the graph
    def test00_stress(self):
        threads = []
        for i in range(CLIENT_COUNT):
            graph = graphs[i]
            t = threading.Thread(target=query_crud, args=(graph, i))
            t.setDaemon(True)
            threads.append(t)
            t.start()

        # wait for threads to return
        for i in range(CLIENT_COUNT):
            t = threads[i]
            t.join()

        # make sure we did not crashed
        conn = self.env.getConnection()
        conn.ping()
        conn.close()

    def test01_bgsave_stress(self):
        # skip test if we're running under Valgrind
        if Env().envRunner.debugger is not None:
            Env().skip() # fork doesn't free memory, so valgrind will complain

        n_nodes = 1000
        n_iterations = 10
        conn = self.env.getConnection()
        graphs[0].query("CREATE INDEX ON :Node(val)")

        t1 = threading.Thread(target=create_nodes, args=(graphs[0], n_iterations, n_nodes))
        t1.setDaemon(True)

        t2 = threading.Thread(target=delete_nodes, args=(graphs[1], n_iterations, n_nodes/2))
        t2.setDaemon(True)

        t3 = threading.Thread(target=read_nodes, args=(graphs[2], n_iterations))
        t3.setDaemon(True)

        t4 = threading.Thread(target=update_nodes(graphs[3], n_iterations))
        t4.setDaemon(True)

        t5 = threading.Thread(target=BGSAVE_loop, args=(self.env, conn, 3))
        t5.setDaemon(True)

        t1.start()
        t2.start()
        t3.start()
        t4.start()
        t5.start()

        # wait for threads to join
        t1.join()
        t2.join()
        t3.join()
        t4.join()
        t5.join()

        # make sure we did not crashed
        conn.ping()
        conn.close()

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

