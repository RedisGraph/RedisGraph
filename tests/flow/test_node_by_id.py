import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

GRAPH_ID = "g"
redis_graph = None

class testNodeByIDFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph(GRAPH_ID, redis_con)
        self.populate_graph()

    def populate_graph(self):
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
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id > 0 RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        query = """MATCH (n) WHERE 0 < ID(n) RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE 0 < n.id RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        # All nodes.
        query = """MATCH (n) WHERE ID(n) >= 0 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id >= 0 RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        query = """MATCH (n) WHERE 0 <= ID(n) RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE 0 <= n.id RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        # A single node.
        query = """MATCH (n) WHERE ID(n) = 0 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id = 0 RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        # 4 nodes (6,7,8,9)
        query = """MATCH (n) WHERE ID(n) > 5 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id > 5 RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        query = """MATCH (n) WHERE 5 < ID(n) RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE 5 < n.id RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        # 5 nodes (5, 6,7,8,9)
        query = """MATCH (n) WHERE ID(n) >= 5 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id >= 5 RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        query = """MATCH (n) WHERE 5 <= ID(n) RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE 5 <= n.id RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        # 5 nodes (0,1,2,3,4)
        query = """MATCH (n) WHERE ID(n) < 5 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id < 5 RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        query = """MATCH (n) WHERE 5 < ID(n) RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE 5 < n.id RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        # 6 nodes (0,1,2,3,4,5)
        query = """MATCH (n) WHERE ID(n) <= 5 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id <= 5 RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        query = """MATCH (n) WHERE 5 >= ID(n) RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE 5 >= n.id RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        # All nodes except last one.
        query = """MATCH (n) WHERE ID(n) < 9 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id < 9 RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        query = """MATCH (n) WHERE 9 > ID(n) RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE 9 > n.id RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        # All nodes.
        query = """MATCH (n) WHERE ID(n) <= 9 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id <= 9 RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        query = """MATCH (n) WHERE 9 >= ID(n) RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE 9 >= n.id RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        # All nodes.
        query = """MATCH (n) WHERE ID(n) < 100 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id < 100 RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        query = """MATCH (n) WHERE 100 > ID(n) RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE 100 > n.id RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        # All nodes.
        query = """MATCH (n) WHERE ID(n) <= 100 RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE n.id <= 100 RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        query = """MATCH (n) WHERE 100 >= ID(n) RETURN n ORDER BY n.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (n) WHERE 100 >= n.id RETURN n ORDER BY n.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        # cartesian product, tests reset works as expected.
        query = """MATCH (a), (b) WHERE ID(a) > 5 AND ID(b) <= 5 RETURN a,b ORDER BY a.id, b.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (a), (b) WHERE a.id > 5 AND b.id <= 5 RETURN a,b ORDER BY a.id, b.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

        query = """MATCH (a), (b) WHERE 5 < ID(a) AND 5 >= ID(b) RETURN a,b ORDER BY a.id, b.id"""
        resultsetA = redis_graph.query(query).result_set
        self.env.assertIn("NodeByIdSeek", redis_graph.execution_plan(query))
        query = """MATCH (a), (b) WHERE 5 < a.id AND 5 >= b.id RETURN a,b ORDER BY a.id, b.id"""
        self.env.assertNotIn("NodeByIdSeek", redis_graph.execution_plan(query))
        resultsetB = redis_graph.query(query).result_set
        self.env.assertEqual(resultsetA, resultsetB)

    # Try to fetch none existing entities by ID(s).
    def test_for_none_existing_entity_ids(self):
        # Try to fetch an entity with a none existing ID.
        queries = ["""MATCH (a:person) WHERE ID(a) = 999 RETURN a""",
                    """MATCH (a:person) WHERE ID(a) > 999 RETURN a""",
                    """MATCH (a:person) WHERE ID(a) > 800 AND ID(a) < 900 RETURN a"""]

        for query in queries:
            resultset = redis_graph.query(query).result_set        
            self.env.assertEquals(len(resultset), 0)    # Expecting no results.
            self.env.assertIn("Node By Label and ID Scan", redis_graph.execution_plan(query))
