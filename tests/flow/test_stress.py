import os
import sys
import time
from pathos.pools import ProcessPool as Pool
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
            return True
        except:
            return False

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
        # skip test if we're running under Valgrind
        if Env().envRunner.debugger is not None:
            Env().skip() # valgrind is not working correctly with replication

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
        indexes = range(CLIENT_COUNT)
        pool = Pool(nodes=CLIENT_COUNT)

        # invoke queries
        m = pool.amap(query_crud, graphs, indexes)

        # wait for processes to return
        m.wait()

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

        pool = Pool(nodes=5)

        t1 = pool.apipe(create_nodes, graphs[0], n_iterations, n_nodes)

        t2 = pool.apipe(delete_nodes, graphs[1], n_iterations, n_nodes/2)

        t3 = pool.apipe(read_nodes, graphs[2], n_iterations)

        t4 = pool.apipe(update_nodes, graphs[3], n_iterations)

        t5 = pool.apipe(BGSAVE_loop, self.env, conn, 3)

        # wait for processes to join
        t1.wait()
        t2.wait()
        t3.wait()
        t4.wait()
        t5.wait()

        # make sure we did not crashed
        conn.ping()
        conn.close()

    # def test02_clean_shutdown(self):
    #     # issue SHUTDOWN while traffic is generated
    #     indexes = range(CLIENT_COUNT)
    #     pool = Pool(nodes=CLIENT_COUNT)

    #     # invoke queries
    #     m = pool.amap(query_crud, graphs, indexes)

    #     # sleep for half a second, allowing threads to kick in
    #     sleep(0.2)

    #     conn = self.env.getConnection()
    #     conn.shutdown()

    #     # wait for processes to return
    #     m.wait()

        # TODO: exit code doesn't seems to work
        # self.env.assertTrue(self.env.checkExitCode())

