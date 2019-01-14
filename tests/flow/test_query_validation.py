import os
import sys
import redis
import string
import random
import unittest
from base import FlowTestsBase
from redisgraph import Graph, Node, Edge

redis_graph = None
dis_redis = None
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
        from .disposableredis import DisposableRedis
        # Bring up our own redis-server instance.
        dis_redis = DisposableRedis(loadmodule=os.path.dirname(os.path.abspath(__file__)) + '/../../src/redisgraph.so')
        dis_redis.start()
        conn = dis_redis.client()
    return conn

class QueryValidationFlowTest(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "QueryValidationFlowTest"
        global redis_graph
        global redis_con
        redis_con = get_redis()
        
        # Create a single graph.
        GRAPH_ID = random_string()
        redis_graph = Graph(GRAPH_ID, redis_con)
        node = Node(properties={"age": 34})
        redis_graph.add_node(node)
        redis_graph.commit()

    @classmethod
    def tearDownClass(cls):
        if dis_redis is not None:
            dis_redis.stop()

    # Expect an error when trying to use a function which does not exists.
    def test01_none_existing_function(self):
        query = """MATCH (n) RETURN noneExistingFunc(n.age) AS cast"""
        try:
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    # Make sure function validation is type case insensitive.
    def test02_case_insensitive_function_name(self):
        try:
            query = """MATCH (n) RETURN mAx(n.age)"""
            redis_graph.query(query)
        except redis.exceptions.ResponseError:
            # function validation should be case insensitive.
            assert(False)
    
    def test03_edge_missing_relation_type(self):
        try:
            query = """CREATE (n:Person {age:32})-[]->(:person {age:30})"""
            redis_graph.query(query)
            assert(False)
        except redis.exceptions.ResponseError:
            # Expecting an error.
            pass

    def test04_escaped_quotes(self):
        query = r"CREATE (:escaped{prop1:'single \' char', prop2: 'double \" char', prop3: 'mixed \' and \" chars'})"
        actual_result = redis_graph.query(query)
        assert(actual_result.nodes_created == 1)
        assert(actual_result.properties_set == 3)

        query = r"MATCH (a:escaped) RETURN a"
        actual_result = redis_graph.query(query)
        expected_result = [['a.prop3', 'a.prop2', 'a.prop1'],
                           ['mixed \' and " chars', 'double " char', "single ' char"]]

        assert(actual_result.result_set == expected_result)


if __name__ == '__main__':
    unittest.main()
