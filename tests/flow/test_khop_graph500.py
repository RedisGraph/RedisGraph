# -*- coding: utf-8 -*-
import os
import sys
import csv
import click
from RLTest import Env
from click.testing import CliRunner

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from redisgraph import Graph, Node, Edge
from base import FlowTestsBase

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../demo/bulk_insert/')
from bulk_insert import bulk_insert

redis_con = None
port = None
redis_graph = None

# GRAPH.QUERY graph500_64 "MATCH (s)-[*3]->(t) WHERE s.id=3235527 RETURN (t)"

# ejr@qNaN:~/Projects/lucata/khop_benchmark/graph-database-benchmark/benchmark/redisgraph$ python bulk_insert.py graph500_64 -n  ~/Projects/lucata/testing/RG-base/demo/bulk_insert/resources/graph500_64_unique_node -r ~/Projects/lucata/testing/RG-base/demo/bulk_insert/resources/graph500_64
# 64 nodes created with label 'graph500_64_unique_node'
# 400 relations created for type 'graph500_64'. skipped 0 entities due to non exiting identifier
# Construction of graph 'graph500_64' complete: 64 nodes created, 400 relations created in 0.050139 seconds

class testGraphKHopFlow(FlowTestsBase):
    def __init__(self):
        self.env = Env()
        global redis_graph
        global redis_con
        redis_con = self.env.getConnection()
        port = self.env.envRunner.port
        redis_graph = Graph("graph", redis_con)

    # Run bulk loader script and validate terminal output
    def test01_import_g500(self):
        graphname = "graph"
        runner = CliRunner()

        csv_path = os.path.dirname(os.path.abspath(__file__)) + '/../../demo/bulk_insert/resources/'
        res = runner.invoke(bulk_insert, ['--port', port,
                                          '--nodes', csv_path + 'graph500_64_unique_node',
                                          '--relations', csv_path + 'graph500_64',
                                          graphname])

        # The script should report 64 node creations and 400 edge creations
        self.env.assertEquals(res.exit_code, 0)
        self.env.assertIn('64 nodes created', res.output)
        self.env.assertIn('400 relations created', res.output)

    def test02_create_index(self):
        global redis_graph
        query_result = redis_graph.query('CREATE INDEX ON :graph500_64_unique_node(id)')
        self.env.assertEquals(query_result.indices_created, 1)
        #self.env.assertIn('Indices created 1.0', query_result

    # Check all vertices three hops away from the source vertex
    def test03_run_3hop(self):
        global redis_graph
        result = [968107, 2903882, 3332655, 1223642, 4010250, 2548240]
        query_result = redis_graph.query('MATCH (s)-[*3]->(t) WHERE s.id=3235527 RETURN (t)')
        self.env.assertEquals(len(query_result.result_set), len(result))
        for row in query_result.result_set:
            id = row[0].properties['id']
            self.env.assertIn(id, result)
            result.remove(id)
        self.env.assertEquals(len(result), 0)

    # Look up the source vertex through an index
    def test05_run_3hop_idx(self):
        global redis_graph
        result = [968107, 2903882, 3332655, 1223642, 4010250, 2548240]
        query_result = redis_graph.query('MATCH (s:graph500_64_unique_node)-[*3]->(t) WHERE s.id=3235527 RETURN (t)')
        self.env.assertEquals(len(query_result.result_set), len(result))
        for row in query_result.result_set:
            id = row[0].properties['id']
            self.env.assertIn(id, result)
            result.remove(id)
        self.env.assertEquals(len(result), 0)
        


