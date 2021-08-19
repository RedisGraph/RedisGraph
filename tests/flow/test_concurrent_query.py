import os
from RLTest import Env
from base import FlowTestsBase
from redis import ResponseError
from redisgraph import Graph, Node, Edge
from pathos.pools import ProcessPool as Pool

GRAPH_ID = "G"                      # Graph identifier.
CLIENT_COUNT = 16                   # Number of concurrent connections.
graphs = None                       # One graph object per client.
people = ["Roi", "Alon", "Ailon", "Boaz", "Tal", "Omri", "Ori"]

def query_aggregate(graph, query):
    for i in range(10):
        actual_result = graph.query(query)
        person_count = actual_result.result_set[0][0]
        if person_count != len(people):
            return False
    
    return True

def query_neighbors(graph, query):
    # Fully connected graph + header row.
    expected_resultset_size = len(people) * (len(people)-1)

    for i in range(10):
        actual_result = graph.query(query)
        if len(actual_result.result_set) is not expected_resultset_size:
            return False
    
    return True

def query_write(graph, query):
    for i in range(10):
        actual_result = graph.query(query)        
        if actual_result.nodes_created != 1 or actual_result.properties_set != 1:
            return False

    return True

def thread_run_query(graph, query):
    try:
        return graph.query(query)
    except ResponseError as e:
        return str(e)

def delete_graph(graph):
    # Try to delete graph.
    try:
        graph.delete()
        return True
    except:
        # Graph deletion failed.
        return False

def run_concurrent(env, queries, f):
    pool = Pool(nodes=CLIENT_COUNT)

    # invoke queries
    result = pool.map(f, graphs, queries)

    # validate all process return true
    env.assertTrue(all(result))

