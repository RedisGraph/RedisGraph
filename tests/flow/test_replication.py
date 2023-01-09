from common import *
from index_utils import *
from constraint_utils import *
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
        s = Node(label='L', properties={'id': 0, 'name': 'abcd', 'height' : 178})
        t = Node(label='L', properties={'id': 1, 'name': 'efgh', 'height' : 178})
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

        # create node unique constraint
        assert create_node_unique_constraint(graph, "L", "id", sync=True)

        # add another unique constraint
        assert create_node_unique_constraint(graph, "L", "id", "name", sync=True)

        # add failed unique constraint
        graph.query("CREATE (:FISH {age: 10, name: 'sharki'})")
        graph.query("CREATE (:FISH {age: 10, name: 'goldi'})")
        assert create_node_unique_constraint(graph, "FISH", "age", sync=False)
        wait_on_constraint_to_fail(graph, "FISH", "unique")

        # add failed unique constraint which should be replicated as Pending and then fail
        graph.query("UNWIND range(0,1000000) AS x CREATE (:CAT {age: x, height: x + 1})")
        graph.query("CREATE (:CAT {age: 10})")
        assert create_node_unique_constraint(graph, "CAT", "age", sync=False)

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


        # make sure both primary and replica have the same set of constraints
        q = "CALL db.constraints() YIELD type, label, properties, entitytype WHERE label = 'L' RETURN *"
        result = graph.query(q).result_set
        replica_result = replica.query(q).result_set
        env.assertEquals(replica_result, result)

        # drop constraint
        assert drop_node_unique_constraint(graph, "L", "id")

        # the WAIT command forces master slave sync to complete
        source_con.execute_command("WAIT", "1", "0")

        # make sure both primary and replica have the same set of constraints
        q = "CALL db.constraints() YIELD type, label, properties, entitytype WHERE label = 'L' RETURN *"
        result = graph.query(q).result_set
        replica_result = replica.query(q).result_set
        env.assertEquals(replica_result, result)

        # make sure both primary and replica have the same set of constraints
        q = "CALL db.constraints() YIELD type, label, properties, entitytype WHERE label = 'FISH' RETURN *"
        result = graph.query(q).result_set
        replica_result = replica.query(q).result_set
        env.assertEquals(result, [['NODE', 'FISH', ['age'], 'unique']])
        env.assertEquals(replica_result, []) # we are not replicating failed constraints

        # drop failed constraint
        assert drop_node_unique_constraint(graph, "FISH", "age")

        # the WAIT command forces master slave sync to complete
        source_con.execute_command("WAIT", "1", "0")

        # make sure both primary and replica have the same set of constraints
        q = "CALL db.constraints() YIELD type, label, properties, entitytype WHERE label = 'L' RETURN *"
        result = graph.query(q).result_set
        replica_result = replica.query(q).result_set
        env.assertEquals(replica_result, result)

        # make sure both primary and replica have the same set of constraints
        q = "CALL db.constraints() YIELD type, label, properties, entitytype WHERE label = 'FISH' RETURN *"
        result = graph.query(q).result_set
        replica_result = replica.query(q).result_set
        env.assertEquals(result, [])
        env.assertEquals(replica_result, []) # we are not replicating failed constraints

        # make sure both primary and replica have the same set of constraints
        q = "CALL db.constraints() YIELD type, label, properties, entitytype WHERE label = 'CAT' RETURN *"
        result = graph.query(q).result_set
        replica_result = replica.query(q).result_set
        env.assertEquals(replica_result, result)

        # drop failed constraint
        assert drop_node_unique_constraint(graph, "CAT", "age")

        # the WAIT command forces master slave sync to complete
        source_con.execute_command("WAIT", "1", "0")

        # make sure both primary and replica have the same set of constraints
        q = "CALL db.constraints() YIELD type, label, properties, entitytype WHERE label = 'CAT' RETURN *"
        result = graph.query(q).result_set
        replica_result = replica.query(q).result_set
        env.assertEquals(replica_result, result)