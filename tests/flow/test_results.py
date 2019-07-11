import os
import sys
from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

graph = None
redis_con = None

people = ["Roi", "Alon", "Ailon", "Boaz"]


class testResultSetFlow(FlowTestsBase):
    def __init__(self):
        super(testResultSetFlow, self).__init__()
        global graph
        global redis_con
        redis_con = self.env.getConnection()
        graph = Graph("G", redis_con)
        self.populate_graph()

    def populate_graph(self):
        global graph
        nodes = {}
        # Create entities
        for idx, p in enumerate(people):
            node = Node(label="person", properties={"name": p, "val": idx})
            graph.add_node(node)
            nodes[p] = node

        # Fully connected graph
        for src in nodes:
            for dest in nodes:
                if src != dest:
                    edge = Edge(nodes[src], "know", nodes[dest])
                    graph.add_edge(edge)

        graph.commit()


    # Verify that scalar returns function properly
    def test01_return_scalars(self):
        query = """MATCH (a) RETURN a.name, a.val ORDER BY a.val"""
        result = graph.query(query)

        expected_result = [['Roi', 0],
                           ['Alon', 1],
                           ['Ailon', 2],
                           ['Boaz', 3]]

        self.env.assertEquals(len(result.result_set), 4)
        self.env.assertEquals(len(result.header), 2) # 2 columns in result set
        self.env.assertEquals(result.result_set, expected_result)

    # Verify that full node returns function properly
    def test02_return_nodes(self):
        query = """MATCH (a) RETURN a"""
        result = graph.query(query)

        # TODO add more assertions after updated client format is defined
        self.env.assertEquals(len(result.result_set), 4)
        self.env.assertEquals(len(result.header), 1) # 1 column in result set

    # Verify that full edge returns function properly
    def test03_return_edges(self):
        query = """MATCH ()-[e]->() RETURN e"""
        result = graph.query(query)

        # TODO add more assertions after updated client format is defined
        self.env.assertEquals(len(result.result_set), 12) # 12 relations (fully connected graph)
        self.env.assertEquals(len(result.header), 1) # 1 column in result set

    def test04_mixed_returns(self):
        query = """MATCH (a)-[e]->() RETURN a.name, a, e ORDER BY a.val"""
        result = graph.query(query)

        # TODO add more assertions after updated client format is defined
        self.env.assertEquals(len(result.result_set), 12) # 12 relations (fully connected graph)
        self.env.assertEquals(len(result.header), 3) # 3 columns in result set


    # Verify that the DISTINCT operator works with full entity returns
    def test05_distinct_full_entities(self):
        graph2 = Graph("H", redis_con)
        query = """CREATE (a)-[:e]->(), (a)-[:e]->()"""
        result = graph2.query(query)
        self.env.assertEquals(result.nodes_created, 3)
        self.env.assertEquals(result.relationships_created, 2)

        query = """MATCH (a)-[]->() RETURN a"""
        non_distinct = graph2.query(query)
        query = """MATCH (a)-[]->() RETURN DISTINCT a"""
        distinct = graph2.query(query)

        self.env.assertEquals(len(non_distinct.result_set), 2)
        self.env.assertEquals(len(distinct.result_set), 1)
