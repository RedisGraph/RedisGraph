import os
import sys
from RLTest import Env
from redisgraph import Graph, Node, Edge

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from base import FlowTestsBase

graph = None
redis_con = None

people = ["Roi", "Alon", "Ailon", "Boaz"]


class testResultSetFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env(decodeResponses=True)
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

    # Verify that RETURN * projections include all user-defined aliases.
    def test06_return_all(self):
        query = """MATCH (a)-[e]->(b) RETURN *"""
        result = graph.query(query)
        # Validate the header strings of the 3 columns.
        # NOTE - currently, RETURN * populates values in alphabetical order, but that is subject to later change.
        self.env.assertEqual(result.header[0][1], 'a')
        self.env.assertEqual(result.header[1][1], 'b')
        self.env.assertEqual(result.header[2][1], 'e')
        # Verify that 3 columns are returned
        self.env.assertEqual(len(result.result_set[0]), 3)

    # Tests for aggregation functions default values. Fix for issue 767.
    def test07_agg_func_default_values(self):
        # Test for aggregation over non existing node properties.
        # Max default value is null.
        query = """MATCH (a) return max(a.missing_field)"""
        result = graph.query(query)
        self.env.assertEqual(None, result.result_set[0][0])

        # Min default value is null.
        query = """MATCH (a) return min(a.missing_field)"""
        result = graph.query(query)
        self.env.assertEqual(None, result.result_set[0][0])

        # Count default value is 0.
        query = """MATCH (a) return count(a.missing_field)"""
        result = graph.query(query)
        self.env.assertEqual(0, result.result_set[0][0])

        # Avarage default value is 0.
        query = """MATCH (a) return avg(a.missing_field)"""
        result = graph.query(query)
        self.env.assertEqual(0, result.result_set[0][0])

        # Collect default value is an empty array.
        query = """MATCH (a) return collect(a.missing_field)"""
        result = graph.query(query)
        self.env.assertEqual([], result.result_set[0][0])

        # StdDev default value is 0.
        query = """MATCH (a) return stdev(a.missing_field)"""
        result = graph.query(query)
        self.env.assertEqual(0, result.result_set[0][0])

        # percentileCont default value is null.
        query = """MATCH (a) return percentileCont(a.missing_field, 0.1)"""
        result = graph.query(query)
        self.env.assertEqual(None, result.result_set[0][0])

        # percentileDisc default value is null.
        query = """MATCH (a) return percentileDisc(a.missing_field, 0.1)"""
        result = graph.query(query)
        self.env.assertEqual(None, result.result_set[0][0])

    # Test returning multiple occurrence of an expression.
    def test08_return_duplicate_expression(self):
        query = """MATCH (a) RETURN max(a.val), max(a.val)"""
        result = graph.query(query)
        self.env.assertEqual(result.result_set[0][0], result.result_set[0][1])

        query = """MATCH (a) return max(a.val) as x, max(a.val) as x"""
        result = graph.query(query)
        self.env.assertEqual(result.result_set[0][0], result.result_set[0][1])

        query = """MATCH (a) RETURN a.val, a.val LIMIT 1"""
        result = graph.query(query)
        self.env.assertEqual(result.result_set[0][0], result.result_set[0][1])

        query = """MATCH (a) return a.val as x, a.val as x LIMIT 1"""
        result = graph.query(query)
        self.env.assertEqual(result.result_set[0][0], result.result_set[0][1])

    # Test implicit result-set size limit
    def test09_implicit_resultset_limit(self):
        query = "MATCH (a) RETURN a"

        result = graph.query(query)
        record_count = len(result.result_set)

        # make sure limit is greater than 0
        assert(record_count > 1)
        limit = record_count -1

        # enforce implicit limit
        redis_con.execute_command("GRAPH.CONFIG", "SET", "RESULTSET_SIZE", limit)

        result = graph.query(query)
        limited_record_count = len(result.result_set)
        assert(limited_record_count == limit)

        # lift limit, -1 stands for unlimited
        redis_con.execute_command("GRAPH.CONFIG", "SET", "RESULTSET_SIZE", -1)

        # re-issue query
        result = graph.query(query)
        unlimited_record_count = len(result.result_set)
        assert(unlimited_record_count == record_count)

