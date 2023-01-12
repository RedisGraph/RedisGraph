from common import *
from collections import Counter

graph = None
redis_con = None
GRAPH_ID = "G"

class testForeachFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global graph
        global redis_con
        redis_con = self.env.getConnection()
        graph = Graph(redis_con, GRAPH_ID)

    def expect_error(self, query, expected_err_msg):
        try:
            graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError as e:
            self.env.assertIn(expected_err_msg, str(e))

    def expect_type_error(self, query):
        self.expect_error(query, "Type mismatch")

    def get_res_and_assertEquals(self, query, expected_result):
        actual_result = graph.query(query)
        self.env.assertEquals(actual_result.result_set, expected_result)
        return actual_result

    # check that FOREACH with a literal list works properly
    def test01_literal_list(self):
        # graph is empty
        res = graph.query(
            "FOREACH(i in range(0, 4) | CREATE (n:N {v: i}))"
            )
        # 5 nodes should have been created, and 5 properties set
        self.env.assertEquals(res.nodes_created, 5)
        self.env.assertEquals(res.properties_set, 5)

        # validate that the correct nodes have been created
        for i in range(5):
            q_i = f'MATCH (n:N {{v: {i}}}) return n.v'
            res_i = [[i]]
            self.get_res_and_assertEquals(q_i, res_i)

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


    # tests that the modifications made in the FOREACH clause are correct
    def test03_modifications(self):
        # using the nodes created above
        # update the nodes' properties
        res = graph.query(
            'FOREACH(i in range(0, 4) | MERGE (n:N {v: i}) SET n.v=i^2)'
            )

        # make sure no nodes were created
        self.env.assertEquals(res.nodes_created, 0)

        # validate that the update is correct
        for i in range(5):
            q_i = f'MATCH (n:N {{v: {i**2}}}) return n.v'
            res_i = [[i**2]]
            self.get_res_and_assertEquals(q_i, res_i)

        # update the nodes labels
        res = graph.query(
            'FOREACH(i in range(0, 4) | MERGE (n:N {v: i^2}) SET n:M)'
            )

        # validate that the update is correct
        for i in range(5):
            q_i = f'MATCH (n:N {{v: {i**2}}}) return labels(n)'
            res_i = [[['N', 'M']]]
            self.get_res_and_assertEquals(q_i, res_i)

    # tests a CASE WHEN THEN ELSE
    def test04_case(self):
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
    def test05_tie_with_foreach(self):
        # clean db
        self.env.flush()

        graph.query(
            "CYPHER li = [0, 1, 2, 3, 4] FOREACH(i in $li | CREATE (n:N {v: i}))"
        )

        # send a query that demands the tieing of segments containing foreach
        query = """
                MATCH (n:N) WITH collect(n) as ns
                FOREACH(n in ns | CREATE (:N {v: n.v}))
                MATCH (new_n:N) WITH collect(new_n) as new_ns
                FOREACH(n in new_ns | CREATE (:N {v: n.v^10}))
                """

        res = graph.query(query)

        # 5 + 10 = 15 nodes created
        self.env.assertEquals(res.nodes_created, 15)

        # 5 + 10 = 15 properties set
        self.env.assertEquals(res.properties_set, 15)

        # clean db
        self.env.flush()

    # validate that multiple records are passed to Foreach appropriately.
    # namely, the Foreach clause should run once for every record passed to it
    def test06_multiple_records(self):
        # clear db
        self.env.flush

        # create 5 nodes
        graph.query("UNWIND range(0, 4) as val CREATE (n:N {v: val})")

        # execute a FOREACH clause for every node matched
        query = """MATCH (n:N) FOREACH(i in [1] | CREATE (:M {v: 2*n.v}))"""
        res = graph.query(query)
        self.env.assertEquals(res.nodes_created, 5)
        self.env.assertEquals(res.properties_set, 5)

        # validate that the creation is correct
        for i in range(5):
            q_i = f'MATCH (m:M {{v: {2 * i}}}) RETURN m.v'
            res_i = [[2 * i]]
            self.get_res_and_assertEquals(q_i, res_i)

    # validate that Foreach accesses fields correctly
    def test07_field_access(self):
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

    # embedded foreach clause inside a foreach clause
    def test08_embedded_foreach(self):
        # clean db
        self.env.flush()

        # the following query should create 4 nodes, with v properties 1 to 4.
        res = graph.query("FOREACH(li in [[1, 2], [3, 4]] | \
                           FOREACH(j in li | CREATE (n:N {v: j})))")

        # validate the creation
        self.env.assertEquals(res.nodes_created, 4)
        self.env.assertEquals(res.properties_set, 4)
        res2 = graph.query("MATCH (n:N) return n.v")
        self.env.assertEquals(Counter([li[0] for li in res2.result_set]),
                              {i: 1 for i in range(1, 5)})

    # test that an embedded REMOVE clause affects the graph correctly
    def test09_remove(self):
        # state of the graph:
        # the graph has four nodes with label N, with property v set to 1 to 4

        # remove the properties of the nodes using remove\
        res = graph.query("MATCH (n:N) WITH collect(n) as ns FOREACH(n in ns | REMOVE n.v, n:N)")

        # validate removal
        self.env.assertEquals(res.properties_removed, 4)
        self.env.assertEquals(res.labels_removed, 4)
        res2 = graph.query("MATCH (n) return labels(n), n.v")
        self.env.assertEquals(res2.result_set, [[[], None], [[], None], [[], None], [[], None]])


    # test that DELETE acts appropriately when embedded in FOREACH
    def test10_embedded_delete(self):
        # state of the graph:
        # the graph has four nodes with no labels or properties

        # delete the entities (the four nodes)
        res = graph.query("MATCH (n) DELETE n")

        # validate the deletion
        self.env.assertEquals(res.nodes_deleted, 4)


    # check all clauses, with different permutation
    # keeping here a list of checked permutations of the clauses (to be deleted)
    # updating clauses aloud in FOREACH: SET, REMOVE, CREATE, MERGE, DELETE, and FOREACH
    # have already checked:
    #   CREATE
    #   MERGE -> SET
    #   FOREACH -> FOREACH
    #   DELETE

    # to add:
    #   MERGE --> DELETE
    #   DELETE --> MERGE
    #   SET --> MERGE
    #   SET --> REMOVE
    #   etc..
