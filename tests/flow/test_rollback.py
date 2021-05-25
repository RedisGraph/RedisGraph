import sys
from RLTest import Env
from base import FlowTestsBase
from redis import ResponseError
from redisgraph import Graph, Node

redis_con = None
redis_graph = None
redis_graph = None
people = ["Roi", "Alon", "Ailon", "Boaz", "Tal", "Omri", "Ori"]

class testRollback(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
        global redis_graph
        redis_con = self.env.getConnection()
        redis_graph = Graph("G", redis_con)
        self.populate_graph()

    def populate_graph(self):
        global redis_graph
        
        nodes = {}
         # Create entities        
        for p in people:
            node = Node(label="person", properties={"v": 2})
            redis_graph.add_node(node)
            nodes[p] = node

        redis_graph.commit()

    def test01_update_rollback(self):
        query = "MATCH (n) SET n.v = 1 WITH n AS n SET n.v = 1 * ‘str’"
        try:
            # The query is expected to fail
            redis_graph.query(query)
            assert(False)
        except ResponseError as error:
            self.env.assertContains("Invalid input", str(error))

        # Verify that query was completely rollbacked
        query2 = "MATCH (n) RETURN n.v"
        actual_result = redis_graph.query(query2)
        v = actual_result.result_set[0][0]
        self.env.assertEquals(v, 2)

    def test02_create_rollback(self):
        query = "UNWIND range(0,20) as i CREATE (n:Node {t: i}) WITH n AS n SET n.v = 1 * ‘str’"
        try:
            # The query is expected to fail
            redis_graph.query(query)
            assert(False)
        except ResponseError as error:
            self.env.assertContains("Invalid input", str(error))

        # Verify that query was completely rollbacked
        query2 = "MATCH (n:Node {t: 1}) RETURN count(n)"
        try:
            actual_result = redis_graph.query(query2)
            # If query rolledback then count should be 0
            self.env.assertEquals(actual_result.result_set, [])
        except ResponseError as error:
            assert(False)
