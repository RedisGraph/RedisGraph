import time
import random_graph
from common import *

GRAPH_ID = "graph_copy"
graph = None
redis_con = None

def copy_graph(src, dest):
    attempts = 10
    for i in range(0, attempts):
        try:
            redis_con.execute_command("GRAPH.COPY", src, dest)
            break
        except redis.exceptions.ResponseError as e:
            # retry in case we didn't managed to fork
            if "failed to create fork process" in repr(e):
                time.sleep(0.5) # sleep for half a second
                continue
            else:
                raise(e)

class testGraphCopy():
    def __init__(self):
        global graph
        global redis_con

        self.env = Env(decodeResponses=True)
        redis_con = self.env.getConnection()
        redis_graph = Graph(redis_con, GRAPH_ID)

    def tearDown(self):
        pass
        redis_con.flushall()

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
    # trivial validations tests
    #--------------------------------------------------------------------------

    def test_copy_none_existing_graph(self):
        # try to copy a none existing graph
        src = 'none_existing'
        dest = 'copy_of_' + src
        
        try:
            # expecting graph copy operation to fail
            copy_graph(src, dest)
            self.env.assertFalse(True)
        except:
            pass

    def test_copy_none_graph_key(self):
        # try to copy a key which is not a graph
        src = 'x'
        dest = 'copy_of_' + src

        redis_con.set(src, 1)
        try:
            # expecting graph copy operation to fail
            copy_graph(src, dest)
            self.env.assertFalse(True)
        except:
            pass
    
    def test_copy_into_existing_key(self):
        # try to copy a graph into an existing key
        src = 'g'
        dest = 'copy_of+' + src

        # create destination key
        redis_con.set(dest, 1)

        # create src graph key
        g = Graph(redis_con, src)
        g.query("CREATE ()")

        try:
            # expecting graph copy operation to fail
            copy_graph(src, dest)
            self.env.assertFalse(True)
        except:
            pass

    def test_copy_empty_graph(self):
        # create an empty graph
        src = 'empty_graph'
        dest = 'copy_of_' + src

        s = Graph(redis_con, src)
        s.query("CREATE (n) DELETE (n)")

        copy_graph(src, dest)

        t = Graph(redis_con, dest)
        self.env.assertTrue(self.graphs_eq(s, t))

    def test_copy_single_node_graph(self):
        src = 'single_node_graph'
        dest = 'copy_of_' + src
        
        s = Graph(redis_con, src)
        s.query("CREATE (:L{v:1})")

        copy_graph(src, dest)

        t = Graph(redis_con, dest)
        self.env.assertTrue(self.graphs_eq(s, t))

    def test_copy_single_edge_graph(self):
        src = 'single_edge_graph'
        dest = 'copy_of_' + src
        
        s = Graph(redis_con, src)
        s.query("CREATE ()-[:R {v:1}]->()")

        copy_graph(src, dest)

        t = Graph(redis_con, dest)
        self.env.assertTrue(self.graphs_eq(s, t))

    def test_copy_multiedge_graph(self):
        src = 'multi_edge_graph'
        dest = 'copy_of_' + src
        
        s = Graph(redis_con, src)
        s.query("CREATE (a)-[:R {v:1}]->(z), (a)-[:R {v:2}]->(z)")

        copy_graph(src, dest)

        t = Graph(redis_con, dest)
        self.env.assertTrue(self.graphs_eq(s, t))

    def test_copy_graph_with_deleteions(self):
        src = 'deleted_entity_graph'
        dest = 'copy_of_' + src
        
        s = Graph(redis_con, src)
        s.query("CREATE (:L {v:1}), (:L{v:2})")
        s.query("MATCH (a:L {v:1}) DELETE a")

        copy_graph(src, dest)

        t = Graph(redis_con, dest)
        self.env.assertTrue(self.graphs_eq(s, t))

        # make sure 's' and 't' reuse the deleted node
        s_res = s.query("CREATE (:L {v:1})")
        t_res = t.query("CREATE (:L {v:1})")
        self.env.assertTrue(self.graphs_eq(s, t))

    def test_copy_indexed_graph(self):
        src = 'indexed_graph'
        dest = 'copy_of_' + src
        
        s = Graph(redis_con, src)
        s.query("CREATE INDEX FOR (n:L) ON (n.v)")

        copy_graph(src, dest)

        t = Graph(redis_con, dest)
        self.env.assertTrue(self.graphs_eq(s, t))

    def test_copy_random_graph(self):
        src = 'random_graph'
        dest = 'copy_of_' + src
        
        s = Graph(redis_con, src)
        nodes, edges = random_graph.create_random_schema()
        random_graph.create_random_graph(s, nodes, edges)
        random_graph.run_random_graph_ops(s, nodes, edges, random_graph.ALL_OPS)

        copy_graph(src, dest)

        t = Graph(redis_con, dest)
        self.env.assertTrue(self.graphs_eq(s, t))

