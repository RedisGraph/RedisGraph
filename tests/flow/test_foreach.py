from common import *
from collections import Counter

graph = None
redis_con = None
GRAPH_ID = "G"

class testForeachFlow():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global graph
        global redis_con
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
        for i in range(5):
            q_i = f'MATCH (n:N {{v: {i}}}) return n.v'
            res_i = [[i]]
            self.get_res_and_assertEquals(q_i, res_i)

        # validate MERGE (without creation) and SET
        res = graph.query(
            'FOREACH(i in range(0, 4) | MERGE (n:N {v: i}) SET n.v=i^2)'
            )

        # make sure no nodes were created, and 3 properties were set
        self.env.assertEquals(res.nodes_created, 0)
        self.env.assertEquals(res.properties_set, 3)

        # validate that the update is correct
        for i in range(5):
            q_i = f'MATCH (n:N {{v: {i**2}}}) return n.v'
            res_i = [[i**2]]
            self.get_res_and_assertEquals(q_i, res_i)
        
        # validate MERGE creation
        res = graph.query(
            'FOREACH(i in range(0, 4) | MERGE (m:M {v: i}))'
        )

        # 5 nodes should have been created, and 5 properties set
        self.env.assertEquals(res.nodes_created, 5)
        self.env.assertEquals(res.properties_set, 5)

        # delete the entities (the four nodes)
        res = graph.query("FOREACH(i in range(0, 4) | MERGE (m:M {v: i}) DELETE m)")

        # validate the deletion
        self.env.assertEquals(res.nodes_deleted, 5)
        
        # remove the properties of the nodes using remove
        res = graph.query("MATCH (n:N) WITH collect(n) as ns FOREACH(n in ns | REMOVE n.v, n:N)")

        # validate removal
        self.env.assertEquals(res.properties_removed, 5)
        self.env.assertEquals(res.labels_removed, 5)
        res = graph.query("MATCH (n) return labels(n), n.v")
        self.env.assertEquals(res.result_set, 
                    [[[], None], [[], None],[[], None], [[], None], [[], None]])
        
        # embedded foreach
        res = graph.query(
            """FOREACH(i in [0, 1, 2, 3, 4] | FOREACH(j in [1, 2] | \
                                                CREATE (n:N) \
                                                SET n.i=i, n.j=j)) \
            """
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
        for i in range(5):
            q_i = f'MATCH (n:N {{v: {i}}}) return n.v'
            res_i = [[i]]
            self.get_res_and_assertEquals(q_i, res_i)
        
        # validate MERGE (without creation) and SET
        res = graph.query(
            """CYPHER li = [0, 1, 2, 3, 4]
            FOREACH(i in $li | MERGE (n:N {v: i}) SET n.v=i^2)"""
            )

        # make sure no nodes were created, and 3 properties were set
        self.env.assertEquals(res.nodes_created, 0)
        self.env.assertEquals(res.properties_set, 3)

        # validate that the update is correct
        for i in range(5):
            q_i = f'MATCH (n:N {{v: {i**2}}}) return n.v'
            res_i = [[i**2]]
            self.get_res_and_assertEquals(q_i, res_i)
        
        # validate MERGE creation
        res = graph.query(
            'CYPHER li = [0, 1, 2, 3, 4] FOREACH(i in $li | MERGE (m:M {v: i}))'
        )

        # 5 nodes should have been created, and 5 properties set
        self.env.assertEquals(res.nodes_created, 5)
        self.env.assertEquals(res.properties_set, 5)

        # delete the entities (the four nodes)
        res = graph.query("CYPHER li = [0, 1, 2, 3, 4] FOREACH(i in $li | MERGE (m:M {v: i}) DELETE m)")

        # validate the deletion
        self.env.assertEquals(res.nodes_deleted, 5)
        
        # remove the properties of the nodes using remove
        res = graph.query("MATCH (n:N) WITH collect(n) as ns FOREACH(n in ns | REMOVE n.v, n:N)")

        # validate removal
        self.env.assertEquals(res.properties_removed, 5)
        self.env.assertEquals(res.labels_removed, 5)
        res = graph.query("MATCH (n) return labels(n), n.v")
        self.env.assertEquals(res.result_set, 
                    [[[], None], [[], None],[[], None], [[], None], [[], None]])

        # embedded foreach
        res = graph.query(
            """CYPHER li = [0, 1, 2, 3, 4] \
            FOREACH(i in $li | FOREACH(j in [1, 2] | \
                                    CREATE (n:N) \
                                    SET n.i=i, n.j=j)) \
            """
        )

        # validate the actions of the query
        self.env.assertEquals(res.nodes_created, 10)
        self.env.assertEquals(res.properties_set, 20)

    # tests a CASE WHEN THEN ELSE
    def test03_case(self):
        # clean db
        self.env.flush()

        # perform a conditional query using a CASE expression
        res = graph.query(
            """FOREACH(do_perform IN CASE WHEN true THEN [1] ELSE [] END | \
                CREATE (:N))"""
            )

        # make sure a node was created
        self.env.assertEquals(res.nodes_created, 1)

        # same case with a negative test-clause
        res = graph.query(
            """FOREACH(do_perform IN CASE WHEN false THEN [1] ELSE [] END | \
                CREATE (:N))"""
            )

        # make sure a node was not created
        self.env.assertEquals(res.nodes_created, 0)

    # test the tieing of different segments with FOREACH
    def test04_tie_with_foreach(self):
        # clean db
        self.env.flush()

        graph.query(
            "CYPHER li = [0, 1, 2, 3, 4] FOREACH(i in $li | CREATE (n:N {v: i}))"
        )

        # send a query that demands the tieing of segments containing foreach
        query = """
                MATCH (n:N) WITH collect(n) as ns
                FOREACH(n in ns | CREATE (:N {v: n.v}))
                WITH ns
                FOREACH(n in ns | CREATE (:N {v: n.v^10}))
                """

        res = graph.query(query)

        # 5 + 5 = 10 nodes created
        self.env.assertEquals(res.nodes_created, 10)

        # 5 + 5 = 10 properties set
        self.env.assertEquals(res.properties_set, 10)

    # validate that multiple records are passed to Foreach appropriately.
    # namely, the Foreach clause should run once for every record passed to it
    def test05_multiple_records(self):
        # clear db
        self.env.flush

        # create 5 nodes
        graph.query("UNWIND range(0, 4) as val CREATE (n:N {v: val})")

        # execute a FOREACH clause for every node matched
        res = graph.query("MATCH (n:N) FOREACH(i in [1] | CREATE (:M {v: 2*n.v}))")
        self.env.assertEquals(res.nodes_created, 5)
        self.env.assertEquals(res.properties_set, 5)

        # validate that the creation is correct
        for i in range(5):
            q_i = f'MATCH (m:M {{v: {2 * i}}}) RETURN m.v'
            res_i = [[2 * i]]
            self.get_res_and_assertEquals(q_i, res_i)

    # validate that Foreach accesses fields correctly
    def test06_field_access(self):
        # clean db
        self.env.flush()

        # create two nodes with list properties
        graph.query("CREATE (:N {li: [1, 2, 3, 4]}), (:M {li: [1, 2, 3, 4]})")

        # run a Foreach clause for every node, accessing its list
        res = graph.query("MATCH (n) FOREACH(i in n.li | \
                           CREATE (t:TEMP {v: i}))")

        # validate the creation
        self.env.assertEquals(res.nodes_created, 8)
        self.env.assertEquals(res.properties_set, 8)
        res2 = graph.query("MATCH (t:TEMP) RETURN t.v")
        c1 = {i : 2 for i in range(1, 5)}
        c_actual = Counter([li[0] for li in res2.result_set])
        self.env.assertEquals(c1, c_actual)
    
    # mid-evaluation failure (memory free'd appropriately)
    def test07_midfail(self):
        try:
            graph.query("FOREACH(i in [1, 2, 0, 3] | CREATE (n:N {v: 1/i}))")
        except redis.exceptions.ResponseError as e:
            self.env.assertIn("Division by zero", str(e))
