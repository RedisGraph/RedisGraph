import os
import sys
import string
import random
import unittest
from redisgraph import Graph, Node, Edge

import redis
from .base import FlowTestsBase

dis_redis = None
redis_graph = None
redis_con = None

def random_string(size=6, chars=string.ascii_letters):
    return ''.join(random.choice(chars) for _ in range(size))

def get_redis():
    global dis_redis
    conn = redis.Redis()
    try:
        conn.ping()
        # Assuming RedisGraph is loaded.
    except redis.exceptions.ConnectionError:
        from .redis_base import DisposableRedis
        # Bring up our own redis-server instance.
        dis_redis = DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')
        dis_redis.start()
        conn = dis_redis.client()
    return conn


class NodeByIDFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "NodeByIDFlowTest"
        global redis_graph
        global redis_con
        redis_con = get_redis()
        GRAPH_ID = random_string()
        redis_graph = Graph(GRAPH_ID, redis_con)
        cls.populate_graph()

    @classmethod
    def tearDownClass(cls):
        if dis_redis is not None:
            dis_redis.stop()

    @classmethod
    def populate_graph(cls):
        global redis_graph

        # Create entities
        for i in range(10):            
            node = Node(label="person", properties={"id": i})
            redis_graph.add_node(node)
        redis_graph.commit()

        # Make sure node id attribute matches node's internal ID.
        query = """MATCH (n) SET n.id = ID(n)"""
        redis_graph.query(query)

    # Expect an error when trying to use a function which does not exists.
    def test_get_nodes(self):
        # All nodes, not including first node.
        query = """MATCH (n) WHERE ID(n) > 0 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id > 0 RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)

        query = """MATCH (n) WHERE 0 < ID(n) RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE 0 < n.id RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)
        
        # All nodes.
        query = """MATCH (n) WHERE ID(n) >= 0 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id >= 0 RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)

        query = """MATCH (n) WHERE 0 <= ID(n) RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE 0 <= n.id RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)
        
        # A single node.
        query = """MATCH (n) WHERE ID(n) = 0 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id = 0 RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)
        
        # 4 nodes (6,7,8,9)
        query = """MATCH (n) WHERE ID(n) > 5 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id > 5 RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)

        query = """MATCH (n) WHERE 5 < ID(n) RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE 5 < n.id RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)
        
        # 5 nodes (5, 6,7,8,9)
        query = """MATCH (n) WHERE ID(n) >= 5 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id >= 5 RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)

        query = """MATCH (n) WHERE 5 <= ID(n) RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE 5 <= n.id RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)
        
        # 5 nodes (0,1,2,3,4)
        query = """MATCH (n) WHERE ID(n) < 5 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id < 5 RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)

        query = """MATCH (n) WHERE 5 < ID(n) RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE 5 < n.id RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)
        
        # 6 nodes (0,1,2,3,4,5)
        query = """MATCH (n) WHERE ID(n) <= 5 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id <= 5 RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)

        query = """MATCH (n) WHERE 5 >= ID(n) RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE 5 >= n.id RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)
        
        # All nodes except last one.
        query = """MATCH (n) WHERE ID(n) < 9 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id < 9 RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)

        query = """MATCH (n) WHERE 9 > ID(n) RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE 9 > n.id RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)
        
        # All nodes.
        query = """MATCH (n) WHERE ID(n) <= 9 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id <= 9 RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)

        query = """MATCH (n) WHERE 9 >= ID(n) RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE 9 >= n.id RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)

        # All nodes.
        query = """MATCH (n) WHERE ID(n) < 100 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id < 100 RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)

        query = """MATCH (n) WHERE 100 > ID(n) RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE 100 > n.id RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)
        
        # All nodes.
        query = """MATCH (n) WHERE ID(n) <= 100 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id <= 100 RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)

        query = """MATCH (n) WHERE 100 >= ID(n) RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE 100 >= n.id RETURN n ORDER BY n.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)

        # cartesian product, tests reset works as expected.
        query = """MATCH (a), (b) WHERE ID(a) > 5 AND ID(b) <= 5 RETURN a,b ORDER BY a.id, b.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (a), (b) WHERE a.id > 5 AND b.id <= 5 RETURN a,b ORDER BY a.id, b.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)

        query = """MATCH (a), (b) WHERE 5 < ID(a) AND 5 >= ID(b) RETURN a,b ORDER BY a.id, b.id"""
        resultsetA = redis_graph.query(query).result_set
        self.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (a), (b) WHERE 5 < a.id AND 5 >= b.id RETURN a,b ORDER BY a.id, b.id"""
        self.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.assertEqual(resultsetA, resultsetB)

if __name__ == '__main__':
    unittest.main()
