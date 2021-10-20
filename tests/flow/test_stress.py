import os
import time
from time import sleep
from RLTest import Env
from redisgraph import Graph
from base import FlowTestsBase
from pathos.pools import ProcessPool as Pool

graphs       = None  # one graph object per client
GRAPH_ID     = "G"   # graph identifier

def query_crud(graph, query_id):
    for i in range(10):
        create_query = "CREATE (n:node {v:'%s'}), (n)-[:have]->({value:'%s'}), (n)-[:have]->({value:'%s'})" % (query_id, query_id, query_id)
        read_query   = "MATCH (n0:node {v:'%s'})<-[:have]-(n:node)-[:have]->(n1:node) return n1.v" % query_id
        update_query = "MATCH (n:node {v: '%s'}) SET n.x = '%s'" % (query_id, query_id)
        delete_query = "MATCH (n:node {v: '%s'})-[:have*]->(n1:node) DELETE n, n1" % query_id

        try:
            graph.query(create_query)
            graph.query(read_query)
            graph.query(update_query)
            graph.query(delete_query)
            return True
        except:
            return False

# run n_iterations and create n node in each iteration
def create_nodes(graph, n_iterations):
    for i in range(n_iterations):
        graph.query("CREATE (:Node {val: %d})-[:R]->()" % i)

# run n_iterations and delete n in each iteration
def delete_nodes(graph, n_iterations):
    for i in range(n_iterations):
        graph.query("MATCH (n:Node) WITH n LIMIT 1 DELETE n")

# run n_iterations and update all nodes in each iteration
def update_nodes(graph, n_iterations):
    for i in range(n_iterations):
        graph.query("MATCH (n:Node) WITH n LIMIT 1 SET n.v = 1")

# run n_iterations and execute a read query in each iteration
def read_nodes(graph, n_iterations):
    for i in range(n_iterations):
        graph.query("MATCH (n:Node)-[:R]->() RETURN n LIMIT 1")

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
        # skip test if we're running under Valgrind
        if self.env.envRunner.debugger is not None or os.getenv('COV') == '1':
            self.env.skip() # valgrind is not working correctly with multi process

        global graphs
        graphs = []

        self.client_count = self.env.getConnection().execute_command("GRAPH.CONFIG", "GET", "THREAD_COUNT")[1] * 5

        for i in range(0, self.client_count):
            graphs.append(Graph(GRAPH_ID, self.env.getConnection()))

    def __del__(self):
        if self.env.envRunner.debugger is not None or os.getenv('COV') == '1':
            return

        for i in range(0, self.client_count):
            g = graphs[0]
            g.redis_con.close()

    # Count number of nodes in the graph
    def test00_stress(self):
        ids = range(self.client_count)
        pool = Pool(nodes=self.client_count)

        # invoke queries
        pool.map(query_crud, graphs, ids)

        # make sure we did not crashed
        conn = self.env.getConnection()
        conn.ping()
        conn.close()

    def test01_bgsave_stress(self):
        n_reads = 50000
        n_creations = 50000
        n_updates = n_creations/10
        n_deletions = n_creations/2

        conn = self.env.getConnection()
        graphs[0].query("CREATE INDEX ON :Node(val)")

        pool = Pool(nodes=5)

        t1 = pool.apipe(create_nodes, graphs[0], n_creations)

        t2 = pool.apipe(delete_nodes, graphs[1], n_deletions)

        t3 = pool.apipe(read_nodes, graphs[2], n_reads)

        t4 = pool.apipe(update_nodes, graphs[3], n_updates)

        t5 = pool.apipe(BGSAVE_loop, self.env, conn, 10000)

        # wait for processes to join
        t1.wait()
        t2.wait()
        t3.wait()
        t4.wait()
        t5.wait()

        # make sure we did not crashed
        conn.ping()
        conn.close()

    def test02_clean_shutdown(self):
        # skip test if we're running under COV=1
        if os.getenv('COV') == '1':
            self.env.skip() # valgrind is not working correctly with multi process

        # issue SHUTDOWN while traffic is generated
        indexes = range(self.client_count)
        pool = Pool(nodes=self.client_count)

        # invoke queries
        m = pool.amap(query_crud, graphs, indexes)

        # sleep for half a second, allowing threads to kick in
        sleep(0.2)

        self.env.stop()

        # wait for processes to return
        m.wait()

        self.env.assertTrue(self.env.checkExitCode())

