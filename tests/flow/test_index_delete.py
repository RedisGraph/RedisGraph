from common import *

GRAPH_ID = "index_delete"
redis_con = None
redis_graph = None

class testIndexDeletionFlow():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_graph
        global redis_con
        redis_con = self.env.getConnection()
        redis_graph = Graph(redis_con, GRAPH_ID)

    def test01_drop_missing_index(self):
        # drop none existing exact match index
        try:
            result = redis_graph.query("DROP INDEX ON :person(age)")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Unable to drop index on :person(age): no such index.", str(e))

        # drop none existing full-text index
        try:
            result = redis_graph.query("CALL db.idx.fulltext.drop('L')")
            self.env.assertTrue(False)
        except ResponseError as e:
            self.env.assertContains("Unable to drop index on :L: no such index.", str(e))

    def test02_drop_exact_match_node_index(self):
        # create index over node person:age
        result = redis_graph.query("CREATE INDEX FOR (p:person) ON (p.age)")
        self.env.assertEquals(result.indices_created, 1)

        # drop index
        result = redis_graph.query("DROP INDEX ON :person(age)")
        self.env.assertEquals(result.indices_deleted, 1)

        # validate no indexes in graph
        result = redis_graph.query("CALL db.indexes()")
        self.env.assertEquals(len(result.result_set), 0)

    def test03_drop_exact_match_edge_index(self):
        # create an index over edge follow:created_at
        result = redis_graph.query("CREATE INDEX FOR ()-[r:follow]-() ON (r.created_at)")
        self.env.assertEquals(result.indices_created, 1)

        # delete index
        result = redis_graph.query("DROP INDEX ON :follow(created_at)")
        self.env.assertEquals(result.indices_deleted, 1)

        # validate no indexes in graph
        result = redis_graph.query("CALL db.indexes()")
        self.env.assertEquals(len(result.result_set), 0)

    def test04_drop_fulltext_node_index(self):
        # create an index over L:title
        redis_graph.query("CALL db.idx.fulltext.createNodeIndex('L', 'title')")

        # delete index
        result = redis_graph.query("CALL db.idx.fulltext.drop('L')")

        # validate no indexes in graph
        result = redis_graph.query("CALL db.indexes()")
        self.env.assertEquals(len(result.result_set), 0)

    def test05_drop_index_during_population(self):
        # 1. populate a graph
        # 2. create an index and wait for it to be sync
        # 3. constantly update indexed entities
        # 4. drop index
        # 5. validate execution-plan + indexes report
