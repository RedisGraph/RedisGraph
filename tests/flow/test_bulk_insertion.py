import os
import sys
import unittest
from .disposableredis import DisposableRedis
from redisgraph import Graph, Node, Edge
from base import FlowTestsBase
sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../../demo/bulk_insert/')

import click
from click.testing import CliRunner

from bulk_insert import bulk_insert

redis_con = None
port = None
graphname = None

def redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

class GraphBulkInsertFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        global redis_con
        global port 
	global graphname
        graphname = "bulk_test"
        cls.r = redis()
        cls.r.start()
        redis_con = cls.r.client()
	port = cls.r.port

    @classmethod
    def tearDownClass(cls):
        cls.r.stop()

    def test01_bulk_insert_single_label(self):
	global redis_con
	global port

	runner = CliRunner()
	# Insert the 'person' label
        csv_path = os.path.dirname(os.path.abspath(__file__)) + '/../../demo/bulk_insert/'
	res = runner.invoke(bulk_insert, ['--port', port, '--nodes', csv_path + 'person.csv', graphname])

	# The script should report 11 node creations
	assert res.exit_code == 0
	assert '11 Nodes created' in res.output

	# Query the newly-created graph
	redis_graph = Graph(graphname, redis_con)
	query_result = redis_graph.query('MATCH (p:person) RETURN p')

	# Verify that the label exists, has the correct attributes, and is properly populated
	expected_result=[['p.first_name', 'p.last_name'],
			 ['Roi', 'Lipman'],
			 ['Alon', 'Fital'],
			 ['Ailon', 'Velger'],
			 ['Omri', 'Traub'],
			 ['Tal', 'Doron'],
			 ['Ori', 'Laslo'],
			 ['Noam', 'Nativ'],
			 ['Hila', 'Rahamani'],
			 ['Lucy', 'Fital'],
			 ['Jane', 'Velger'],
			 ['Shelly', 'Laslo']]
	assert query_result.result_set == expected_result

    def test02_relation_within_label(self):
	global redis_con
	global port

	runner = CliRunner()
	# Insert the 'person' label and the 'know' and 'married' relations
        csv_path = os.path.dirname(os.path.abspath(__file__)) + '/../../demo/bulk_insert/'
	res = runner.invoke(bulk_insert, ['--port', port, '--nodes', csv_path + 'person.csv',
					  '--relations', csv_path + 'know.csv',
					  '--relations', csv_path + 'married.csv',
					  graphname])

	# The script should report 11 node creations and 33 edge creations
	assert res.exit_code == 0
	assert '11 Nodes created' in res.output
	assert '33 Relations created' in res.output

	redis_graph = Graph(graphname, redis_con)
	# Verify that the right number of relations are found
	query_result = redis_graph.query('MATCH (p:person)-[:know]->(q:person) RETURN COUNT(q)')
	assert int(float(query_result.result_set[1][0])) == 29

	query_result = redis_graph.query('MATCH (p:person)-[:married]->(q:person) RETURN COUNT(q)')
	assert int(float(query_result.result_set[1][0])) == 4

    def test03_multiple_labels(self):
	global redis_con
	global port

	runner = CliRunner()
	# Create nodes for 'person' and 'country',
	# intra-label relations 'know' and 'married',
	# and inter-label relation 'visit'
        csv_path = os.path.dirname(os.path.abspath(__file__)) + '/../../demo/bulk_insert/'
	res = runner.invoke(bulk_insert, ['--port', port, '--nodes', csv_path + 'person.csv',
					  '--nodes', csv_path + 'country.csv',
					  '--relations', csv_path + 'married.csv',
					  '--relations', csv_path + 'know.csv',
					  '--relations', csv_path + 'visit.csv',
					  graphname])

	# The script should report 13 node creations and 36 edge creations
	assert res.exit_code == 0
	assert '13 Nodes created' in res.output
	assert '36 Relations created' in res.output

	redis_graph = Graph(graphname, redis_con)
	# Verify that the right number of relations are found
	query_result = redis_graph.query('MATCH (p:person)-[:know]->(q:person) RETURN COUNT(q)')
	assert int(float(query_result.result_set[1][0])) == 29

	query_result = redis_graph.query('MATCH (p:person)-[:married]->(q:person) RETURN COUNT(q)')
	assert int(float(query_result.result_set[1][0])) == 4

	query_result = redis_graph.query('MATCH (p:person)-[:visit]->(c:country) RETURN COUNT(c)')
	assert int(float(query_result.result_set[1][0])) == 3

if __name__ == '__main__':
    unittest.main()
