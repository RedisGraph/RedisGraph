import time
import asyncio
import redis.asyncio
import random_graph
from common import *

GRAPH_ID = "graph_copy"

class testGraphCopy():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        self.redis_con = self.env.getConnection()

    def tearDown(self):
        pass
        self.redis_con.flushall()

    def copy_graph(self, src, dest):
        attempts = 10
        for i in range(0, attempts):
            try:
                self.redis_con.execute_command("GRAPH.COPY", src, dest)
                break
            except redis.exceptions.ResponseError as e:
                # retry in case we didn't managed to fork
                if "failed to create fork process" in repr(e):
                    time.sleep(0.5) # sleep for half a second
                    continue
                else:
                    raise(e)

    # compare two graphs
    # graphs are consider equal if they have the same:
    # 1. nodes
    # 2. edges
    # 3. indicies
    # 4. schemas
    def graphs_eq(self, a, b):
        # queries used to validate graphs equality
        queries = [
            # node count
            "MATCH (n) RETURN count(n)",                             
            # edge count
            "MATCH ()-[e]->() RETURN count(e)",                      
            # indicies
            "CALL db.indexes() YIELD type, label RETURN * ORDER BY type, label",                
            # schemas
            "CALL db.labels() YIELD label RETURN * ORDER BY label",                       
            # schemas
            "CALL db.relationshipTypes() YIELD relationshipType RETURN * ORDER BY relationshipType", 
            # schemas
            "CALL db.propertyKeys() YIELD propertyKey RETURN * ORDER BY propertyKey",           
            # nodes
            "MATCH (n) RETURN n ORDER BY ID(n)",                     
            # edges
            "MATCH ()-[e]->() RETURN e ORDER BY ID(e)"               
        ]

        for q in queries:
            a_res = a.query(q).result_set
            b_res = b.query(q).result_set
            if(a_res != b_res):
                return False

            self.env.assertTrue(a_res == b_res)
        return True

    #--------------------------------------------------------------------------
    # test error reporting
    #--------------------------------------------------------------------------

    def test_copy_none_existing_graph(self):
        # try to copy a none existing graph
        src = 'none_existing'
        dest = 'copy_of_' + src
        
        try:
            # expecting graph copy operation to fail
            self.copy_graph(src, dest)
            self.env.assertFalse(True)
        except:
            pass

    def test_copy_none_graph_key(self):
        # try to copy a key which is not a graph
        src = 'x'
        dest = 'copy_of_' + src

        self.redis_con.set(src, 1)
        try:
            # expecting graph copy operation to fail
            self.copy_graph(src, dest)
            self.env.assertFalse(True)
        except:
            pass
    
    def test_copy_into_existing_key(self):
        # try to copy a graph into an existing key
        src = 'g'
        dest = 'copy_of+' + src

        # create destination key
        self.redis_con.set(dest, 1)

        # create src graph key
        g = Graph(self.redis_con, src)
        g.query("CREATE ()")

        try:
            # expecting graph copy operation to fail
            self.copy_graph(src, dest)
            self.env.assertFalse(True)
        except:
            pass

    #--------------------------------------------------------------------------
    # test different graphs cloning
    #--------------------------------------------------------------------------

    def test_copy_empty_graph(self):
        # create an empty graph
        src = 'empty_graph'
        dest = 'copy_of_' + src

        s = Graph(self.redis_con, src)
        s.query("CREATE (n) DELETE (n)")

        self.copy_graph(src, dest)

        t = Graph(self.redis_con, dest)
        self.env.assertTrue(self.graphs_eq(s, t))

    def test_copy_single_node_graph(self):
        src = 'single_node_graph'
        dest = 'copy_of_' + src
        
        s = Graph(self.redis_con, src)
        s.query("CREATE (:L{v:1})")

        self.copy_graph(src, dest)

        t = Graph(self.redis_con, dest)
        self.env.assertTrue(self.graphs_eq(s, t))

    def test_copy_single_edge_graph(self):
        src = 'single_edge_graph'
        dest = 'copy_of_' + src
        
        s = Graph(self.redis_con, src)
        s.query("CREATE ()-[:R {v:1}]->()")

        self.copy_graph(src, dest)

        t = Graph(self.redis_con, dest)
        self.env.assertTrue(self.graphs_eq(s, t))

    def test_copy_multiedge_graph(self):
        src = 'multi_edge_graph'
        dest = 'copy_of_' + src
        
        s = Graph(self.redis_con, src)
        s.query("CREATE (a)-[:R {v:1}]->(z), (a)-[:R {v:2}]->(z)")

        self.copy_graph(src, dest)

        t = Graph(self.redis_con, dest)
        self.env.assertTrue(self.graphs_eq(s, t))

    def test_copy_graph_with_deleteions(self):
        src = 'deleted_entity_graph'
        dest = 'copy_of_' + src
        
        s = Graph(self.redis_con, src)
        s.query("CREATE (:L {v:1}), (:L{v:2})")
        s.query("MATCH (a:L {v:1}) DELETE a")

        self.copy_graph(src, dest)

        t = Graph(self.redis_con, dest)
        self.env.assertTrue(self.graphs_eq(s, t))

        # make sure 's' and 't' reuse the deleted node
        s_res = s.query("CREATE (:L {v:1})")
        t_res = t.query("CREATE (:L {v:1})")
        self.env.assertTrue(self.graphs_eq(s, t))

    def test_copy_indexed_graph(self):
        src = 'indexed_graph'
        dest = 'copy_of_' + src
        
        s = Graph(self.redis_con, src)
        s.query("CREATE INDEX FOR (n:L) ON (n.v)")

        self.copy_graph(src, dest)

        t = Graph(self.redis_con, dest)
        self.env.assertTrue(self.graphs_eq(s, t))

    def test_copy_random_graph(self):
        src = 'random_graph'
        dest = 'copy_of_' + src
        
        s = Graph(self.redis_con, src)
        nodes, edges = random_graph.create_random_schema()
        random_graph.create_random_graph(s, nodes, edges)
        random_graph.run_random_graph_ops(s, nodes, edges, random_graph.ALL_OPS)

        self.copy_graph(src, dest)

        t = Graph(self.redis_con, dest)
        self.env.assertTrue(self.graphs_eq(s, t))

    def test_copy_of_copy(self):
        # test multiple copies of the same graph 's'
        # t0 = copy(s)
        # t1 = copy(t0)
        # .
        # .
        # .
        # tn = copy(ti)
        # expecting tn == s

        src = 'random_graph_0'
        dest = None
        dest_format = 'copy_{n}_of_' + src
        
        s = Graph(self.redis_con, src)
        nodes, edges = random_graph.create_random_schema()
        random_graph.create_random_graph(s, nodes, edges)
        random_graph.run_random_graph_ops(s, nodes, edges, random_graph.ALL_OPS)

        for i in range(0, 10):
            dest = dest_format.format(n=i)
            self.copy_graph(src, dest)
            src = dest

        # validate last cloned graph is the same as the original graph
        t = Graph(self.redis_con, dest)
        self.env.assertTrue(self.graphs_eq(s, t))

    #--------------------------------------------------------------------------
    # test async graph cloning
    #--------------------------------------------------------------------------

    async def _async_copy_graph(self, con, src, dest, event):
        event.set()
        
        # issue copy and immediately start querying the source graph
        attempts = 10
        for i in range(0, attempts):
            try:
                await con.execute_command("GRAPH.COPY", src, dest)
                break
            except redis.exceptions.ResponseError as e:
                # retry in case we didn't managed to fork
                if "failed to create fork process" in repr(e):
                    await asyncio.sleep(0.5) # sleep for half a second
                    continue
                else:
                    event.clear()
                    raise(e)

        event.clear()

    async def _copy_under_load(self):
        # get an async redis connections
        async_con_0 = redis.asyncio.Redis(host='localhost', port=self.env.port)
        async_con_1 = redis.asyncio.Redis(host='localhost', port=self.env.port)

        src = 'under_load'
        dest = 'copy_of_' + src

        # create a 500K node 250K edge graph
        s = Graph(self.redis_con, src)
        s.query("UNWIND range(0, 250000) AS x CREATE (:A{v:x})-[:R]->(:Z{v:x})")
        node_count = s.query("MATCH (n) RETURN count(n)").result_set[0][0]

        # create an event notify us when graph.copy is about to be called
        copy_start_event = asyncio.Event()

        # asynchronously clone graph
        task = asyncio.create_task(self._async_copy_graph(async_con_0, src, dest, copy_start_event))

        # wait for _async_graph_copy to start
        await copy_start_event.wait()

        # as long as the graph is being copied
        queries_processed = 0
        while copy_start_event.is_set():
            # read
            await async_con_1.execute_command("GRAPH.QUERY", src, "MATCH (a) RETURN a LIMIT 1")
            # write
            await async_con_1.execute_command("GRAPH.QUERY", src, "CREATE ()")
            # read
            await async_con_1.execute_command("GRAPH.QUERY", src, "MATCH (a) RETURN count(a)")
            queries_processed += 3

        t = Graph(self.redis_con, dest)
        # validate at least 1K queries been processed while graph been copied
        self.env.assertGreater(queries_processed, 1000)
        self.env.assertFalse(self.graphs_eq(s, t))

    def test_copy_under_load(self):
        # test graph copy while copied graph is being queried
        asyncio.run(self._copy_under_load())

    async def _del_while_copy(self):
        # get an async redis connections
        async_con_0 = redis.asyncio.Redis(host='localhost', port=self.env.port)
        async_con_1 = redis.asyncio.Redis(host='localhost', port=self.env.port)

        src = 'del_while_copy'
        dest = 'copy_of_' + src

        # create a 500K node 250K edge graph
        s = Graph(self.redis_con, src)
        s.query("UNWIND range(0, 250000) AS x CREATE (:A{v:x})-[:R]->(:Z{v:x})")
        node_count = s.query("MATCH (n) RETURN count(n)").result_set[0][0]

        # create an event notify us when graph.copy is about to be called
        copy_start_event = asyncio.Event()

        # asynchronously clone graph
        task = asyncio.create_task(self._async_copy_graph(async_con_0, src, dest, copy_start_event))

        # wait for _async_graph_copy to start
        await copy_start_event.wait()

        # wait 200ms before issuing graph delete
        await asyncio.sleep(0.2)

        # delete source graph
        await async_con_1.delete(src)

        # wait for _async_graph_copy to finish
        await task

        # validate copied graph
        t = Graph(self.redis_con, dest)
        node_count = t.query("MATCH (n) RETURN count(n)").result_set[0][0]
        edge_count = t.query("MATCH ()-[e]->() RETURN count(e)").result_set[0][0]

        self.env.assertEqual(node_count, 500002)
        self.env.assertEqual(edge_count, 250001)
        # make sure src graph was removed
        self.env.assertFalse(self.redis_con.exists(src))

    def test_del_while_copy(self):
        # test graph copy while during the copy operation the source graph
        # is been asked to be deleted
        asyncio.run(self._del_while_copy())

