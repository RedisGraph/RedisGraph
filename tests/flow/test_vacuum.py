from common import *

GRAPH_ID = "vacuum"

class testVacuum():
    def __init__(self):
        self.env = Env(decodeResponses=True)
        self.conn = self.env.getConnection()
        self.g = Graph(self.conn, GRAPH_ID)
        self.block_size = self.g.config("NODE_CREATION_BUFFER")[1]

    def vacuum(self, key):
        self.conn.execute_command("GRAPH.VACUUM", key)

    def redis_used_memory(self):
        return self.conn.info('memory')['used_memory']

    # vacuum none existing graph
    def test01_vacuum_none_existing_graph(self):
        # capture memory usage before vacuum
        original_mem_usage = self.redis_used_memory()

        # vacuum
        try:
            # expecting an exception
            self.vacuum("none_existing")
            self.env.assertTrue(False)
        except:
            pass

        # validate memory consumption did not dropped
        self.env.assertLessEqual(abs(original_mem_usage - self.redis_used_memory()),
                                 0.1 * original_mem_usage)

    # vacuum an empty graph
    def test02_vacuum_empty_graph(self):
        # create empty graph
        self.g.query("return 1")

        # capture memory usage before vacuum
        original_mem_usage = self.redis_used_memory()

        # vacuum
        self.vacuum(GRAPH_ID)

        # validate memory consumption did not dropped
        self.env.assertGreaterEqual(original_mem_usage, self.redis_used_memory())

    def test03_vacuum_no_fragmentation(self):
        # create a graph with no fragmentation
        # i.e. no deleted entities

        g = self.g
        res = g.query(f"UNWIND range(1, {self.block_size}-1) as x CREATE (:L)")
        self.env.assertEqual(res.nodes_created, self.block_size-1)

        # capture memory usage before vacuum
        original_mem_usage = self.redis_used_memory()

        # vacuum
        self.vacuum(GRAPH_ID)

        # validate memory consumption did not dropped
        self.env.assertGreaterEqual(original_mem_usage, self.redis_used_memory())

    def test_04_vacuum_fragmented_graph(self):
        g = self.g

        # create nodes
        res = g.query(f"UNWIND range(1, {self.block_size} * 10) as x CREATE (:L)")
        self.env.assertEqual(res.nodes_created, self.block_size * 10)

        node_count = g.query("MATCH (n) RETURN count(n)").result_set[0][0]

        # delete nodes with odd ID
        res = g.query("MATCH (n) WHERE ID(n) % 2 = 1 DELETE n")

        # validate half of the nodes been deleted
        self.env.assertTrue(int(res.nodes_deleted) == int((node_count / 2)))

        # capture memory usage before vacuum
        original_mem_usage = self.redis_used_memory()

        # vacuum
        self.vacuum(GRAPH_ID)

        # validate memory consumption did not dropped
        new_mem_usage = self.redis_used_memory()
        self.env.assertLess(new_mem_usage, original_mem_usage)

        # expecting at least 25% memory reduction
        self.env.assertGreaterEqual(new_mem_usage / original_mem_usage, 0.25)

    def test_05_entity_migration(self):
        # make sure migrated node maintains its attribute-set and labels

        # flush
        self.env.getConnection().flushall()
        self.g = Graph(self.conn, GRAPH_ID)

        # create the following graph:
        # (0:A:B:C {v:0}), (1:C:D:E {v:1})
        #
        # delete node with ID 0
        #
        # vacuum
        #
        # expecting node with ID 1 to migrate

        res = self.g.query("CREATE (:A:B:C {v:0})")
        self.env.assertEqual(res.nodes_created, 1)

        res = self.g.query("CREATE (:C:D:E {v:1})")
        self.env.assertEqual(res.nodes_created, 1)

        # delete node with internal ID 0
        res = self.g.query("MATCH (n:A:B:C {v:0}) WHERE ID(n) = 0 DELETE n")
        self.env.assertEqual(res.nodes_deleted, 1)

        self.vacuum(GRAPH_ID)

        res = self.g.query("MATCH (n:C:D:E {v:1}) RETURN n")
        n = res.result_set[0][0]
        self.env.assertEqual(n.id, 0)

    def test_06_migrate_entity_between_blocks(self):
        # create a graph big enough such that a migrated entity crosses
        # several blocks

        # flush
        self.env.getConnection().flushall()
        self.g = Graph(self.conn, GRAPH_ID)

        # create enough nodes to occupy 5 blocks
        res = self.g.query(f"UNWIND range(0, {self.block_size} * 5) as x CREATE (:L{{v:x}})")
        self.env.assertEqual(int(res.nodes_created)-1, int(self.block_size * 5))

        # create an additional node, this one is expected to migrate after vacuum
        self.g.query("CREATE (:L:A:B {v:-1})")

        # delete node with lowest ID
        res = self.g.query("MATCH (n) WHERE ID(n) = 0 DELETE n")
        self.env.assertEqual(res.nodes_deleted, 1)

        self.vacuum(GRAPH_ID)

        res = self.g.query("MATCH (n) WHERE ID(n) = 0 RETURN n")
        n = res.result_set[0][0]
        self.env.assertEqual(n.id, 0)
        self.env.assertEqual(len(n.labels), 3)
        self.env.assertTrue("L" in n.labels)
        self.env.assertTrue("A" in n.labels)
        self.env.assertTrue("B" in n.labels)
        self.env.assertEqual(len(n.properties), 1)
        self.env.assertEqual(n.properties['v'], -1)

    def test_07_entity_migration(self):
        # when performing vacuum nodes IDs might change
        # a node with ID 10 might get its ID decrement to 5

        # create the following graph:
        # (0:L{v:0}), (1:L:X{v:1}), (2:L:Y{v:2}), (3:L{v:3}), (4:L{v:4})
        # (0)-[]->(3)
        # (4)-[]->(0)
        #
        # delete nodes (1) and (2)
        # we're expecting the following graph after vacuum
        # (0), (1{v:4}), (2{v:3})
        # (0)-[]->(2)
        # (1)-[]->(0)
        #
        # as node 4 and 3 been relocated to 1 and 2

        # flush
        self.env.getConnection().flushall()
        self.g = Graph(self.conn, GRAPH_ID)

        # create graph
        # create nodes
        res = self.g.query("UNWIND range(0, 4) as x CREATE (:L {v:x})")
        self.env.assertEqual(res.nodes_created, 5)

        # update node 1 and 2 labels
        self.g.query("MATCH (a{v:1}) WHERE ID(a) = 1 SET a:X")
        self.g.query("MATCH (a{v:2}) WHERE ID(a) = 2 SET a:Y")

        # connect nodes 3 and 4 to 0
        self.g.query("MATCH (a{v:0}), (b{v:3}) WHERE ID(a) = 0 AND ID(b) = 3 CREATE (a)-[:R]->(b)")
        self.g.query("MATCH (a{v:0}), (b{v:4}) WHERE ID(a) = 0 AND ID(b) = 4 CREATE (b)-[:R]->(a)")

        # delete nodes 1 and 2
        res = self.g.query("MATCH (a), (b) WHERE ID(a) = 1 AND ID(b) = 2 DELETE a, b")
        self.env.assertEqual(res.nodes_deleted, 2)

        # vacuum, expecting node 4 and 3 to migrate to 1 and 2
        self.vacuum(GRAPH_ID)

        # make sure 4^ is still connected to 0
        res = self.g.query("MATCH (a{v:4})-[]->(b) RETURN a, b").result_set
        a = res[0][0]
        b = res[0][1]
        self.env.assertEqual(a.id, 1)
        self.env.assertEqual(b.id, 0)
        self.env.assertEqual(len(a.labels), 1)
        self.env.assertEqual(len(b.labels), 1)
        self.env.assertEqual(a.labels[0], "L")
        self.env.assertEqual(b.labels[0], "L")

        # reversed pattern
        # make sure 4^ is still connected to 0
        res = self.g.query("MATCH (a{v:0}) WITH a MATCH (a)<-[]-(b{v:4}) RETURN a, b").result_set
        a = res[0][0]
        b = res[0][1]
        self.env.assertEqual(b.id, 1)
        self.env.assertEqual(a.id, 0)
        self.env.assertEqual(len(a.labels), 1)
        self.env.assertEqual(len(b.labels), 1)
        self.env.assertEqual(a.labels[0], "L")
        self.env.assertEqual(b.labels[0], "L")

        # make sure 0 is still connected to 3^
        # (0)->(3^)
        res = self.g.query("MATCH (a)-[]->(b{v:3}) RETURN a, b").result_set
        a = res[0][0]
        b = res[0][1]
        self.env.assertEqual(a.id, 0)
        self.env.assertEqual(b.id, 2)
        self.env.assertEqual(len(a.labels), 1)
        self.env.assertEqual(len(b.labels), 1)
        self.env.assertEqual(a.labels[0], "L")
        self.env.assertEqual(b.labels[0], "L")

        # reversed pattern
        # make sure 0 is still connected to 3^
        # (0)->(3^)
        res = self.g.query("MATCH (a{v:0}) WITH a MATCH (a)-[]->(b{v:3}) RETURN a, b").result_set
        a = res[0][0]
        b = res[0][1]
        self.env.assertEqual(a.id, 0)
        self.env.assertEqual(b.id, 2)
        self.env.assertEqual(len(a.labels), 1)
        self.env.assertEqual(len(b.labels), 1)
        self.env.assertEqual(a.labels[0], "L")
        self.env.assertEqual(b.labels[0], "L")

    def test_08_no_migration(self):
        # create a graph where all of the deleted slots are at the end
        # no migration should occur

        # flush
        self.env.getConnection().flushall()
        self.g = Graph(self.conn, GRAPH_ID)

        # create graph with enough nodes to occupy 3 blocks
        self.g.query(f"UNWIND range(0, {self.block_size} * 3) as x CREATE (:L {{v:x}})")

        self.g.query("MATCH (n) RETURN count(n)")

        # delete 1000 nodes with biggest IDs
        self.g.query("MATCH (n) WITH n ORDER BY ID(n) DESC LIMIT 1000 DELETE n")

        expected = self.g.query("MATCH (n) RETURN n ORDER BY ID(n) LIMIT 300000").result_set

        self.vacuum(GRAPH_ID)

        actual = self.g.query("MATCH (n) RETURN n ORDER BY ID(n) LIMIT 300000").result_set

        self.env.assertEqual(expected, actual)

