from common import *

GRAPH_ID = "index"
redis_graph = None
redis_con = None


class testIndexDeletionFlow():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_graph
        global redis_con
        redis_con = self.env.getConnection()
        redis_graph = Graph(redis_con, GRAPH_ID)

    def test01_drop_index(self):
        # drop not existed index
        try:
            result = redis_graph.query("DROP INDEX ON :person(age)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Unable to drop index on :person(age): no such index.", str(e))

        # create index over node person:age
        result = redis_graph.query("CREATE INDEX FOR (p:person) ON (p.age)")
        self.env.assertEquals(result.indices_created, 1)

        # drop index
        result = redis_graph.query("DROP INDEX ON :person(age)")
        self.env.assertEquals(result.indices_deleted, 1)

        # create an index over edge follow:created_at
        result = redis_graph.query("CREATE INDEX FOR ()-[r:follow]-() ON (r.created_at)")
        self.env.assertEquals(result.indices_created, 1)

        # delete index
        result = redis_graph.query("DROP INDEX ON :follow(created_at)")
        self.env.assertEquals(result.indices_deleted, 1)

        result = redis_graph.query("CALL db.indexes()")
        self.env.assertEquals(len(result.result_set), 0)

    def test04_drop_fulltext_node_index(self):
        # create an index over L:title
        create_fulltext_index(redis_graph, 'L', 'title', sync=True)

        # delete index
        drop_fulltext_index(redis_graph, 'L')

        # validate no indexes in graph
        result = list_indicies(redis_graph)
        self.env.assertEquals(len(result.result_set), 0)

    def test05_drop_index_during_population(self):
        # 1. populate a graph
        # 2. create an index and wait for it to be sync
        # 3. constantly update indexed entities
        # 4. drop index
        # 5. validate execution-plan + indexes report

        # populate a graph
        q = "UNWIND range(0, 1000) AS x CREATE (:N{v:x})"
        redis_graph.query(q)

        # create an index and wait for it to be sync
        create_node_exact_match_index(redis_graph, 'N', 'v', sync=True)

        # constantly update indexed entities
        # drop index
        # validate execution-plan + indexes report
        start_idx = 0
        end_idx = 100
        for i in range(start_idx, end_idx):
            # update indexed entities
            q = f"MATCH (n:N) WHERE n.v = {i} SET n.v = -n.v"
            redis_graph.query(q)

            if i < end_idx / 2:
                # validate execution-plan + indexes report
                plan = redis_graph.execution_plan(q)
                self.env.assertIn("Index", plan)

                indicies = list_indicies(redis_graph).result_set
                self.env.assertEquals(len(indicies), 1)

            elif i == end_idx / 2:
                # drop index
                drop_exact_match_index(redis_graph, 'N', 'v')

            else:
                # validate execution-plan + indexes report
                plan = redis_graph.execution_plan(q)
                self.env.assertNotIn("Index", plan)

                indicies = list_indicies(redis_graph).result_set
                self.env.assertEquals(len(indicies), 0)

    def test06_reset_order(self):
        """Tests that the reset order is correct, i.e., that the reading ops are
        reset before the writing ops (otherwise we write while a read-lock is
        held)."""

        # clear the db
        self.env.flush()
        redis_graph = Graph(self.env.getConnection(), GRAPH_ID)

        # create data
        redis_graph.query(
            """
            WITH 1 AS x
            CREATE (:X {uid: toString(x)})-[:R]->(y:Y {v: x})
            """
        )

        # create an index
        create_node_exact_match_index(redis_graph, 'X', 'uid', sync=True)
        create_node_exact_match_index(redis_graph, 'Y', 'v', sync=True)

        # utilize the index for a scan, followed by a deletion of the indexed
        # entity and setting of a property on the other entity
        res = redis_graph.query(
            """
            MATCH (x:X {uid: '1'})-[:R]->(y:Y)
            DELETE y
            SET x.uid = '10'
            RETURN x
            """
        )

        # validate results
        self.env.assertEquals(res.nodes_deleted, 1)
        self.env.assertEquals(res.relationships_deleted, 1)
        self.env.assertEquals(len(res.result_set), 1)
        self.env.assertEquals(res.result_set[0][0],
            Node(label='X', properties={'uid': '10'}))
