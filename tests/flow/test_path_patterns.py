import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase


def create_pipe(redis_con):
    pipe = Graph("pipe", redis_con)
    node_props = ['v1', 'v2', 'v3', 'v4', 'v5']

    nodes = []
    for idx, v in enumerate(node_props):
        node = Node(label="L", properties={"val": v})
        nodes.append(node)
        pipe.add_node(node)

    edge = Edge(nodes[0], "A", nodes[1])
    pipe.add_edge(edge)

    edge = Edge(nodes[1], "A", nodes[2])
    pipe.add_edge(edge)

    edge = Edge(nodes[2], "B", nodes[3])
    pipe.add_edge(edge)

    edge = Edge(nodes[3], "B", nodes[4])
    pipe.add_edge(edge)

    pipe.commit()
    return pipe


def create_parallel_pipe(redis_con):
    parallel_pipe = Graph("parallel pipe", redis_con)
    node_props = ['v1', 'v2', 'v3', 'v4']

    nodes = []
    for idx, v in enumerate(node_props):
        node = Node(label="L", properties={"val": v})
        nodes.append(node)
        parallel_pipe.add_node(node)

    edge = Edge(nodes[0], "A", nodes[1])
    parallel_pipe.add_edge(edge)

    edge = Edge(nodes[0], "C", nodes[2])
    parallel_pipe.add_edge(edge)

    edge = Edge(nodes[1], "B", nodes[3])
    parallel_pipe.add_edge(edge)

    edge = Edge(nodes[2], "D", nodes[3])
    parallel_pipe.add_edge(edge)

    parallel_pipe.commit()
    return parallel_pipe


def create_bintree(redis_con):
    tree = Graph("bintree", redis_con)
    node_props = ['v1', 'v2', 'v3', 'v4', 'v5', 'v6', 'v7']

    nodes = []
    for idx, v in enumerate(node_props):
        node = Node(label="L", properties={"val": v})
        nodes.append(node)
        tree.add_node(node)

    edge = Edge(nodes[0], "A", nodes[1])
    tree.add_edge(edge)

    edge = Edge(nodes[0], "B", nodes[2])
    tree.add_edge(edge)

    edge = Edge(nodes[1], "C", nodes[3])
    tree.add_edge(edge)

    edge = Edge(nodes[1], "D", nodes[4])
    tree.add_edge(edge)

    edge = Edge(nodes[2], "C", nodes[5])
    tree.add_edge(edge)

    edge = Edge(nodes[2], "D", nodes[6])
    tree.add_edge(edge)

    tree.commit()
    return tree


def create_reversed_pipe(redis_con):
    reversed_pipe = Graph("reversed_pipe", redis_con)
    node_props = ['v1', 'v2', 'v3', 'v4', 'v5']

    nodes = []
    for idx, v in enumerate(node_props):
        node = Node(label="L", properties={"val": v})
        nodes.append(node)
        reversed_pipe.add_node(node)

    edge = Edge(nodes[0], "A", nodes[1])
    reversed_pipe.add_edge(edge)

    edge = Edge(nodes[2], "B", nodes[1])
    reversed_pipe.add_edge(edge)

    edge = Edge(nodes[3], "A", nodes[2])
    reversed_pipe.add_edge(edge)

    edge = Edge(nodes[3], "B", nodes[4])
    reversed_pipe.add_edge(edge)

    reversed_pipe.commit()
    return reversed_pipe


