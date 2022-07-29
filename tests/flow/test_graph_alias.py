from common import *

ALIAS = "alias"
GRAPH_ID_0 = "g0"
GRAPH_ID_1 = "g1"

class testGraphAlias():
    def __init__(self):
        self.env = Env()

    def test01_error_checking(self):
        con = self.env.getConnection()

        # creating an alias against a none existing key should fail
        try:
            con.execute_command("GRAPH.ALIAS", "alias", "none-existing-key")
            # we shouldn't be here, expected to fail
            self.env.assertTrue(False)
        except:
            pass

        # creating an alias against a none graph key should fail
        try:
            con.set("x", 1)
            con.execute_command("GRAPH.ALIAS", "alias", "x")
            # we shouldn't be here, expected to fail
            self.env.assertTrue(False)
        except:
            pass

        # creating an alias using an already existing key should fail
        try:
            # create a graph
            con.execute_command("GRAPH.QUERY", GRAPH_ID_0, "RETURN 1")
            # the key 'x' is already
            con.execute_command("GRAPH.ALIAS", "x", GRAPH_ID_0)
            # we shouldn't be here, expected to fail
            self.env.assertTrue(False)
        except:
            pass

        # creating an alias expects exactly 2 arguments
        try:
            con.execute_command("GRAPH.ALIAS", ALIAS, GRAPH_ID_0, "extra arg")
            # we shouldn't be here, expected to fail
            self.env.assertTrue(False)
        except:
            pass

    def test02_query_via_alias(self):
        # validate that we're able to query a graph via an alias
        con = self.env.getConnection()

        # creae a graph
        con.execute_command("GRAPH.QUERY", GRAPH_ID_0, "CREATE (:L {v:1})")

        # create alias
        con.execute_command("GRAPH.ALIAS", ALIAS, GRAPH_ID_0)

        # query graph
        alias_graph  = Graph(con, ALIAS)
        direct_graph = Graph(con, GRAPH_ID_0)

        # validate we're getting the same result-set
        alias_resultset  = alias_graph.query("MATCH (n) RETURN n").result_set
        direct_resultset = direct_graph.query("MATCH (n) RETURN n").result_set

        self.env.assertEquals(alias_resultset, direct_resultset)

    def test03_swap_graphs(self):
        # test we're able to switch from one graph to another using an alias
        con = self.env.getConnection()
        con.flushall()

        # create Graph_0
        a = Node("A", properties={'a':0})
        g0 = Graph(con, GRAPH_ID_0)
        g0.add_node(a)
        g0.commit()

        # create Graph_1
        z = Node("A", properties={'a':9})
        g1 = Graph(con, GRAPH_ID_1)
        g1.add_node(z)
        g1.commit()

        q = "MATCH (n) RETURN n"
        g0_resultset = g0.query(q).result_set
        g1_resultset = g1.query(q).result_set

        # create alias
        # alias -> G0
        con.execute_command("GRAPH.ALIAS", ALIAS, GRAPH_ID_0)
        alias_graph = Graph(con, ALIAS)

        # query graph via alias
        alias_resultset = alias_graph.query(q).result_set
        #self.env.assertEquals(g0_resultset, alias_resultset)

        # swap graphs
        # TODO: use MULTI-EXEC to perform the swap
        # alias -> G1
        con.delete(ALIAS)
        con.execute_command("GRAPH.ALIAS", ALIAS, GRAPH_ID_1)

        # query graph via alias
        alias_resultset = alias_graph.query(q).result_set
        self.env.assertEquals(g1_resultset, alias_resultset)

    def test04_recreate_graph_via_alias(self):
        # an alias pointing to a delete node will re-create it
        # when queried
        con = self.env.getConnection()
        con.flushall()

        # create graph
        con.execute_command("GRAPH.QUERY", GRAPH_ID_0, "RETURN 1")

        # create alias
        # alias -> G0
        con.execute_command("GRAPH.ALIAS", ALIAS, GRAPH_ID_0)

        # delete G0
        con.delete(GRAPH_ID_0)
        self.env.assertFalse(con.exists(GRAPH_ID_0))

        # query graph via alias
        con.execute_command("GRAPH.QUERY", ALIAS, "RETURN 1")

        # validate graph was created
        self.env.assertTrue(con.exists(GRAPH_ID_0))

#TODO:
# test persistence
#TODO:
# handle graph schema change due to swap, re-introduce graph hash sig
