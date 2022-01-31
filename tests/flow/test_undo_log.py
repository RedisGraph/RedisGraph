from RLTest import Env
from redisgraph import Graph

GRAPH_ID = "undo-log"
class testUndoLog():
    def __init__(self):
        self.env = Env()
        self.redis_con = self.env.getConnection()
        self.graph = Graph(GRAPH_ID, self.redis_con)

    def tearDown(self):
        self.redis_con.flushall()

    # TODOS:
    # 1. add a test for restoring implicit deleted edges

    def test01_undo_create_node(self):
        try:
            self.graph.query("CREATE (n:N) WITH n CREATE (a:N {v: n})")
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
                                CREATE (a:N {v: r})""")
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
                                CREATE (a:N {v: n})""")
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
                                CREATE (a:N {v: r})""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # deleted edge should be revived, expecting a single edge
        result = self.graph.query("MATCH ()-[r:R]->() RETURN COUNT(r)")
        self.env.assertEquals(result.result_set[0][0], 1)

    def test05_undo_update_node(self):
        self.graph.query("CREATE (:N {a: 1, b:'str', c:[1, 'str', point({x:1, y:2}), d:point({x:1, y:2})})")
        try:
            self.graph.query("""MATCH (n:N {a: 1})
                                SET n.a = 2, n.b = '', n.c = [], n.d = point({x:2, y:1})
                                WITH n
                                CREATE (a:N {v: n})""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # expecting the original attributes to be restored
        result = self.graph.query("MATCH (n:N) RETURN n.a, n.b, n.c, n.d")
        self.env.assertEquals(result.result_set[0][0], 1)
        self.env.assertEquals(result.result_set[0][1], 'str')
        self.env.assertEquals(result.result_set[0][2], [1, 'str', {'x':1, 'y':2}])
        self.env.assertEquals(result.result_set[0][3], {'x':1, 'y':2})
    
    def test06_undo_update_edge(self):
        self.graph.query("CREATE (:N)-[:R {v: 1}]->(:N)")
        try:
            self.graph.query("""MATCH ()-[r]->()
                              SET r.v = 2
                              WITH r
                              CREATE (a:N {v: r})""")
            # we're not supposed to be here, expecting query to fail
            self.env.assertTrue(False) 
        except:
            pass

        # expecting the original attributes to be restored
        result = self.graph.query("MATCH ()-[r]->() RETURN r.v")
        self.env.assertEquals(result.result_set[0][0], 1)

    # TODO: adapt changes from above
    def test07_undo_delete_indexed_node(self):
        self.graph.query("CREATE INDEX FOR (n:N) ON (n.v)")
        self.graph.query("CREATE (:N {v: 0})")
        try:
            self.graph.query("MATCH (n:N) DELETE n WITH n CREATE (a:N {v: n})")
        except:
            result = self.graph.query("MATCH (n:N {v: 0}) RETURN COUNT(n)")
            # TODO: verify execution-plan utilizes the index
            self.env.assertEquals(result.result_set[0][0], 1)

    def test08_undo_delete_indexed_edge(self):
        self.graph.query("CREATE INDEX FOR ()-[r:R]->() ON (r.v)")
        self.graph.query("CREATE (:N)-[:R {v: 0}]->(:N)")
        try:
            self.graph.query("MATCH ()-[r:R]->() DELETE r WITH r CREATE (a:N {v: r})")
        except:
            result = self.graph.query("MATCH ()-[r:R {v: 0}]->() RETURN COUNT(r)")
            self.env.assertEquals(result.result_set[0][0], 1)

    def test09_undo_update_indexed_node(self):
        self.graph.query("CREATE INDEX FOR (n:N) ON (n.v)")
        self.graph.query("CREATE (:N {v: 1})")
        try:
            self.graph.query("MATCH (n:N {v: 1}) SET n.v = 2 WITH n CREATE (a:N {v: n})")
        except:
            result = self.graph.query("MATCH (n:N {v: 1}) RETURN n.v")
            self.env.assertEquals(result.result_set[0][0], 1)
    
    def test10_undo_update_indexed_edge(self):
        self.graph.query("CREATE INDEX FOR ()-[r:R]->() ON (r.v)")
        self.graph.query("CREATE (:N)-[:R {v: 1}]->(:N)")
        try:
            self.graph.query("MATCH ()-[r]->() SET r.v = 2 WITH r CREATE (a:N {v: r})")
        except:
            result = self.graph.query("MATCH ()-[r:R {v: 1}]->() RETURN r.v")
            self.env.assertEquals(result.result_set[0][0], 1)

