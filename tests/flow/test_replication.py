from common import *
from index_utils import *
import time

GRAPH_ID = "replication"


# test to see if replication works as expected
# RedisGraph should replicate all write queries which had an effect on the
# underline graph, e.g. CREATE, DELETE, UPDATE operations as well as
# index creation and removal, read queries shouldn't be replicated.

class testReplication(FlowTestsBase):
    def __init__(self):
        # skip test if we're running under Valgrind
        if VALGRIND or SANITIZER != "":
            Env.skip(None) # valgrind is not working correctly with replication

        self.env = Env(decodeResponses=True, env='oss', useSlaves=True)

    def test_CRUD_replication(self):
        # create a simple graph
        env = self.env
        source_con = env.getConnection()
        replica_con = env.getSlaveConnection()

        # enable write commands on slave, required as all RedisGraph
        # commands are registered as write commands
        replica_con.config_set("slave-read-only", "no")

        # perform CRUD operations
        # create a simple graph
        graph = Graph(source_con, GRAPH_ID)
        replica = Graph(replica_con, GRAPH_ID)
        s = Node(label='L', properties={'id': 0, 'name': 'abcd'})
        t = Node(label='L', properties={'id': 1, 'name': 'efgh'})
        e = Edge(s, 'R', t)

        graph.add_node(s)
        graph.add_node(t)
        graph.add_edge(e)
        graph.flush()

        # create index
        create_node_exact_match_index(graph, 'L', 'id', sync=True)

        # create full-text index
        q = "CALL db.idx.fulltext.createNodeIndex('L', 'name')"
        graph.query(q)

        # add fields to existing index
        q = "CALL db.idx.fulltext.createNodeIndex('L', 'title', 'desc')"
        graph.query(q)

        # create full-text index with index config
        q = "CALL db.idx.fulltext.createNodeIndex({label: 'L1', language: 'german', stopwords: ['a', 'b'] }, 'title', 'desc')"
        graph.query(q)

        # update entity
        q = "MATCH (n:L {id:1}) SET n.id = 2"
        graph.query(q)

        # delete entity
        q = "MATCH (n:L {id:0}) DELETE n"
        graph.query(q)

        # the WAIT command forces master slave sync to complete
        source_con.execute_command("WAIT", "1", "0")

        # make sure index is available on replica
        q = "MATCH (s:L {id:2}) RETURN s.name"
        plan = graph.execution_plan(q)
        replica_plan = replica.execution_plan(q)
        env.assertIn("Index Scan", plan)
        env.assertEquals(replica_plan, plan)

        # issue query on both source and replica
        # make sure results are the same
        result = graph.query(q).result_set
        replica_result = replica.query(q).result_set
        env.assertEquals(replica_result, result)

        # make sure node count on both primary and replica is the same
        q = "MATCH (n) RETURN count(n)"
        result = graph.query(q).result_set
        replica_result = replica.query(q).result_set
        env.assertEquals(replica_result, result)

        # make sure nodes are in sync
        q = "MATCH (n) RETURN n ORDER BY n"
        result = graph.query(q).result_set
        replica_result = replica.query(q).result_set
        env.assertEquals(replica_result, result)

        # remove label
        q = "MATCH (s:L {id:2}) REMOVE s:L RETURN s"
        result = graph.query(q)
        env.assertEqual(result.labels_removed, 1)

        # the WAIT command forces master slave sync to complete
        source_con.execute_command("WAIT", "1", "0")

        q = "MATCH (s:L {id:2}) RETURN s"
        result = graph.query(q).result_set
        replica_result = replica.query(q).result_set
        env.assertEquals(replica_result, result)
        env.assertEqual(len(result), 0)

        # remove property
        q = "MATCH (s {id:2}) SET s.id = NULL RETURN s"
        result = graph.query(q)
        env.assertEqual(result.properties_removed, 1)

        # the WAIT command forces master slave sync to complete
        source_con.execute_command("WAIT", "1", "0")

        q = "MATCH (s {id:2}) RETURN s"
        result = graph.query(q).result_set
        replica_result = replica.query(q).result_set
        env.assertEquals(replica_result, result)
        env.assertEqual(len(result), 0)


        # make sure both primary and replica have the same set of indexes
        q = "CALL db.indexes() YIELD type, label, properties, language, stopwords, entitytype"
        result = graph.query(q).result_set
        replica_result = replica.query(q).result_set
        env.assertEquals(replica_result, result)

        # drop fulltext index
        q = "CALL db.idx.fulltext.drop('L')"
        graph.query(q)

        # the WAIT command forces master slave sync to complete
        source_con.execute_command("WAIT", "1", "0")

        # make sure both primary and replica have the same set of indexes
        q = "CALL db.indexes() YIELD type, label, properties, language, stopwords, entitytype"
        result = graph.query(q).result_set
        replica_result = replica.query(q).result_set
        env.assertEquals(replica_result, result)
