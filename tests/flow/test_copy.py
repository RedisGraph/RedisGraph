import random
from common import *

redis_con = None
port = None

class testGraphCopy():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        self.redis_con = self.env.getConnection()

    # assert graph A and B are the same
    def assert_graphs_eq(self, A, B):
        A = Graph(self.redis_con, A)
        B = Graph(self.redis_con, B)

        #-----------------------------------------------------------------------
        # validations queries
        #-----------------------------------------------------------------------

        queries = [
                "MATCH (n) RETURN count(n)",        # number of nodes
                "MATCH ()-[e]->() RETURN count(e)", # number of edges
                "CALL db.indexes()",                # indices
                "CALL db.labels()",                 # labels
                "CALL db.relationshipTypes()",      # relationship types
                "MATCH (n) RETURN n",               # validate nodes
                "MATCH ()-[e]->() RETURN e"         # validate edges
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

    def test03_empty_graph_with_indices(self):
        # create a graph 3 indices
        # two exact match
        # one full-text
        queries = [
                "CREATE INDEX FOR (p:Person) ON (p.age)",
                "CREATE INDEX FOR ()-[e:E]->() ON (e.weight)",
                "CALL db.idx.fulltext.createNodeIndex('Movie', 'title')"
                ]
        self.populate_and_compare("indices", queries)

    def test04_deleted_nodes_and_edges(self):
        # create a 20 nodes and 10 edge
        # delete just a few nodes and edges
        # create 2 nodes and an edge
        queries = [
                "UNWIND range(0,10) AS x CREATE (:A{v:x})-[:E{v:x}]->(:Z{v:x})",
                "UNWIND range(1,10,4) AS x MATCH (n) WHERE ID(n) = x DELETE n",
                "MATCH ()-[e]->() WHERE rand() > 0.7 DELETE e",
                "CREATE (:A)-[:E]->(:Z)"
                ]
        self.populate_and_compare("deleted_entities", queries)

    def test05_copy_of_a_copy(self):
        # create a copy of a copy of a random graph
        # compare copy of copy to the original

        # nodes and edges
        # indcies
        # delete some entities
        queries = [
                "UNWIND range(0, 1000) AS x WITH x AS x WHERE rand() > 0.4 CREATE (:A{v:x})-[:E{v:x}]->(:Z{v:x})",
                "CREATE INDEX FOR (a:A) ON (a.v)",
                "CREATE INDEX FOR ()-[e:E]->() ON (e.v)",
                "MATCH ()-[e]->() WHERE rand() > 0.7 DELETE e",
                "MATCH (n) WHERE rand() > 0.7 DELETE n",
                #"CALL db.idx.fulltext.createNodeIndex('Z', 'v')"
                ]

        graph_name = "copies"
        g = Graph(self.redis_con, graph_name)

        for q in queries:
            g.query(q)

        # copy graph
        self.env.execute_command("COPY", graph_name, graph_name + "_copy")
        self.assert_graphs_eq(graph_name, graph_name + "_copy")
        #self.env.execute_command("COPY", graph_name + "_copy", graph_name + "_copy_copy")
        #self.assert_graphs_eq(graph_name, graph_name + "_copy_copy")

