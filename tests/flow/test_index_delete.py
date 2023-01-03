from common import *
from index_utils import *

GRAPH_ID = "index_delete"
g = None
redis_con = None

class testIndexDeletionFlow():
    def __init__(self):
        global g
        global redis_con
        # skip test if we're running under Valgrind
        # drop index is an async operation which can cause Valgraind
        # to wrongfully report as a leak
        if VALGRIND:
            Env.skip()

        self.env = Env(decodeResponses=True)

        redis_con = self.env.getConnection()
        g = Graph(redis_con, GRAPH_ID)

    def test01_drop_missing_index(self):
        # drop none existing exact match index
        try:
            result = drop_exact_match_index(g, 'person', 'age')
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Unable to drop index on :person(age): no such index.", str(e))

        # drop none existing full-text index
        try:
            result = drop_fulltext_index(g, 'L')
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Unable to drop index on :L: no such index.", str(e))

    def test02_drop_exact_match_node_index(self):
        # create index over node person:age
        result = create_node_exact_match_index(g, 'person', 'age', sync=True)
        self.env.assertEquals(result.indices_created, 1)

        # drop index
        result = drop_exact_match_index(g, 'person', 'age')
        self.env.assertEquals(result.indices_deleted, 1)

        # validate no indexes in graph
        result = list_indicies(g)
        self.env.assertEquals(len(result.result_set), 0)

    def test03_drop_exact_match_edge_index(self):
        # create an index over edge follow:created_at
        result = create_edge_exact_match_index(g, 'follow', 'created_at', sync=True)
        self.env.assertEquals(result.indices_created, 1)

        # delete index
        result = drop_exact_match_index(g, 'follow', 'created_at')
        self.env.assertEquals(result.indices_deleted, 1)

        # validate no indexes in graph
        result = list_indicies(g)
        self.env.assertEquals(len(result.result_set), 0)

    def test04_drop_fulltext_node_index(self):
        # create an index over L:title
        create_fulltext_index(g, 'L', 'title', sync=True)

        # delete index
        drop_fulltext_index(g, 'L')

        # validate no indexes in graph
        result = list_indicies(g)
        self.env.assertEquals(len(result.result_set), 0)

    def test05_drop_index_during_population(self):
        # 1. populate a graph
        # 2. create an index and wait for it to be sync
        # 3. constantly update indexed entities
        # 4. drop index
        # 5. validate execution-plan + indexes report

        # populate a graph
        q = "UNWIND range(0, 1000) AS x CREATE (:N{v:x})"
        g.query(q)

        # create an index and wait for it to be sync
        create_node_exact_match_index(g, 'N', 'v', sync=True)

        # constantly update indexed entities
        # drop index
        # validate execution-plan + indexes report
        start_idx = 0
        end_idx = 100
        for i in range(start_idx, end_idx):
            # update indexed entities
            q = f"MATCH (n:N) WHERE n.v = {i} SET n.v = -n.v"
            g.query(q)

            if i < end_idx / 2:
                # validate execution-plan + indexes report
                plan = g.execution_plan(q)
                self.env.assertIn("Index", plan)

                indicies = list_indicies(g).result_set
                self.env.assertEquals(len(indicies), 1)

            elif i == end_idx / 2:
                # drop index
                drop_exact_match_index(g, 'N', 'v')

            else:
                # validate execution-plan + indexes report
                plan = g.execution_plan(q)
                self.env.assertNotIn("Index", plan)

                indicies = list_indicies(g).result_set
                self.env.assertEquals(len(indicies), 0)

