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

        