import os
import sys
import unittest
from redisgraph import Graph, Node, Edge

# import redis
from .disposableredis import DisposableRedis

from base import FlowTestsBase

redis_graph = None

def redis():
    return DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')

class GraphMergeFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "GraphMergeFlowTest"
        global redis_graph
        cls.r = redis()
        cls.r.start()
        redis_con = cls.r.client()
        redis_graph = Graph("G", redis_con)

        # cls.r = redis.Redis()
        # redis_graph = Graph("G", cls.r)

    @classmethod
    def tearDownClass(cls):
        cls.r.stop()
        # pass

    # Create a single node without any labels or properties.
    def test01_single_node_with_label(self):
        global redis_graph
        query = """MERGE (robert:Critic)"""
        result = redis_graph.query(query)
        assert(result.labels_added == 1)
        assert(result.nodes_created == 1)
        assert(result.properties_set == 0)
    
    # Retry to create an existing entity.
    def test02_existing_single_node_with_label(self):
        global redis_graph
        query = """MERGE (robert:Critic)"""
        result = redis_graph.query(query)
        assert(result.labels_added == 0)
        assert(result.nodes_created == 0)
        assert(result.properties_set == 0)
    
    # Create a single node with two properties and no labels.
    def test03_single_node_with_properties(self):
        global redis_graph
        query = """MERGE (charlie { name: 'Charlie Sheen', age: 10 })"""
        result = redis_graph.query(query)        
        assert(result.labels_added == 0)
        assert(result.nodes_created == 1)
        assert(result.properties_set == 2)
    
    # Retry to create an existing entity.
    def test04_existing_single_node_with_properties(self):
        global redis_graph
        query = """MERGE (charlie { name: 'Charlie Sheen', age: 10 })"""
        result = redis_graph.query(query)        
        assert(result.labels_added == 0)
        assert(result.nodes_created == 0)
        assert(result.properties_set == 0)
    
    # Create a single node with both label and property.
    def test05_single_node_both_label_and_property(self):
        global redis_graph
        query = """MERGE (michael:Person { name: 'Michael Douglas' })"""
        result = redis_graph.query(query)        
        assert(result.labels_added == 1)
        assert(result.nodes_created == 1)
        assert(result.properties_set == 1)
    
    # Retry to create an existing entity.
    def test06_existing_single_node_both_label_and_property(self):
        global redis_graph
        query = """MERGE (michael:Person { name: 'Michael Douglas' })"""
        result = redis_graph.query(query)        
        assert(result.labels_added == 0)
        assert(result.nodes_created == 0)
        assert(result.properties_set == 0)

    # Create a single edge and additional two nodes.
    def test07_merge_on_relationship(self):
        global redis_graph
        query = """MERGE (charlie:ACTOR)-[r:ACTED_IN]->(wallStreet:MOVIE)"""
        result = redis_graph.query(query)        
        assert(result.labels_added == 2)
        assert(result.nodes_created == 2)
        assert(result.properties_set == 0)
        assert(result.relationships_created == 1)
    
    # Retry to create a single edge and additional two nodes.
    def test08_existing_merge_on_relationship(self):
        global redis_graph
        query = """MERGE (charlie:ACTOR)-[r:ACTED_IN]->(wallStreet:MOVIE)"""
        result = redis_graph.query(query)        
        assert(result.labels_added == 0)
        assert(result.nodes_created == 0)
        assert(result.properties_set == 0)
        assert(result.relationships_created == 0)

    # Update existing entity
    def test09_update_existing_node(self):
        global redis_graph
        query = """MERGE (charlie { name: 'Charlie Sheen', age: 10 }) SET charlie.age = 11, charlie.lastname='Sheen' """
        result = redis_graph.query(query)
        assert(result.labels_added == 0)
        assert(result.nodes_created == 0)
        assert(result.properties_set == 2)
        assert(result.relationships_created == 0)

        query = """MATCH (charlie { name: 'Charlie Sheen' }) RETURN charlie"""
        actual_result = redis_graph.query(query)
        expected_result = [['charlie.age', 'charlie.name', 'charlie.lastname'],
                           ['11.000000', 'Charlie Sheen', 'Sheen']]
        assert(actual_result.result_set == expected_result)

    # Update new entity
    def test10_update_new_node(self):
        global redis_graph
        query = """MERGE (tamara:ACTOR { name: 'tamara tunie' }) SET tamara.age = 59, tamara.name = 'Tamara Tunie' """
        result = redis_graph.query(query)
        assert(result.labels_added == 0)
        assert(result.nodes_created == 1)
        assert(result.properties_set == 2)
        assert(result.relationships_created == 0)

        query = """MATCH (tamara:ACTOR { name: 'Tamara Tunie' }) RETURN tamara"""
        actual_result = redis_graph.query(query)
        expected_result = [['tamara.name', 'tamara.age'],
                           ['Tamara Tunie', '59.000000']]
        assert(actual_result.result_set == expected_result)

    # Create a single edge and additional two nodes.
    def test11_update_new_relationship(self):
        global redis_graph
        query = """MERGE (franklin:ACTOR { name: 'Franklin Cover' })-[r:ACTED_IN {rate:5.7}]->(almostHeroes:MOVIE) SET r.date=1998, r.rate=5.8"""
        result = redis_graph.query(query)
        assert(result.labels_added == 0)
        assert(result.nodes_created == 2)
        assert(result.properties_set == 2)
        assert(result.relationships_created == 1)
    
    # Update existing relation
    def test12_update_existing_edge(self):
        global redis_graph
        query = """MERGE (franklin:ACTOR { name: 'Franklin Cover' })-[r:ACTED_IN {rate:5.8, date:1998}]->(almostHeroes:MOVIE) SET r.date=1998, r.rate=5.9"""
        result = redis_graph.query(query)
        assert(result.labels_added == 0)
        assert(result.nodes_created == 0)
        assert(result.properties_set == 2)
        assert(result.relationships_created == 0)

        query = """MATCH (franklin:ACTOR { name: 'Franklin Cover' })-[r:ACTED_IN {rate:5.9, date:1998}]->(almostHeroes:MOVIE) RETURN franklin, r"""
        actual_result = redis_graph.query(query)
        expected_result = [['franklin.name', 'franklin.age', 'r.rate', 'r.date'],
                           ['Franklin Cover', 'NULL', '5.900000', '1998.000000']]
        assert(actual_result.result_set == expected_result)

if __name__ == '__main__':
    unittest.main()
