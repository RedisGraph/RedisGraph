from RLTest import Env
from redisgraph import Graph

class testTraversalConstruction():
    def __init__(self):
        self.env = Env()
        self.redis_con = self.env.getConnection()
        self.graph = Graph("g", self.redis_con)

    def test01_undo_create_node(self):
        self.redis_con.flushall()
        try:
            self.graph.query("CREATE (n:N) WITH n CREATE (a:N {v: n})")
        except:
            result = self.graph.query("MATCH (n:N) RETURN COUNT(n)")
            self.env.assertEquals(result.result_set[0][0], 0)

    def test02_undo_create_edge(self):
        self.redis_con.flushall()
        self.graph.query("CREATE (:N {v: 1}), (:N {v: 2})")
        try:
            self.graph.query("MATCH (s:N {v: 1}), (t:N {v: 2}) CREATE (s)-[r:R]->(t) WITH r CREATE (a:N {v: r})")
        except:
            result = self.graph.query("MATCH ()-[r:R]->() RETURN COUNT(r)")
            self.env.assertEquals(result.result_set[0][0], 0)

    def test03_undo_delete_node(self):
        self.redis_con.flushall()
        self.graph.query("CREATE (:N)")
        try:
            self.graph.query("MATCH (n:N) DELETE n WITH n CREATE (a:N {v: n})")
        except:
            result = self.graph.query("MATCH (n:N) RETURN COUNT(n)")
            self.env.assertEquals(result.result_set[0][0], 1)

    def test04_undo_delete_edge(self):
        self.redis_con.flushall()
        self.graph.query("CREATE (:N)-[:R]->(:N)")
        try:
            self.graph.query("MATCH ()-[r:R]->() DELETE r WITH r CREATE (a:N {v: r})")
        except:
            result = self.graph.query("MATCH ()-[r:R]->() RETURN COUNT(r)")
            self.env.assertEquals(result.result_set[0][0], 1)

    def test05_undo_update_node(self):
        self.redis_con.flushall()
        self.graph.query("CREATE (:N {v: 1})")
        try:
            self.graph.query("MATCH (n:N {v: 1}) SET n.v = 2 WITH n CREATE (a:N {v: n})")
        except:
            result = self.graph.query("MATCH (n:N) RETURN n.v")
            self.env.assertEquals(result.result_set[0][0], 1)
    
    def test06_undo_update_edge(self):
        self.redis_con.flushall()
        self.graph.query("CREATE (:N)-[:R {v: 1}]->(:N)")
        try:
            self.graph.query("MATCH ()-[r]->() SET r.v = 2 WITH r CREATE (a:N {v: r})")
        except:
            result = self.graph.query("MATCH ()-[r]->() RETURN r.v")
            self.env.assertEquals(result.result_set[0][0], 1)
