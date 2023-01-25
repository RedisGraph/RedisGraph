from common import *
from collections import Counter

graph = None
GRAPH_ID = "G"

class testForeachFlow():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global graph
        redis_con = self.env.getConnection()
        graph = Graph(redis_con, GRAPH_ID)

    def get_res_and_assertEquals(self, query, expected_result):
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)
        return actual_result

    # check that FOREACH with a literal list works properly
    def test01_literal_list(self):
        # graph is empty
        res = graph.query("FOREACH(i in range(0, 4) | CREATE (n:N {v: i}))")

        # 5 nodes should have been created, and 5 properties set
        self.env.assertEquals(res.nodes_created, 5)
        self.env.assertEquals(res.properties_set, 5)

        # validate that the correct nodes have been created
        self.get_res_and_assertEquals("MATCH (n) RETURN n.v ORDER BY n.v",
                                      [[0], [1], [2], [3], [4]])

        # validate MERGE (without creation) and SET
        res = graph.query(
            'FOREACH(i in range(0, 4) | MERGE (n:N {v: i}) SET n.v=i^2)'
            )

        # make sure no nodes were created, and 3 properties were set
        self.env.assertEquals(res.nodes_created, 0)
        self.env.assertEquals(res.properties_set, 3)

        # validate that the update is correct
        self.get_res_and_assertEquals("MATCH (n) RETURN n.v ORDER BY n.v",
                                      [[0], [1], [4], [9], [16]])
        
        # validate MERGE creation
        res = graph.query(
            'FOREACH(i in range(0, 4) | MERGE (m:M {v: i}))'
        )

        # 5 nodes should have been created, and 5 properties set
        self.env.assertEquals(res.nodes_created, 5)
        self.env.assertEquals(res.properties_set, 5)

        # delete the entities (the four nodes)
        res = graph.query("""
        FOREACH(i in range(0, 4) |
            MERGE (m:M {v: i}) DELETE m
        )""")

        # validate the deletion
        self.env.assertEquals(res.nodes_deleted, 5)
        
        # remove the properties and labels of the nodes using REMOVE
        res = graph.query("""
        MATCH (n:N)
        WITH collect(n) as ns
        FOREACH(n in ns |
            REMOVE n.v, n:N
        )""")

        # validate removal
        self.env.assertEquals(res.properties_removed, 5)
        self.env.assertEquals(res.labels_removed, 5)
        res = graph.query("MATCH (n) return labels(n), n.v")
        self.env.assertEquals(res.result_set, 
                    [[[], None], [[], None],[[], None], [[], None], [[], None]])
        
        # embedded foreach
        res = graph.query("""
        FOREACH(i in [0, 1, 2, 3, 4] |
            FOREACH(j in [1, 2] |
                CREATE (n:N)
                SET n.i=i, n.j=j
            )
        )"""
        )

        # validate the actions of the query
        self.env.assertEquals(res.nodes_created, 10)
        self.env.assertEquals(res.properties_set, 20)
        

    # tests that FOREACH with an aliased list works properly
    def test02_aliased_list(self):
        # clear db
        self.env.flush()

        res = graph.query(
        "CYPHER li = [0, 1, 2, 3, 4] FOREACH(i in $li | CREATE (n:N {v: i}))"
        )

        # 5 nodes should have been created, and 5 properties set
        self.env.assertEquals(res.nodes_created, 5)
        self.env.assertEquals(res.properties_set, 5)

        # validate that the correct nodes have been created
        self.get_res_and_assertEquals("MATCH (n) RETURN n.v ORDER BY n.v",
                                      [[0], [1], [2], [3], [4]])

        # validate MERGE (without creation) and SET
        res = graph.query("""
        CYPHER li = [0, 1, 2, 3, 4]
        FOREACH(i in $li |
            MERGE (n:N {v: i}) SET n.v=i^2
        )"""
        )

        # make sure no nodes were created, and 3 properties were set
        self.env.assertEquals(res.nodes_created, 0)
        self.env.assertEquals(res.properties_set, 3)

        # validate that the update is correct
        self.get_res_and_assertEquals("MATCH (n) RETURN n.v ORDER BY n.v",
                                      [[0], [1], [4], [9], [16]])

        # validate MERGE creation
        res = graph.query("""
        CYPHER li = [0, 1, 2, 3, 4]
        FOREACH(i in $li |
            MERGE (m:M {v: i})
        )"""
        )

        # 5 nodes should have been created, and 5 properties set
        self.env.assertEquals(res.nodes_created, 5)
        self.env.assertEquals(res.properties_set, 5)

        # delete the entities (the four nodes)
        res = graph.query("""
        CYPHER li = [0, 1, 2, 3, 4]
        FOREACH(i in $li |
            MERGE (m:M {v: i}) DELETE m
        )"""
        )

        # validate the deletion
        self.env.assertEquals(res.nodes_deleted, 5)
        
        # remove the properties of the nodes using remove
        res = graph.query("""
        MATCH (n:N)
        WITH collect(n) as ns
        FOREACH(n in ns |
            REMOVE n.v, n:N
        )"""
        )

        # validate removal
        self.env.assertEquals(res.properties_removed, 5)
        self.env.assertEquals(res.labels_removed, 5)
        res = graph.query("MATCH (n) return labels(n), n.v")
        self.env.assertEquals(res.result_set, 
                    [[[], None], [[], None],[[], None], [[], None], [[], None]])

        # embedded foreach
        res = graph.query("""
        CYPHER li = [0, 1, 2, 3, 4]
        FOREACH(i in $li |
            FOREACH(j in [1, 2] |
                CREATE (n:N)
                SET n.i=i, n.j=j
            )
        )"""
        )

        # validate the actions of the query
        self.env.assertEquals(res.nodes_created, 10)
        self.env.assertEquals(res.properties_set, 20)

    # tests a CASE WHEN THEN ELSE
    def test03_case(self):
        # clean db
        self.env.flush()

        # perform a conditional query using a CASE expression
        res = graph.query("""
        FOREACH(do_perform IN CASE WHEN true THEN [1] ELSE [] END |
            CREATE (:N)
        )"""
        )

        # make sure a node was created
        self.env.assertEquals(res.nodes_created, 1)

        # same case with a negative test-clause
        res = graph.query("""
        FOREACH(do_perform IN CASE WHEN false THEN [1] ELSE [] END |
            CREATE (:N)
        )"""
        )

        # make sure a node was not created
        self.env.assertEquals(res.nodes_created, 0)

    # test the tieing of different segments with FOREACH
    def test04_tie_with_foreach(self):
        # clean db
        self.env.flush()

        # populate graph
        graph.query(
            "CYPHER li = [0, 1, 2, 3, 4] FOREACH(i in $li | CREATE (n:N {v: i}))"
        )

        # send a query that demands the tieing of segments containing foreach
        query = """
        MATCH (n:N) WITH collect(n) as ns
        FOREACH(n in ns |
            CREATE (:N {v: n.v})
        )
        WITH ns
        FOREACH(n in ns |
            CREATE (:N {v: n.v^10})
        )
        """

        res = graph.query(query)

        # 5 + 5 = 10 nodes created
        self.env.assertEquals(res.nodes_created, 10)

    # validate that multiple records are passed to Foreach appropriately.
    # namely, the Foreach clause should run once for every record passed to it
    def test05_multiple_records(self):
        # clear db
        self.env.flush()

        # create 5 nodes
        graph.query("UNWIND range(0, 4) as val CREATE (n:N {v: val})")

        # execute a FOREACH clause for every node matched
        res = graph.query("""
        MATCH (n:N)
        FOREACH(i in [1] |
            CREATE (m:M {v: 2*n.v})
        )"""
        )
        self.env.assertEquals(res.nodes_created, 5)
        self.env.assertEquals(res.properties_set, 5)

        # validate that the creation is correct
        self.get_res_and_assertEquals("MATCH (m:M) RETURN m.v ORDER BY m.v",
                                      [[0], [2], [4], [6], [8]])

    # validate that Foreach accesses fields correctly
    def test06_field_access(self):
        # clean db
        self.env.flush()

        # create two nodes with list properties
        graph.query("CREATE (:N {li: [1, 2, 3, 4]}), (:M {li: [1, 2, 3, 4]})")

        # run a Foreach clause for every node, accessing its list
        res = graph.query("""MATCH (n) FOREACH(i in n.li |
                           CREATE (t:TEMP {v: i}))""")

        # validate the creation
        self.get_res_and_assertEquals("""MATCH (t:TEMP) RETURN t.v, count(t.v)
                                         ORDER BY t.v""",
                                      [[1, 2], [2, 2], [3, 2], [4, 2]])

    # mid-evaluation failure (memory free'd appropriately)
    def test07_midfail(self):
        # clean db
        self.env.flush()

        try:
            graph.query("FOREACH(i in [1, 2, 0, 3] | CREATE (n:N {v: 1/i}))")
        except redis.exceptions.ResponseError as e:
            self.env.assertIn("Division by zero", str(e))

        # assure that no nodes are left (undo-log was applied correctly)
        res = graph.query("MATCH (n) return count(n)")
        self.env.assertEquals(res.result_set[0][0], 0)

        # collect may not be used inside the list expression of FOREACH
        try:
            graph.query("""
            MATCH (n:N)
            FOREACH(n in collect(n) |
                CREATE (m:M {v: n.v})
            )"""
            )
        except redis.exceptions.ResponseError as e:
            self.env.assertIn("Invalid use of aggregating function 'collect'", str(e))

    # complicate things up
    def test08_complex(self):
        global graph

        # ----------------------------------------------------------------------
        # addressing node properties in the record sent to Foreach
        # ----------------------------------------------------------------------

        # create a single node
        graph.query("CREATE (:N {v: 1, name: 'RAZ', li: [1, 2, 3, 4]})")

        query = """
        MATCH (n:N {v: 1, name: toUpper('raz')})
        FOREACH(x in n.li |
            CREATE (m:M {v: n.v, name: toLower(n.name), li: [x]})
        )
        """

        res = graph.query(query)

        # validate the query statistics
        self.env.assertEquals(res.nodes_created, 4)
        self.env.assertEquals(res.properties_set, 12)
        self.env.assertEquals(res.nodes_deleted, 0)

        # validate that the end state is correct
        # there should be one node in the graph, with label N, 
        # properties: {v: 2, name: 'RAZmon', li: [1] and 4 nodes with label M
        # and properties: {v: 1, name: 'raz'} and li from 1 to 4
        res = graph.query("MATCH (n:N) return n")
        n = Node(label='N', properties={'v': 1, 'name': 'RAZ', 'li': [1, 2, 3, 4]})
        self.env.assertEquals(len(res.result_set), 1)
        self.env.assertEquals(res.result_set[0][0], n)
        res = graph.query("MATCH (m:M) return m")
        self.env.assertEquals(len(res.result_set), 4)
        for i in range(len(res.result_set)):
            m = Node(label= 'M', properties={'v': 1, 'name': 'raz', 'li': [i+1]})
            self.env.assertEquals(res.result_set[i][0], m)

        # clear nodes with label M
        res = graph.query("MATCH (m:M) DELETE m")
        self.env.assertEquals(res.nodes_deleted, 4)

        # ----------------------------------------------------------------------
        # triple embedding of a FOREACH clause, referencing node properties
        # ----------------------------------------------------------------------

        query = """
        MATCH (n:N)
        WITH collect(n) as ns
        FOREACH(n in ns |
            FOREACH(x in n.li |
                FOREACH(do_action IN CASE WHEN x > 2 THEN [1] ELSE [] END |
                    CREATE (t:TEMP {v: 2 * x + n.v, name: toLower(n.name), li: n.li + [x]})
                )
            )
        )
        """

        res = graph.query(query)

        # validate the query statistics
        self.env.assertEquals(res.nodes_created, 2)
        self.env.assertEquals(res.properties_set, 6)
        self.env.assertEquals(res.nodes_deleted, 0)

        # validate that the correct nodes were created
        n1 = Node(label = 'TEMP',
                  properties={'v': 7, 'name': 'raz', 'li': [1, 2, 3, 4, 3]})
        n2 = Node(label = 'TEMP',
                  properties={'v': 9, 'name': 'raz', 'li': [1, 2, 3, 4, 4]})
        res = self.get_res_and_assertEquals("""MATCH (t:TEMP) RETURN t ORDER BY
                                            t.v""",
                                            [[n1], [n2]])

        # ----------------------------------------------------------------------
        # triple embedded Foreach clause followed by reading and writing clauses
        # ----------------------------------------------------------------------

        # clear db
        self.env.flush()
        # Make a new graph object with no cache (problematic label info)
        graph = Graph(self.env.getConnection(), GRAPH_ID)

        # create a single node
        graph.query("CREATE (:N {v: 1, name: 'RAZ', li: [1, 2, 3, 4]})")

        query = """
        MATCH (n:N)
        WITH collect(n) as ns, toLower(n.name) as name
        FOREACH(n in ns |
            FOREACH(x in n.li |
                FOREACH(do_action IN CASE WHEN x > 2 THEN [1] ELSE [] END |
                    CREATE (t:TEMP {v: x, li: n.li + x, name: name})
                )
            )
        )
        WITH ns, name
        MATCH (t: TEMP)
        SET t.li = t.li + toUpper(name)
        WITH ns, t
        FOREACH(n in ns | CREATE (:TEMP))
        """

        res = graph.query(query)

        # validate the correctness of the new state
        self.env.assertEquals(res.nodes_created, 4)
        self.env.assertEquals(res.properties_set, 8)
        self.env.assertEquals(res.nodes_deleted, 0)

        # validate that the altered state is correct
        res = graph.query("MATCH (t:TEMP) return t")
        t1 = Node(label='TEMP', properties={'v': 3,
                                            'li': [1, 2, 3, 4, 3, 'RAZ'],
                                            'name': 'raz'})
        t2 = Node(label='TEMP', properties={'v': 4,
                                            'li': [1, 2, 3, 4, 4, 'RAZ'],
                                            'name': 'raz'})
        t3 = Node(label='TEMP')
        t4 = Node(label='TEMP')
        nodes = [t1, t2, t3, t4]
        for i in range(4):
            self.env.assertEquals(res.result_set[i][0], nodes[i])

        # delete newly created nodes
        graph.query("MATCH (t:TEMP) DELETE t")

    # a WITH clause must appear between FOREACH and a reading clause
    # (MATCH, UNWIND, CALL)
    def test09_clause_order(self):
        try:
            graph.query("FOREACH(i in [1] | CREATE (:M)) MATCH (m:M) RETURN m")
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertIn("A WITH clause is required to introduce MATCH after an updating clause.", str(e))

        try:
            graph.query("""
            FOREACH(i in [1] |
                CREATE (:M)
            )
            UNWIND [1, 2, 3] as x
            RETURN x"""
            )
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertIn("A WITH clause is required to introduce UNWIND after an updating clause.", str(e))

        try:
            graph.query("FOREACH(i in [1] | CREATE (:M)) CALL db.labels() YIELD\
                label RETURN label")
            self.env.assertTrue(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertIn("A WITH clause is required to introduce CALL after an updating clause.", str(e))

    # validate that edge (relationship) manipulation (creation, deletion update)
    # operations work correctly when embedded in a FOREACH clause
    def test10_edge_manipulation(self):
        # clear db and instantiate a new graph object (with an empty cache)
        global graph
        self.env.flush()
        graph = Graph(self.env.getConnection(), GRAPH_ID)

        # ----------------------------------------------------------------------
        # create an edge between 2 nodes
        # ----------------------------------------------------------------------

        # create two nodes and an edge between them
        res = graph.query("""
        CREATE (n1:N {v: 1}), (n2:N {v: 2})
        FOREACH(i in [1] |
            CREATE (n1)-[:R]->(n2)
        )"""
        )
        self.env.assertEquals(res.nodes_created, 2)
        self.env.assertEquals(res.relationships_created, 1)

        # ----------------------------------------------------------------------
        # delete an edge
        # ----------------------------------------------------------------------
        
        # delete the edge using FOREACH
        res = graph.query("MATCH (n1)-[r]-(n2) FOREACH(i in [1] | DELETE r)")
        self.env.assertEquals(res.nodes_deleted, 0)
        self.env.assertEquals(res.relationships_deleted, 1)

        # ----------------------------------------------------------------------
        # delete a node that has an edge. Validate that the edge is deleted as
        # well when the node is deleted
        # ----------------------------------------------------------------------

        # create an edge between the two existing nodes
        res = graph.query("""
        MERGE (n1:N {v: 1})
        MERGE (n2:N {v: 2})
        FOREACH(i in [1] |
            CREATE (n1)-[:R]->(n2)
        )"""
        )
        self.env.assertEquals(res.nodes_created, 0)
        self.env.assertEquals(res.relationships_created, 1)

        # delete the nodes with FOREACH
        res = graph.query("MATCH (n) FOREACH(i in [1] | DELETE n)")
        self.env.assertEquals(res.nodes_deleted, 2)
        self.env.assertEquals(res.relationships_deleted, 1)