class testConcurrentQueryFlow(FlowTestsBase):
    def __init__(self):
        # skip test if we're running under Valgrind
        if Env().envRunner.debugger is not None:
            Env().skip() # valgrind is not working correctly with multi processing

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
        queries = [q] * CLIENT_COUNT
        run_concurrent(self.env, queries, query_aggregate)
    
    # Concurrently get neighbors of every node.
    def test02_retrieve_neighbors(self):
        q = """MATCH (p:person)-[know]->(n:person) RETURN n.name"""
        queries = [q] * CLIENT_COUNT
        run_concurrent(self.env, queries, query_neighbors)

    # Concurrent writes
    def test_03_concurrent_write(self):        
        queries = ["""CREATE (c:country {id:"%d"})""" % i for i in range(CLIENT_COUNT)]
        run_concurrent(self.env, queries, query_write)
    
    # Try to delete graph multiple times.
    def test_04_concurrent_delete(self):
        pool = Pool(nodes=CLIENT_COUNT)

        # invoke queries
        assertions = pool.map(delete_graph, graphs)

        # Exactly one thread should have successfully deleted the graph.
        self.env.assertEquals(assertions.count(True), 1)

    # Try to delete a graph while multiple queries are executing.
    def test_05_concurrent_read_delete(self):
        redis_con = self.env.getConnection()
        
        ##############################################################################################
        # Delete graph via Redis DEL key.
        ##############################################################################################
        self.populate_graph()
        pool = Pool(nodes=CLIENT_COUNT)

        q = """UNWIND (range(0, 10000)) AS x WITH x AS x WHERE (x / 900) = 1 RETURN x"""
        queries = [q] * CLIENT_COUNT
        # invoke queries
        m = pool.amap(thread_run_query, graphs, queries)

        redis_con.delete(GRAPH_ID)

        # wait for processes to return
        m.wait()

        # get the results
        result = m.get()

        # validate result.
        self.env.assertTrue(all([r.result_set[0][0] == 900 for r in result]))

        # Make sure Graph is empty, e.g. graph was deleted.
        resultset = graphs[0].query("MATCH (n) RETURN count(n)").result_set
        self.env.assertEquals(resultset[0][0], 0)

        ##############################################################################################        
        # Delete graph via Redis FLUSHALL.
        ##############################################################################################
        self.populate_graph()
        q = """UNWIND (range(0, 10000)) AS x WITH x AS x WHERE (x / 900) = 1 RETURN x"""
        queries = [q] * CLIENT_COUNT
        # invoke queries
        m = pool.amap(thread_run_query, graphs, queries)

        redis_con.flushall()

        # wait for processes to return
        m.wait()

        # get the results
        result = m.get()

        # validate result.
        self.env.assertTrue(all([r.result_set[0][0] == 900 for r in result]))

        # Make sure Graph is empty, e.g. graph was deleted.
        resultset = graphs[0].query("MATCH (n) RETURN count(n)").result_set
        self.env.assertEquals(resultset[0][0], 0)

        ##############################################################################################        
        # Delete graph via GRAPH.DELETE.
        ##############################################################################################
        self.populate_graph()
        q = """UNWIND (range(0, 10000)) AS x WITH x AS x WHERE (x / 900) = 1 RETURN x"""
        queries = [q] * CLIENT_COUNT
        # invoke queries
        m = pool.amap(thread_run_query, graphs, queries)

        graphs[-1].delete()

        # wait for processes to return
        m.wait()

        # get the results
        result = m.get()

        # validate result.
        self.env.assertTrue(all([r.result_set[0][0] == 900 for r in result]))

        # Make sure Graph is empty, e.g. graph was deleted.
        resultset = graphs[0].query("MATCH (n) RETURN count(n)").result_set
        self.env.assertEquals(resultset[0][0], 0)

    def test_06_concurrent_write_delete(self):
        # Test setup - validate that graph exists and possible results are None
        graphs[0].query("MATCH (n) RETURN n")

        pool = Pool(nodes=1)
        redis_con = self.env.getConnection()
        heavy_write_query = """UNWIND(range(0,999999)) as x CREATE(n)"""
        writer = pool.apipe(thread_run_query, graphs[0], heavy_write_query)
        redis_con.delete(GRAPH_ID)
        writer.wait()
        possible_exceptions = ["Encountered different graph value when opened key " + GRAPH_ID,
                               "Encountered an empty key when opened key " + GRAPH_ID]
        result = writer.get()
        if isinstance(result, str):
            self.env.assertContains(result, possible_exceptions)
        else:
            self.env.assertEquals(1000000, result.nodes_created)
    
    def test_07_concurrent_write_rename(self):
        # Test setup - validate that graph exists and possible results are None
        graphs[0].query("MATCH (n) RETURN n")

        pool = Pool(nodes=1)
        redis_con = self.env.getConnection()
        new_graph = GRAPH_ID + "2"
        # Create new empty graph with id GRAPH_ID + "2"
        redis_con.execute_command("GRAPH.QUERY",new_graph, """MATCH (n) return n""", "--compact")
        heavy_write_query = """UNWIND(range(0,999999)) as x CREATE(n)"""
        writer = pool.apipe(thread_run_query, graphs[0], heavy_write_query)
        redis_con.rename(GRAPH_ID, new_graph)
        writer.wait()
        # Possible scenarios:
        # 1. Rename is done before query is sent. The name in the graph context is new_graph, so when upon commit, when trying to open new_graph key, it will encounter an empty key since new_graph is not a valid key. 
        #    Note: As from https://github.com/RedisGraph/RedisGraph/pull/820 this may not be valid since the rename event handler might actually rename the graph key, before the query execution.    
        # 2. Rename is done during query executing, so when commiting and comparing stored graph context name (GRAPH_ID) to the retrived value graph context name (new_graph), the identifiers are not the same, since new_graph value is now stored at GRAPH_ID value.

        possible_exceptions = ["Encountered different graph value when opened key " + GRAPH_ID, 
        "Encountered an empty key when opened key " + new_graph]

        result = writer.get()
        if isinstance(result, str):
            self.env.assertContains(result, possible_exceptions)
        else:
            self.env.assertEquals(1000000, result.nodes_created)
        
    def test_08_concurrent_write_replace(self):
        # Test setup - validate that graph exists and possible results are None
        graphs[0].query("MATCH (n) RETURN n")

        pool = Pool(nodes=1)
        redis_con = self.env.getConnection()
        heavy_write_query = """UNWIND(range(0,999999)) as x CREATE(n)"""
        writer = pool.apipe(thread_run_query, graphs[0], heavy_write_query)
        set_result = redis_con.set(GRAPH_ID, "1")
        writer.wait()
        possible_exceptions = ["Encountered a non-graph value type when opened key " + GRAPH_ID,
                               "WRONGTYPE Operation against a key holding the wrong kind of value"]

        result = writer.get()
        if isinstance(result, str):
            # If the SET command attempted to execute while the CREATE query was running,
            # an exception should have been issued.
            self.env.assertContains(result, possible_exceptions)
        else:
            # Otherwise, both the CREATE query and the SET command should have succeeded.
            self.env.assertEquals(1000000, result.nodes_created)
            self.env.assertEquals(set_result, True)

    def test_09_concurrent_multiple_readers_after_big_write(self):
        # Test issue #890
        redis_graphs = []
        for i in range(0, CLIENT_COUNT):
            redis_con = self.env.getConnection()
            redis_graphs.append(Graph("G890", redis_con))
        redis_graphs[0].query("""UNWIND(range(0,999)) as x CREATE()-[:R]->()""")
        read_query = """MATCH (n)-[r:R]->(m) RETURN n, r, m"""

        queries = [read_query] * CLIENT_COUNT
        pool = Pool(nodes=CLIENT_COUNT)

        # invoke queries
        m = pool.amap(thread_run_query, redis_graphs, queries)

        # wait for processes to return
        m.wait()

        # get the results
        result = m.get()
        
        for i in range(CLIENT_COUNT):
            if isinstance(result[i], str):
                self.env.assertIsNone(result[i])
            else:
                self.env.assertEquals(1000, len(result[i].result_set))
