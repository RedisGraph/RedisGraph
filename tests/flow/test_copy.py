import random
from common import *

redis_con = None
port = None

class testGraphCopy():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        self.redis_con = self.env.getConnection()

    def random_graph(self, graph_name):
        # create origin graph
        queries = [
                """UNWIND range(0, 10) AS x
                   WITH x AS x
                   WHERE rand() > 0.4
                   CREATE (:A{v:x})-[:E{v:x}]->(:Z{v:x})""",                 # nodes & edges
                """CREATE INDEX FOR (a:A) ON (a.v)""",                       # node index
                """CREATE INDEX FOR ()-[e:E]->() ON (e.v)""",                # edge index
                """MATCH ()-[e]->() WHERE rand() > 0.7 DELETE e""",          # delete edges
                """MATCH (n) WHERE rand() > 0.7 DELETE n""",                 # delete nodes
                """CREATE (:A{v:rand()})-[:E{v:rand()}]->(:Z{v:rand()})""",  # nodes & edges
                """CALL db.idx.fulltext.createNodeIndex('Z', 'v')"""         # fulltext index
                ]

        g = Graph(self.redis_con, graph_name)

        for q in queries:
            g.query(q)

        return g

    # assert graph A and B are the same
    def assert_graphs_eq(self, A, B):
        A = Graph(self.redis_con, A)
        B = Graph(self.redis_con, B)

        #-----------------------------------------------------------------------
        # validations queries
        #-----------------------------------------------------------------------

        queries = [
                """MATCH (n) RETURN count(n)""",                # number of nodes
                """MATCH ()-[e]->() RETURN count(e)""",         # number of edges
                """CALL db.indexes()
                   YIELD type, label, properties,
                   language, stopwords, entitytype
                   RETURN *
                   ORDER BY type, entitytype""",                # indices
                """CALL db.labels()""",                         # labels
                """CALL db.relationshipTypes()""",              # relationship types
                """MATCH (n) RETURN n ORDER BY ID(n)""",        # validate nodes
                """MATCH ()-[e]->() RETURN e ORDER BY ID(e)"""  # validate edges
                ]

        B_result_sets = []
        for q in queries:
            A_res = A.query(q)
            B_res = B.query(q)
            self.env.assertEquals(A_res.result_set, B_res.result_set)
            B_result_sets.append(B_res.result_set)

        # make sure deletion of the original graph doesn't effect
        # the cloned graph
        # re-issue previous queries and validate against previous results
        A.delete()
        for i, q in enumerate(queries):
            res = B.query(q)
            self.env.assertEquals(res.result_set, B_result_sets[i])

    def populate_and_compare(self, graph_name, queries):
        g = Graph(self.redis_con, graph_name)

        for q in queries:
            g.query(q)

        # copy graph
        self.env.execute_command("COPY", graph_name, graph_name + "_copy")

        # compare graphs
        self.assert_graphs_eq(graph_name, graph_name + "_copy")

    def test00_copy_empty_graph(self):
        q = "RETURN 1"
        self.populate_and_compare("empty", [q])

    def test01_single_node_graph(self):
        # create a graph with a single node
        q = "CREATE (:N{num:1, str:'str', arr:[1,2,3], p:point({latitude: 60, longitude: 60})})"
        self.populate_and_compare("single_node", [q])

    def test02_single_edge_graph(self):
        # create a graph with a single node
        q = "CREATE (:A{v:0})-[:E{num:1, str:'str', arr:[1,2,3], p:point({latitude: 60, longitude: 60})}]->(:Z{v:9})"
        self.populate_and_compare("single_edge", [q])

    def test03_multi_edge_graph(self):
        # create a graph with a single node
        q = "CREATE (a:A{v:0})-[:E{num:1}]->(z:Z{v:9}), (a)-[:E{num:2}]->(z)"
        self.populate_and_compare("multi_edge", [q])

    def test04_empty_graph_with_indices(self):
        # create a graph 3 indices
        # two exact match
        # one full-text
        queries = [
                "CREATE INDEX FOR (p:Person) ON (p.age)",
                "CREATE INDEX FOR ()-[e:E]->() ON (e.weight)",
                "CALL db.idx.fulltext.createNodeIndex('Movie', 'title')"
                ]
        self.populate_and_compare("indices", queries)

    def test05_deleted_nodes_and_edges(self):
        # create a 20 nodes and 10 edge
        # delete just a few nodes and edges
        # create 2 nodes and an edge
        queries = [
                """UNWIND range(0,10) AS x
                   CREATE (:A{v:x})-[:E{v:x}]->(:Z{v:x})""", # create nodes & edges

                """UNWIND range(1,10,4) AS x                 
                   MATCH (n) WHERE ID(n) = x
                   DELETE n""",                              # delete nodes

                """MATCH ()-[e]->()
                 WHERE rand() > 0.7
                 DELETE e""",                                # delete edges

                """CREATE (:A)-[:E]->(:Z)"""                 # create nodes & edges
                ]
        self.populate_and_compare("deleted_entities", queries)

    def test06_copy_of_a_copy(self):
        # create multiple copies of a random graph
        # compare copies to the original

        graph_name = "copies"
        g = self.random_graph(graph_name)

        # create clone of clone
        current = graph_name
        for i in range (100):
            # copy graph
            clone = graph_name + "_" + str(i)
            self.env.execute_command("COPY", current, clone)
            self.assert_graphs_eq(current, clone)
            current = clone

    def test07_save_restore_copy(self):
        # create a random graph
        # clone graph
        # save and restore

        graph_name = "save_restore"
        clone_graph_name = "save_restore_clone"
        g = self.random_graph(graph_name)

        # clone graph
        self.env.execute_command("COPY", graph_name, clone_graph_name)

        # dump and restore
        self.env.dumpAndReload()

        # validate graphs are the same
        self.assert_graphs_eq(graph_name, clone_graph_name)

    def assert_seperation(self, dynamic, static):
        # extract information from static graph
        queries = [
                """MATCH (n) RETURN count(n)""",        # number of nodes
                """MATCH ()-[e]->() RETURN count(e)""", # number of edges
                """CALL db.indexes()
                   YIELD type, label, properties,
                   language, stopwords, entitytype
                   RETURN *
                   ORDER BY type, entitytype""",        # indices
                """CALL db.labels()""",                 # labels
                """CALL db.relationshipTypes()""",      # relationship types
                """MATCH (n) RETURN n""",               # validate nodes
                """MATCH ()-[e]->() RETURN e"""         # validate edges
                ]

        # run queries and save result-sets for later validation
        result_sets = []
        for q in queries:
            res = static.query(q)
            result_sets.append(res.result_set)

        # update dynamic graph
        update_queries = [
                "MATCH (n) SET n.v = rand()",               # update node
                "MATCH ()-[e]->() SET e.v = rand()",        # update edge
                "MATCH (n) WITH n LIMIT 1 DELETE n",        # delete node
                "MATCH ()-[e]->() WITH e LIMIT 1 DELETE e", # delete edge
                "DROP INDEX ON :A(v)",                      # drop index
                "CALL db.idx.fulltext.drop('Z')",           # drop index
                "CREATE (:K{val:1})",                       # create node
                "CREATE ()-[:Edge{weight:1}]->()"           # create edge
                ]

        # update dynamic graph
        for q in update_queries:
            dynamic.query(q)

        # validate static graph wasn't effected
        for i, q in enumerate(queries):
            res = static.query(q)
            self.env.assertEquals(result_sets[i], res.result_set)

    def test08_modify_origin(self):
        # create a graph and form a copy
        # modify the original graph
        # validate changes aren't reflected in the copy

        graph_name = "modifications"
        clone_graph_name = graph_name + "_clone"
        g = self.random_graph(graph_name)

        # clone graph
        self.env.execute_command("COPY", graph_name, clone_graph_name)
        g_clone = Graph(self.redis_con, clone_graph_name)

        self.assert_seperation(g, g_clone)

    def test09_modify_copy(self):
        # create a graph and form a copy
        # modify the copied graph
        # validate changes aren't reflected in the original graph

        graph_name = "modifications"
        clone_graph_name = graph_name + "_clone"
        g = self.random_graph(graph_name)

        # clone graph
        self.env.execute_command("COPY", graph_name, clone_graph_name)
        g_clone = Graph(self.redis_con, clone_graph_name)

        self.assert_seperation(g_clone, g)

