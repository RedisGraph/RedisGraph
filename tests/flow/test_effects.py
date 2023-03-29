import time
import threading
from common import *

GRAPH_ID = "effects"

class testEffects():
    async def wait_for_command(self):
        monitor_response = await self.monitor.next_command()
        return monitor_response
    
    # enable effects replication
    def effects_enable(self):
        self.master.execute_command("GRAPH.CONFIG", "SET", "REPLICATE_EFFECTS", 'YES')
        self.replica.execute_command("GRAPH.CONFIG", "SET", "REPLICATE_EFFECTS", 'YES')

    # disable effects replication
    def effects_disable(self):
        res = self.master.execute_command("GRAPH.CONFIG", "SET", "REPLICATE_EFFECTS", 'NO')

    # checks if effects replication is enabled
    def effects_enabled(self):
        conf = self.master.execute_command("GRAPH.CONFIG", "GET", "REPLICATE_EFFECTS")
        return (conf[1] == 1)

    # checks if effects replication is enabled
    def effects_disabled(self):
        return not self.effects_enabled()

    def monitor_thread(self):
        try:
            with self.replica.monitor() as m:
                for cmd in m.listen():
                    if 'GRAPH.EFFECT' in cmd['command']:
                        self.effects.append(cmd['command'][len('GRAPH.EFFECT ' + GRAPH_ID):])
        except:
            pass

    def wait_for_effect(self, timeout=1):
        # wait for replica to ack write
        self.master.wait(1, 400)

        # wait for monitor to receive effects
        interval = 0.1
        while len(self.effects) == 0 and timeout > 0:
            time.sleep(interval)
            timeout -= interval

        if len(self.effects) > 0:
            return self.effects.pop()
        else:
            return None

    # asserts that master and replica have the same view over the graph
    def assert_graph_eq(self):
        q = "MATCH (n) RETURN n ORDER BY(n)"
        master_resultset = self.master_graph.query(q).result_set
        replica_resultset = self.replica_graph.query(q, read_only=True).result_set
        self.env.assertEquals(master_resultset, replica_resultset)

        q = "MATCH ()-[e]->() RETURN e ORDER BY(e)"
        master_resultset = self.master_graph.query(q).result_set
        replica_resultset = self.replica_graph.query(q, read_only=True).result_set
        self.env.assertEquals(master_resultset, replica_resultset)

    def __init__(self):
        self.env = Env(decodeResponses=True, env='oss', useSlaves=True)
        self.effects = []
        self.master = self.env.getConnection()
        self.replica = self.env.getSlaveConnection()
        self.master_graph = Graph(self.master, GRAPH_ID)
        self.replica_graph = Graph(self.replica, GRAPH_ID)

        self.monitor_thread = threading.Thread(target=self.monitor_thread)
        self.monitor_thread.start()

    def __del__(self):
        # all done, shutdown replica
        # stops monitor thread
        self.replica.shutdown()
    
    def test01_effect_default_config(self):
        # make sure effects are enabled by default
        self.env.assertTrue(self.effects_enabled())

    def test02_add_schema_effect(self, expect_effect=True):
        # test the introduction of a schema by an effect

        # introduce a new label which in turn creates a new schema
        q = "CREATE (:L)"
        res = self.master_graph.query(q)
        self.env.assertEquals(res.labels_added, 1)
        self.env.assertEquals(res.nodes_created, 1)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

        # introduce multiple labels
        q = "CREATE (:X:Y)"
        res = self.master_graph.query(q)
        self.env.assertEquals(res.labels_added, 2)
        self.env.assertEquals(res.nodes_created, 1)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

        # introduce a new relationship-type which in turn creates a new schema
        q = "CREATE ()-[:R]->()"
        res = self.master_graph.query(q)
        self.env.assertEquals(res.relationships_created, 1)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

    def test03_add_attribute_effect(self, expect_effect=True):
        # test the introduction of an attribute by an effect

        # no leftovers from previous test
        self.env.assertEquals(len(self.effects), 0)

        # set a new attribute for each supported attribute type
        q = """MATCH (n:L) WITH n
                LIMIT 1
                SET
                n.a = 1,
                n.b = 'str',
                n.c = True,
                n.d = [1, [2], '3']
            """

        res = self.master_graph.query(q)
        self.env.assertEquals(res.properties_set, 4)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        q = """MATCH ()-[e]->()
                WITH e
                LIMIT 1
                SET
                e.e = point({latitude: 51, longitude: 0}),
                e.f=3.14,
                e.empty_string = ''
            """

        res = self.master_graph.query(q)
        self.env.assertEquals(res.properties_set, 3)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

    def test04_create_node_effect(self, expect_effect=True):
        # test the introduction of a new node by an effect

        # no leftovers from previous test
        self.env.assertEquals(len(self.effects), 0)

        # empty node
        q0 = """CREATE ()"""

        # label-less node with attributes
        q1 = """CREATE ({
                            i:1,
                            s:'str',
                            b:True,
                            a:[1, [2], '3'],
                            p:point({latitude: 51, longitude: 0}),
                            f:3.14,
                            empty_string: ''
                        })"""

        # labeled node without attributes
        q2 = """CREATE (:L)"""

        # node with multiple labels and attributes
        q3 = """CREATE (:A:B {
                            i:1,
                            s:'str',
                            b:True,
                            a:[1, [2], '3'],
                            p:point({latitude: 51, longitude: 0}),
                            f:3.14,
                            empty_string: ''
                        })"""

        queries = [q0, q1, q2, q3]
        for q in queries:
            res = self.master_graph.query(q)
            self.env.assertEquals(res.nodes_created, 1)

            if(expect_effect):
                self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

    def test05_create_edge_effect(self, expect_effect=True):
        # tests the introduction of a new edge by an effect

        # no leftovers from previous test
        self.env.assertEquals(len(self.effects), 0)

        # edge without attributes
        q1 = """CREATE ()-[:R]->()"""

        # edge with attributes
        q2 = """CREATE ()-[:connect {
                                      ei:1,
                                      s:'str',
                                      eb:True,
                                      a:[1, [2], '3'],
                                      ep:point({latitude: 51, longitude: 0}),
                                      f:3.14,
                                      empty_string: ''}
                            ]->()"""

        # edge between an existing node and a new node
        q3 = """MATCH (a) WITH a LIMIT 1 CREATE (a)-[:R]->()"""

        # edge between two existing nodes
        q4 = """MATCH (a), (b) WITH a, b LIMIT 1 CREATE (a)-[:R]->(b)"""

        queries = [q1, q2, q3, q4]
        for q in queries:
            result = self.master_graph.query(q)
            self.env.assertEquals(result.relationships_created, 1)

            if(expect_effect):
                self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

    def test06_update_node_effect(self, expect_effect=True):
        # test an entity attribute set update by an effect

        # no leftovers from previous test
        self.env.assertEquals(len(self.effects), 0)

        q = """MATCH (n:L)
               WITH n
               LIMIT 1
               SET
                    n.xa = 2,
                    n.b = 'string',
                    n.xc = False,
                    n.d = [[2], 1, '3'],
                    n.xe = point({latitude: 41, longitude: 2}),
                    n.f=6.28,
                    n.xempty_string = ''"""

        res = self.master_graph.query(q)
        self.env.assertGreater(res.properties_set, 0)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

        # update using map overwrite
        q = """MATCH (n:L)
               WITH n
               LIMIT 1
               SET n = {
                a:3,
                b:'_string_',
                c:True,
                d:[['3'], 2, 1],
                e:point({latitude: 2, longitude: 41}),
                f:2.68,
                empty_string:''}"""

        res = self.master_graph.query(q)
        self.env.assertGreater(res.properties_set, 0)
        self.env.assertGreater(res.properties_removed, 0)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

        # update using map addition
        q = """MATCH (n:L)
               WITH n
               LIMIT 1
               SET n += {
                a:4,
                b:'string_',
                c:False,
                d:[['1'], 3, 2.0],
                e:point({latitude: 3, longitude: 40}),
                f:8.26,
                empty_string:''}"""

        res = self.master_graph.query(q)
        self.env.assertGreater(res.properties_set, 0)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

        # remove attribute

        q = "MATCH (n:L) WITH n LIMIT 1 SET n.b = NULL"

        res = self.master_graph.query(q)
        self.env.assertEquals(res.properties_removed, 1)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

        # remove all attributes

        q = "MATCH (n:L) WITH n LIMIT 1 SET n = {}"

        res = self.master_graph.query(q)
        self.env.assertGreater(res.properties_removed, 0)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

    def test07_update_edge_effect(self, expect_effect=True):

        # no leftovers from previous test
        self.env.assertEquals(len(self.effects), 0)

        # test an edge attribute set update by an effect
        q = """MATCH ()-[e]->()
               WITH e
               LIMIT 1
               SET
                    e.a = 2,
                    e.b = 'string',
                    e.c = False,
                    e.d = [[2], 1, '3'],
                    e.e = point({latitude: 41, longitude: 2}),
                    e.f=6.28,
                    e.empty_string = ''"""

        res = self.master_graph.query(q)
        self.env.assertGreater(res.properties_set, 0)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

        # update using map overwrite
        q = """MATCH ()-[e]->()
               WITH e
               LIMIT 1
               SET e = {
                a:3,
                b:'_string_',
                c:True,
                d:[['3'], 2, 1],
                e:point({latitude: 2, longitude: 41}),
                f:2.68,
                empty_string:''}"""

        res = self.master_graph.query(q)
        self.env.assertGreater(res.properties_set, 0)
        self.env.assertGreater(res.properties_removed, 0)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

        # update using map addition
        q = """MATCH ()-[e]->()
               WITH e
               LIMIT 1
               SET e += {
                a:4,
                b:'string_',
                c:False,
                d:[['1'], 3, 2.0],
                e:point({latitude: 3, longitude: 40}),
                f:8.26,
                empty_string:''}"""

        res = self.master_graph.query(q)
        self.env.assertGreater(res.properties_set, 0)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

        # remove attribute

        q = "MATCH ()-[e]->() WITH e LIMIT 1 SET e.b = NULL"

        res = self.master_graph.query(q)
        self.env.assertEquals(res.properties_removed, 1)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

        # remove all attributes

        q = "MATCH ()-[e]->() WITH e LIMIT 1 SET e = {}"

        res = self.master_graph.query(q)
        self.env.assertGreater(res.properties_removed, 0)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

    def test08_set_labels_effect(self, expect_effect=True):
        # test the addition of a new node label by an effect

        # no leftovers from previous test
        self.env.assertEquals(len(self.effects), 0)

        q = """MATCH (n:A:B) SET n:C"""
        result = self.master_graph.query(q)
        self.env.assertEquals(result.labels_added, 1)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

        # test the addition of an existing and anew node label by an effect
        q = """MATCH (n:A:B:C) SET n:C:D"""
        result = self.master_graph.query(q)
        self.env.assertEquals(result.labels_added, 1)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

    def test09_remove_labels_effect(self, expect_effect=True):
        # test the removal of a node label by an effect

        # no leftovers from previous test
        self.env.assertEquals(len(self.effects), 0)

        q = """MATCH (n:C) REMOVE n:C RETURN n"""
        result = self.master_graph.query(q)
        self.env.assertEquals(result.labels_removed, 1)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

    def test10_delete_edge_effect(self, expect_effect=True):
        # test the deletion of an edge by an effect

        # no leftovers from previous test
        self.env.assertEquals(len(self.effects), 0)

        q = """MATCH ()-[e]->() WITH e LIMIT 1 DELETE e"""
        result = self.master_graph.query(q)
        self.env.assertEquals(result.relationships_deleted, 1)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

    def test11_delete_node_effect(self, expect_effect=True):
        # test the deletion of a node by an effect

        # no leftovers from previous test
        self.env.assertEquals(len(self.effects), 0)

        # using 'n' and 'x' to try and introduce "duplicated" deletions
        q = "MATCH (n) WITH n as n, n as x DELETE n, x"
        res = self.master_graph.query(q)
        self.env.assertGreater(res.nodes_deleted, 1)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

    def test12_merge_node(self, expect_effect=True):
        # test create and update of a node by an effect

        # no leftovers from previous test
        self.env.assertEquals(len(self.effects), 0)

        q = """MERGE (n:A {v:'red'})
               ON MATCH SET n.v = 'green'
               ON CREATE SET n.v = 'blue'"""
        res = self.master_graph.query(q)
        self.env.assertEquals(res.nodes_created, 1)
        self.env.assertEquals(res.properties_set, 2)
        self.env.assertEquals(res.properties_removed, 1)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

        # this time MERGE will match
        q = """MERGE (n:A {v:'blue'})
               ON MATCH SET n.v = 'green'
               ON CREATE SET n.v = 'red'"""
        res = self.master_graph.query(q)
        self.env.assertEquals(res.properties_set, 1)
        self.env.assertEquals(res.properties_removed, 1)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

    def test13_merge_edge(self, expect_effect=True):
        # test create and update of an edge by an effect

        # no leftovers from previous test
        self.env.assertEquals(len(self.effects), 0)

        q = """MERGE (n:A {v:'red'})
               MERGE (n)-[e:R{v:'red'}]->(n)
               ON MATCH SET e.v = 'green'
               ON CREATE SET e.v = 'blue'"""
        res = self.master_graph.query(q)
        self.env.assertEquals(res.properties_set, 3)
        self.env.assertEquals(res.relationships_created, 1)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

        # this time MERGE will match
        q = """MERGE (n:A {v:'red'})
               MERGE (n)-[e:R{v:'blue'}]->(n)
               ON MATCH SET e.v = 'green'
               ON CREATE SET e.v = 'red'"""
        res = self.master_graph.query(q)
        self.env.assertEquals(res.properties_set, 1)
        self.env.assertEquals(res.properties_removed, 1)

        if(expect_effect):
            self.env.assertIsNotNone(self.wait_for_effect())

        self.assert_graph_eq()

    def test14_large_num_of_chages(self, expect_effect=True):
        # make sure a query which introduces a large number of changes
        # doesn't replicate effects

        # no leftovers from previous test
        self.env.assertEquals(len(self.effects), 0)

        q = """MATCH (n)
               WITH n
               LIMIT 1
               UNWIND range(0, 10000) as x
               SET n.v = x"""

        res = self.master_graph.query(q)
        self.env.assertEquals(res.properties_set, 10001)

        # make sure no effect been sent
        self.env.assertIsNone(self.wait_for_effect(), 0.5)

    def test15_rerun_disable_effects(self):
        # test replication works when effects are disabled

        # no leftovers from previous test
        self.env.assertEquals(len(self.effects), 0)

        # update graph key
        global GRAPH_ID
        GRAPH_ID = "effects_disabled"

        # update graph objects to use new graph key
        self.master_graph = Graph(self.master, GRAPH_ID)
        self.replica_graph = Graph(self.replica, GRAPH_ID)

        # disable effects replication
        self.effects_disable()

        # re-run tests, this time effects is turned off
        # replication should be done via query replication
        self.test02_add_schema_effect(False)
        self.test03_add_attribute_effect(False)
        self.test04_create_node_effect(False)
        self.test05_create_edge_effect(False)
        self.test06_update_node_effect(False)
        self.test07_update_edge_effect(False)
        self.test08_set_labels_effect(False)
        self.test09_remove_labels_effect(False)
        self.test10_delete_edge_effect(False)
        self.test11_delete_node_effect(False)
        self.test12_merge_node(False)
        self.test13_merge_edge(False)
        self.test14_large_num_of_chages(False)

        # make sure no effects had been recieved
        self.env.assertEquals(len(self.effects), 0)

