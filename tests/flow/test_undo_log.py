from RLTest import Env
from redisgraph import Graph

GRAPH_ID = "undo-log"

class testUndoLog():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        self.redis_con = self.env.getConnection()
        self.graph = Graph(GRAPH_ID, self.redis_con)

    def tearDown(self):
        self.redis_con.flushall()

    def test01_undo_create_node(self):
        try:
            self.graph.query("CREATE (n:N) WITH n RETURN 1 * 'a'")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # node (n:N) should be removed, expecting an empty graph
        result = self.graph.query("MATCH (n:N) RETURN COUNT(n)")
        self.env.assertEquals(result.result_set[0][0], 0)

    def test02_undo_create_edge(self):
        self.graph.query("CREATE (:N {v: 1}), (:N {v: 2})")
        try:
            self.graph.query("""MATCH (s:N {v: 1}), (t:N {v: 2})
                                CREATE (s)-[r:R]->(t)
                                WITH r
                                RETURN 1 * 'a'""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # edge [r:R] should have been removed
        result = self.graph.query("MATCH ()-[r:R]->() RETURN COUNT(r)")
        self.env.assertEquals(result.result_set[0][0], 0)

    def test03_undo_delete_node(self):
        self.graph.query("CREATE (:N)")
        try:
            self.graph.query("""MATCH (n:N)
                                DELETE n
                                WITH n
                                RETURN 1 * 'a'""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # deleted node should be revived, expecting a single node
        result = self.graph.query("MATCH (n:N) RETURN COUNT(n)")
        self.env.assertEquals(result.result_set[0][0], 1)

    def test04_undo_delete_edge(self):
        self.graph.query("CREATE (:N)-[:R]->(:N)")
        try:
            self.graph.query("""MATCH ()-[r:R]->()
                                DELETE r
                                WITH r 
                                RETURN 1 * 'a'""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # deleted edge should be revived, expecting a single edge
        result = self.graph.query("MATCH ()-[r:R]->() RETURN COUNT(r)")
        self.env.assertEquals(result.result_set[0][0], 1)

    def test05_undo_update_node(self):
        self.graph.query("CREATE (:N {a: 1, b:'str', c:[1, 'str', point({latitude:1, longitude:2})], d:point({latitude:1, longitude:2})})")
        try:
            self.graph.query("""MATCH (n:N {a: 1})
                                SET n.a = 2, n.b = '', n.c = null, n.d = point({latitude:2, longitude:1})
                                WITH n
                                RETURN 1 * 'a'""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # expecting the original attributes to be restored
        result = self.graph.query("MATCH (n:N) RETURN n.a, n.b, n.c, n.d")
        self.env.assertEquals(result.result_set[0][0], 1)
        self.env.assertEquals(result.result_set[0][1], 'str')
        self.env.assertEquals(result.result_set[0][2], [1, 'str', {'latitude':1, 'longitude':2}])
        self.env.assertEquals(result.result_set[0][3], {'latitude':1, 'longitude':2})

        try:
            self.graph.query("""MATCH (n:N {a: 1})
                                SET n.e = 1
                                WITH n
                                RETURN 1 * 'a'""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # expecting the original attributes to be deleted
        result = self.graph.query("MATCH (n:N) RETURN n.e")
        self.env.assertEquals(result.result_set[0][0], None)

        try:
            self.graph.query("""MATCH (n:N {a: 1})
                                SET n:M
                                WITH n
                                RETURN 1 * 'a'""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # expecting the additional label 'M' to be removed
        result = self.graph.query("MATCH (n:M) RETURN COUNT(n)")
        self.env.assertEquals(result.result_set[0][0], 0)

        try:
            self.graph.query("""MATCH (n:N {a: 1})
                                SET n = {}
                                WITH n
                                RETURN 'a' * 1""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # expecting the original attributes to be restored
        result = self.graph.query("MATCH (n:N) RETURN n.a, n.b, n.c, n.d")
        self.env.assertEquals(result.result_set[0][0], 1)
        self.env.assertEquals(result.result_set[0][1], 'str')
        self.env.assertEquals(result.result_set[0][2], [1, 'str', {'latitude':1, 'longitude':2}])
        self.env.assertEquals(result.result_set[0][3], {'latitude':1, 'longitude':2})

        try:
            self.graph.query("""MATCH (n:N {a: 1})
                                SET n += {e: 1}
                                WITH n
                                RETURN 'a' * 1""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # expecting the original attributes to be restored
        result = self.graph.query("MATCH (n:N) RETURN n.a, n.b, n.c, n.d, n.e")
        self.env.assertEquals(result.result_set[0][0], 1)
        self.env.assertEquals(result.result_set[0][1], 'str')
        self.env.assertEquals(result.result_set[0][2], [1, 'str', {'latitude':1, 'longitude':2}])
        self.env.assertEquals(result.result_set[0][3], {'latitude':1, 'longitude':2})
        self.env.assertEquals(result.result_set[0][4], None)

    def test06_undo_update_edge(self):
        self.graph.query("CREATE (:N)-[:R {v: 1}]->(:N)")
        try:
            self.graph.query("""MATCH ()-[r]->()
                              SET r.v = 2
                              WITH r
                              RETURN 'a' * 1""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # expecting the original attributes to be restored
        result = self.graph.query("MATCH ()-[r]->() RETURN r.v")
        self.env.assertEquals(result.result_set[0][0], 1)

    def test07_undo_delete_indexed_node(self):
        self.graph.query("CREATE INDEX FOR (n:N) ON (n.v)")
        self.graph.query("CREATE (:N {v: 0})")
        try:
            self.graph.query("""MATCH (n:N)
                                DELETE n
                                WITH n
                                RETURN 'a' * 1""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # deleted node should be revived, expecting a single node
        query = "MATCH (n:N {v: 0}) RETURN COUNT(n)"
        plan = self.graph.execution_plan(query)
        self.env.assertContains("Node By Index Scan", plan)
        result = self.graph.query(query)
        self.env.assertEquals(result.result_set[0][0], 1)

    def test08_undo_delete_indexed_edge(self):
        self.graph.query("CREATE INDEX FOR ()-[r:R]->() ON (r.v)")
        self.graph.query("CREATE (:N)-[:R {v: 0}]->(:N)")
        try:
            self.graph.query("""MATCH ()-[r:R]->()
                                DELETE r
                                WITH r
                                RETURN 'a' * 1""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # deleted edge should be revived, expecting a single edge
        query = "MATCH ()-[r:R {v: 0}]->() RETURN COUNT(r)"
        plan = self.graph.execution_plan(query)
        self.env.assertContains("Edge By Index Scan", plan)
        result = self.graph.query(query)
        self.env.assertEquals(result.result_set[0][0], 1)

    def test09_undo_update_indexed_node(self):
        self.graph.query("CREATE INDEX FOR (n:N) ON (n.v)")
        self.graph.query("CREATE (:N {v: 1})")
        try:
            self.graph.query("""MATCH (n:N {v: 1})
                                SET n.v = 2
                                WITH n
                                RETURN 'a' * 1""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # expecting the original attributes to be restored and indexed
        query = "MATCH (n:N {v: 1}) RETURN n.v"
        plan = self.graph.execution_plan(query)
        self.env.assertContains("Node By Index Scan", plan)
        result = self.graph.query(query)
        self.env.assertEquals(result.result_set[0][0], 1)
    
    def test10_undo_update_indexed_edge(self):
        self.graph.query("CREATE INDEX FOR ()-[r:R]->() ON (r.v)")
        self.graph.query("CREATE (:N)-[:R {v: 1}]->(:N)")
        try:
            self.graph.query("""MATCH ()-[r]->()
                                SET r.v = 2
                                WITH r
                                RETURN 'a' * 1""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # expecting the original attributes to be restored and indexed
        query = "MATCH ()-[r:R {v: 1}]->() RETURN r.v"
        plan = self.graph.execution_plan(query)
        self.env.assertContains("Edge By Index Scan", plan)
        result = self.graph.query(query)
        self.env.assertEquals(result.result_set[0][0], 1)

    def test11_undo_implicit_edge_delete(self):
        self.graph.query("CREATE (n:N), (m:N), (n)-[:R]->(m), (n)-[:R]->(m)")
        try:
            self.graph.query("""MATCH (n:N)
                                DETACH DELETE n
                                WITH n
                                RETURN 1 * 'a'""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # deleted node should be revived, expecting a single node
        result = self.graph.query("MATCH (n:N) RETURN COUNT(n)")
        self.env.assertEquals(result.result_set[0][0], 2)
        result = self.graph.query("MATCH ()-[r:R]->() RETURN COUNT(r)")
        self.env.assertEquals(result.result_set[0][0], 2)

    def test11_undo_timeout(self):
        # Change timeout value from default
        response = self.redis_con.execute_command("GRAPH.CONFIG SET TIMEOUT 1")
        self.env.assertEqual(response, "OK")

        try:
            self.graph.query("UNWIND range(1, 1000000) AS x CREATE (n:N)")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except Exception as e:
            pass

        # node (n:N) should be removed, expecting an empty graph
        result = self.graph.query("MATCH (n:N) RETURN COUNT(n)")
        self.env.assertEquals(result.result_set[0][0], 0)