class testPathPattern(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        redis_con = self.env.getConnection()
        self.pipe_graph = create_pipe(redis_con)
        self.reversed_pipe_graph = create_reversed_pipe(redis_con)
        self.tree_graph = create_bintree(redis_con)
        self.parallel_pipe_graph = create_parallel_pipe(redis_con)

    # def test00_path_pattern_explain(self):
    #     query = """
    #     PATH PATTERN S = ()-/ :A [~S | ()] :B /-()
    #     MATCH (a)-/ ~S /->(b)
    #     RETURN a.val, b.val ORDER BY a.val, b.val"""
    #     self.pipe_graph.execution_plan(query)
    #
    # def test01_path_pattern_explain(self):
    #     query = """
    #     PATH PATTERN S1 = ()-/ :A :B /->()
    #     PATH PATTERN S2 = ()-/ ~S1 :B /->()
    #     MATCH (a)-/ ~S2 /->(b)
    #     RETURN a, b"""
    #     self.pipe_graph.execution_plan(query)

    def test00_path_pattern_execution(self):
        query = """
        MATCH (a)-/ :A> /->(b)
        RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = self.pipe_graph.query(query)
        expected_result = [['v1', 'v2'],
                           ['v2', 'v3']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test01_path_pattern_execution(self):
        query = """
        MATCH (a)-/ :A> | :B> /->(b)
        RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = self.pipe_graph.query(query)
        expected_result = [['v1', 'v2'],
                           ['v2', 'v3'],
                           ['v3', 'v4'],
                           ['v4', 'v5']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test02_path_pattern_execution(self):
        query = """
        MATCH (a)-/ :A> :B> /->(b)
        RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = self.pipe_graph.query(query)
        expected_result = [['v2', 'v4']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test03_path_pattern_execution(self):
        query = """
        MATCH (a)-/ [:A> | :B>]> [:C> | :D>]> /->(b)
        RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = self.tree_graph.query(query)
        expected_result = [['v1', 'v4'],
                           ['v1', 'v5'],
                           ['v1', 'v6'],
                           ['v1', 'v7']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    def test04_path_pattern_execution(self):
        query = """
        MATCH (a)-/ [:A> :B>]> | [:C> :D>]> /->(b)
        RETURN a.val, b.val ORDER BY a.val, b.val"""
        actual_result = self.parallel_pipe_graph.query(query)
        expected_result = [['v1', 'v4']]
        self.env.assertEquals(actual_result.result_set, expected_result)

    # def test05_path_pattern_execution(self):
    #     query = """
    #     MATCH (a)-/ <:A /->(b)
    #     RETURN a.val, b.val ORDER BY a.val, b.val"""
    #     actual_result = self.pipe_graph.query(query)
    #     expected_result = [['v2', 'v1'],
    #                        ['v3', 'v2']]
    #     self.env.assertEquals(actual_result.result_set, expected_result)
    #
    # def test06_path_pattern_execution(self):
    #     query = """
    #     MATCH (a)-/ <:A | <:B /->(b)
    #     RETURN a.val, b.val ORDER BY a.val, b.val"""
    #     actual_result = self.pipe_graph.query(query)
    #
    #     query1 = """
    #     MATCH (a)-/ <[:A> | :B>] /->(b)
    #     RETURN a.val, b.val ORDER BY a.val, b.val"""
    #     actual_result1 = self.pipe_graph.query(query1)
    #
    #     expected_result = [['v2', 'v1'],
    #                        ['v3', 'v2'],
    #                        ['v4', 'v3'],
    #                        ['v5', 'v4']]
    #     self.env.assertEquals(actual_result.result_set, expected_result)
    #     self.env.assertEquals(actual_result.result_set, actual_result1.result_set)
    #
    # def test07_path_pattern_execution(self):
    #     query = """
    #     MATCH (a)-/ <:B <:A /->(b)
    #     RETURN a.val, b.val ORDER BY a.val, b.val"""
    #     actual_result = self.pipe_graph.query(query)
    #
    #     query1 = """
    #     MATCH (a)-/ <[:A :B] /->(b)
    #     RETURN a.val, b.val ORDER BY a.val, b.val"""
    #     actual_result1 = self.pipe_graph.query(query1)
    #
    #     expected_result = [['v4', 'v2']]
    #     self.env.assertEquals(actual_result.result_set, expected_result)
    #     self.env.assertEquals(actual_result.result_set, actual_result1.result_set)
    #
    # def test08_path_pattern_execution(self):
    #     query = """
    #     MATCH (a)-/ <[:C | :D] <[:A | :B] /->(b)
    #     RETURN a.val, b.val ORDER BY a.val, b.val"""
    #     actual_result = self.tree_graph.query(query)
    #
    #     query1 = """
    #     MATCH (a)-/ [<:C | <:D] [<:A | <:B] /->(b)
    #     RETURN a.val, b.val ORDER BY a.val, b.val"""
    #     actual_result1 = self.tree_graph.query(query1)
    #     expected_result = [['v4', 'v1'],
    #                        ['v5', 'v1'],
    #                        ['v6', 'v1'],
    #                        ['v7', 'v1']]
    #     self.env.assertEquals(actual_result.result_set, expected_result)
    #     self.env.assertEquals(actual_result.result_set, actual_result1.result_set)
    #
    # def test09_path_pattern_execution(self):
    #     query = """
    #     MATCH (a)-/ <[:A :B] | <[:C :D] /->(b)
    #     RETURN a.val, b.val ORDER BY a.val, b.val"""
    #     actual_result = self.parallel_pipe_graph.query(query)
    #
    #     query1 = """
    #     MATCH (a)-/ [<:B <:A] | [<:D <:C] /->(b)
    #     RETURN a.val, b.val ORDER BY a.val, b.val"""
    #     actual_result1 = self.parallel_pipe_graph.query(query1)
    #
    #     expected_result = [['v4', 'v1']]
    #     self.env.assertEquals(actual_result.result_set, expected_result)
    #     self.env.assertEquals(actual_result.result_set, actual_result1.result_set)
    #
    # def test10_path_pattern_execution(self):
    #     query = """
    #     PATH PATTERN S = ()-/ [:A ~S :B] | [:A :B] /-()
    #     MATCH (a)-/ ~S /->(b)
    #     RETURN a.val, b.val ORDER BY a.val, b.val"""
    #     actual_result = self.pipe_graph.query(query)
    #     expected_result = [['v1', 'v5'], ['v2', 'v4']]
    #     self.env.assertEquals(actual_result.result_set, expected_result)
    #
    # def test11_path_pattern_execution(self):
    #     query = """
    #     PATH PATTERN S = ()-/ :A [~S | ()] :B /-()
    #     MATCH (a)-/ ~S /->(b)
    #     RETURN a.val, b.val ORDER BY a.val, b.val"""
    #     actual_result = self.pipe_graph.query(query)
    #     expected_result = [['v1', 'v5'],
    #                        ['v2', 'v4']]
    #     self.env.assertEquals(actual_result.result_set, expected_result)
    #
    # def test12_path_pattern_execution(self):
    #     query = """
    #     PATH PATTERN S = ()-/ :A [~S | ()] :B /->()
    #     MATCH (a)-[:A]-(b)-/ ~S /->(c)
    #     RETURN a.val, b.val, c.val ORDER BY a.val, b.val, c.val"""
    #     actual_result = self.pipe_graph.query(query)
    #     expected_result = [['v1', 'v2', 'v4'],
    #                        ['v2', 'v1', 'v5'],
    #                        ['v3', 'v2', 'v4']]
    #     self.env.assertEquals(actual_result.result_set, expected_result)
    #
    # def test13_path_pattern_execution(self):
    #     query = """
    #     PATH PATTERN S = ()-/ :A [~S | ()] :B /->()
    #     MATCH (a)-[:A]-(b)-/ ~S /->(c)-[:B]-(d)
    #     RETURN a.val, b.val, c.val, d.val ORDER BY a.val, b.val, c.val, d.val"""
    #     actual_result = self.pipe_graph.query(query)
    #     expected_result = [['v1', 'v2', 'v4', 'v3'],
    #                        ['v1', 'v2', 'v4', 'v5'],
    #                        ['v2', 'v1', 'v5', 'v4'],
    #                        ['v3', 'v2', 'v4', 'v3'],
    #                        ['v3', 'v2', 'v4', 'v5']]
    #     self.env.assertEquals(actual_result.result_set, expected_result)
    #
    # # def test14_path_pattern_execution(self):
    # #     query = """
    # #     PATH PATTERN S = ()-/:A [~S | ()] :B/->()
    # #     MATCH (a)-/ <~S /->(b)
    # #     RETURN a.val, b.val ORDER BY a.val, b.val"""
    # #     actual_result = self.pipe_graph.query(query)
    # #     expected_result = [['v4', 'v2'],
    # #                        ['v5', 'v1']]
    # #     self.env.assertEquals(actual_result.result_set, expected_result)
    #
    # def test15_path_pattern_execution(self):
    #     query = """
    #     PATH PATTERN S = ()-/ :A [<~S | ()] :B /->()
    #     MATCH (a)-/ ~S /->(b)
    #     RETURN a.val, b.val ORDER BY a.val, b.val"""
    #     actual_result = self.reversed_pipe_graph.query(query)
    #     expected_result = [['v1', 'v5'],
    #                        ['v4', 'v2']]
    #     self.env.assertEquals(actual_result.result_set, expected_result)
    #
    # def test16_path_pattern_execution(self):
    #     query = """
    #     PATH PATTERN S = ()-/ :A :B /->()
    #     MATCH (a)-/ <~S /->(b)
    #     RETURN a.val, b.val ORDER BY a.val, b.val"""
    #     actual_result = self.pipe_graph.query(query)
    #     expected_result = [['v4', 'v2']]
    #     self.env.assertEquals(actual_result.result_set, expected_result)
    #
    # def test17_path_pattern_execution(self):
    #     query = """
    #     MATCH (a)-/ :A* /->(b)
    #     RETURN a.val, b.val ORDER BY a.val, b.val"""
    #     actual_result = self.pipe_graph.query(query)
    #     expected_result = [['v1', 'v1'],
    #                        ['v1', 'v2'],
    #                        ['v1', 'v3'],
    #                        ['v2', 'v2'],
    #                        ['v2', 'v3'],
    #                        ['v3', 'v3'],
    #                        ['v4', 'v4'],
    #                        ['v5', 'v5']]
    #
    #     self.env.assertEquals(actual_result.result_set, expected_result)
    #
    # def test18_path_pattern_execution(self):
    #     query = """
    #         PATH PATTERN S = ()-/ <:A <:A* /->()
    #         MATCH (a)-/ ~S /->(b)
    #         RETURN a.val, b.val ORDER BY a.val, b.val"""
    #     actual_result = self.pipe_graph.query(query)
    #     expected_result = [['v2', 'v1'],
    #                        ['v3', 'v1'],
    #                        ['v3', 'v2']]
    #
    #     self.env.assertEquals(actual_result.result_set, expected_result)
