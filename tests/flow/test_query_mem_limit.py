import random
import utils.multiproc as mlp
from utils.multiproc import *
from itertools import cycle
from RLTest import Env
from redisgraph import Graph

# 1. test reading and setting query memory limit configuration

# 2. test overflowing the server when there's no limit,
#    expect no errors

# 3. test querying the server when there's a high memory limit and the queries
#    are all running under the current limit

# 4. test querying the server when there's a tight memory limit
#    expecting an out of memory error

# 5. test a mixture of queries, ~90% successful ones and the rest are expected
#    to fail due to out of memory error

g                  =  None
GRAPH_NAME         =  "max_query_mem"
MEM_HOG_QUERY      =  """UNWIND range(0, 100000) AS x
                         WITH x
                         WHERE (x / 2) = 50
                         RETURN x, count(x)"""

MEM_THRIFTY_QUERY  =  """UNWIND range(0, 10) AS x
                         WITH x
                         WHERE (x / 2) = 50
                         RETURN x, count(x)"""

def issue_query(q):
    try:
        mlp.con.execute_command("GRAPH.QUERY", GRAPH_NAME, q)
        return True
    except Exception as e:
        assert "Query's mem consumption exceeded capacity" in str(e)
        return False

class testQueryMemoryLimit():
    def __init__(self):
        global g
        self.env = Env(decodeResponses=True)
        self.conn = self.env.getConnection()
        g = Graph(GRAPH_NAME, self.conn)

    def stress_server(self, query, nrep, should_fail):
        res = mlp.run_multiproc(self.env, [issue_query]*nrep, [(query,)]*nrep)
        self.env.assertNotEqual(res[0], should_fail)

    def test_01_read_memory_limit_config(self):
        # read configuration, test default value, expecting unlimited memory cap
        result = self.conn.execute_command("GRAPH.CONFIG", "GET", "QUERY_MEM_CAPACITY")
        query_mem_capacity = result[1]
        self.env.assertEquals(query_mem_capacity, 0)

        # update configuration, set memory limit to 1MB
        MB = 1024*1024
        self.conn.execute_command("GRAPH.CONFIG", "SET", "QUERY_MEM_CAPACITY", MB)

        # re-read configuration
        result = self.conn.execute_command("GRAPH.CONFIG", "GET", "QUERY_MEM_CAPACITY")
        query_mem_capacity = result[1]
        self.env.assertEquals(query_mem_capacity, MB)

    def test_02_overflow_no_limit(self):
        # execute query on each one of the threads
        n_queries_to_execute = self.conn.execute_command("GRAPH.CONFIG", "GET", "THREAD_COUNT")[1]

        # set query memory limit as UNLIMITED
        limit = 0
        self.conn.execute_command("GRAPH.CONFIG", "SET", "QUERY_MEM_CAPACITY", limit) 

        self.stress_server(MEM_HOG_QUERY, n_queries_to_execute, False)

    def test_03_no_overflow_with_limit(self):
        # execute query on each one of the threads
        n_queries_to_execute = self.conn.execute_command("GRAPH.CONFIG", "GET", "THREAD_COUNT")[1]

        # set query memory limit to 1GB
        limit = 1024*1024*1024
        self.conn.execute_command("GRAPH.CONFIG", "SET", "QUERY_MEM_CAPACITY", limit) 

        self.stress_server(MEM_HOG_QUERY, n_queries_to_execute, False)

    def test_04_overflow_with_limit(self):
        # execute query on each one of the threads
        n_queries_to_execute = self.conn.execute_command("GRAPH.CONFIG", "GET", "THREAD_COUNT")[1]

        # set query memory limit to 1MB
        limit = 1024*1024
        self.conn.execute_command("GRAPH.CONFIG", "SET", "QUERY_MEM_CAPACITY", limit)

        self.stress_server(MEM_HOG_QUERY, n_queries_to_execute, True)

    def test_05_test_mixed_queries(self):
        queries = []
        should_fail_list = []
        total_query_count = 100
        
        # Query the threadpool_size
        threadpool_size = self.conn.execute_command("GRAPH.CONFIG", "GET", "THREAD_COUNT")[1]

        # set query memory limit to 1MB
        limit = 1024*1024
        self.conn.execute_command("GRAPH.CONFIG", "SET", "QUERY_MEM_CAPACITY", limit)

        for i in range(total_query_count):
            should_fail = False
            q = MEM_THRIFTY_QUERY
            r = random.randint(0, 100)

            if r <= total_query_count * 0.1: # 10%
                q = MEM_HOG_QUERY
                should_fail = True

            should_fail_list.append(should_fail)
            queries.append((q,))

            res = mlp.run_multiproc(self.env, fns=[issue_query]*len(queries), args=queries)
            for r, should_fail in zip(res, should_fail_list):
                self.env.assertNotEqual(r, should_fail)
