import os
import sys
import redis
import string
import random
import unittest
from base import FlowTestsBase
from redisgraph import Graph, Node, Edge

redis_con = None
dis_redis = None
GRAPH_NAME = None
DENSE_GRAPH_NAME = None
redis_graph = None
dense_graph = None
people = ["Roi", "Alon", "Ailon", "Boaz", "Tal", "Omri", "Ori"]
countries = ["Israel", "USA", "Japan", "United Kingdom"]
visits = [("Roi", "USA"), ("Alon", "Israel"), ("Ailon", "Japan"), ("Boaz", "United Kingdom")]

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

class GraphPersistency(FlowTestsBase):
    @classmethod
    def setUpClass(cls):
        print "GraphPersistency"
        global redis_graph
        global dense_graph
        global redis_con
        global GRAPH_NAME
        global DENSE_GRAPH_NAME
        GRAPH_NAME = random_string()
        DENSE_GRAPH_NAME = random_string()
        redis_con = get_redis()
        redis_graph = Graph(GRAPH_NAME, redis_con)
        dense_graph = Graph(DENSE_GRAPH_NAME, redis_con)
        cls.populate_graph()
        cls.populate_dense_graph()

    @classmethod
    def tearDownClass(cls):
        if dis_redis is not None:
            dis_redis.stop()

    @classmethod
    def populate_graph(cls):
        global redis_graph

        if not redis_con.exists(GRAPH_NAME):
            personNodes = {}
            countryNodes = {}
            # Create entities

            for p in people:
                person = Node(label="person", properties={"name": p})
                redis_graph.add_node(person)
                personNodes[p] = person

            for p in countries:
                country = Node(label="country", properties={"name": p})
                redis_graph.add_node(country)
                countryNodes[p] = country

            for v in visits:
                person = v[0]
                country = v[1]
                edge = Edge(personNodes[person], 'visit', countryNodes[country], properties={'purpose': 'pleasure'})
                redis_graph.add_edge(edge)

            redis_graph.commit()

            # Delete nodes, to introduce deleted item within our datablock
            query = """MATCH (n:person) WHERE n.name = 'Roi' or n.name = 'Ailon' DELETE n"""
            redis_graph.query(query)

            query = """MATCH (n:country) WHERE n.name = 'USA' DELETE n"""
            redis_graph.query(query)

            # Create index.
            actual_result = redis_con.execute_command("GRAPH.QUERY", GRAPH_NAME, "CREATE INDEX ON :person(name)")
            actual_result = redis_con.execute_command("GRAPH.QUERY", GRAPH_NAME, "CREATE INDEX ON :country(name)")

    @classmethod
    def populate_dense_graph(cls):
        global dense_graph

        if not redis_con.exists(DENSE_GRAPH_NAME):
            nodes = []
            for i in range(10):
                node = Node(label="n", properties={"val":i})
                dense_graph.add_node(node)
                nodes.append(node)

            for n_idx, n in enumerate(nodes):
                for m_idx, m in enumerate(nodes[:n_idx]):
                    dense_graph.add_edge(Edge(n, "connected", m))

            dense_graph.commit()

    #  Connect a single node to all other nodes.
    def test01_save_load_rdb(self):
        for i in range(2):
            if i == 1:
                # Save RDB & Load from RDB
                redis_con.execute_command("DEBUG", "RELOAD")

            # Verify
            # Expecting 5 person entities.
            query = """MATCH (p:person) RETURN COUNT(p)"""
            actual_result = redis_graph.query(query)
            nodeCount = int(float(actual_result.result_set[1][0]))
            assert(nodeCount == 5)

            query = """MATCH (p:person) WHERE p.name='Alon' RETURN COUNT(p)"""
            actual_result = redis_graph.query(query)
            nodeCount = int(float(actual_result.result_set[1][0]))
            assert(nodeCount == 1)

            # Expecting 3 country entities.
            query = """MATCH (c:country) RETURN COUNT(c)"""
            actual_result = redis_graph.query(query)
            nodeCount = int(float(actual_result.result_set[1][0]))
            assert(nodeCount == 3)

            query = """MATCH (c:country) WHERE c.name = 'Israel' RETURN COUNT(c)"""
            actual_result = redis_graph.query(query)
            nodeCount = int(float(actual_result.result_set[1][0]))
            assert(nodeCount == 1)

            # Expecting 2 visit edges.
            query = """MATCH (n:person)-[e:visit]->(c:country) WHERE e.purpose='pleasure' RETURN COUNT(e)"""
            actual_result = redis_graph.query(query)
            edgeCount = int(float(actual_result.result_set[1][0]))
            assert(edgeCount == 2)

            # Verify indices exists.
            actual_result = redis_con.execute_command("GRAPH.EXPLAIN", GRAPH_NAME, "match (n:person) where n.name = 'Roi' RETURN n")
            assert("Index Scan" in actual_result)

            actual_result = redis_con.execute_command("GRAPH.EXPLAIN", GRAPH_NAME, "match (n:country) where n.name = 'Israel' RETURN n")
            assert("Index Scan" in actual_result)

    # Verify that edges are not modified after entity deletion
    def test02_deleted_entity_migration(self):
        query = """MATCH (p) WHERE ID(p) = 0 OR ID(p) = 3 OR ID(p) = 7 OR ID(p) = 9 DELETE p"""
        actual_result = dense_graph.query(query)
        assert(actual_result.nodes_deleted == 4)
        query = """MATCH (p)-[]->(q) RETURN p, q ORDER BY p.val, q.val"""
        first_result = dense_graph.query(query)

        # Save RDB & Load from RDB
        redis_con.execute_command("DEBUG", "RELOAD")

        second_result = dense_graph.query(query)
        assert(first_result.result_set == second_result.result_set)

    # Strings, numerics, booleans, and NULL properties should be properly serialized and reloaded
    def test03_restore_properties(self):
        graphname = "simple_props"
        graph = Graph(graphname, redis_con)
        query = """CREATE (:p {strval: 'str', numval: 5.5, nullval: NULL, boolval: true})"""
        actual_result = graph.query(query)
        # Verify that node was created correctly
        assert(actual_result.nodes_created == 1)
        assert(actual_result.properties_set == 4)

        # Save RDB & Load from RDB
        redis_con.execute_command("DEBUG", "RELOAD")

        query = """MATCH (p) RETURN p"""
        actual_result = graph.query(query)

        # Verify that the properties are loaded correctly.
        # Note that the order of results is not guaranteed (currently managed by the Schema),
        # so this may need to be updated in the future.
        expected_result = [['p.boolval', 'p.nullval', 'p.numval', 'p.strval'],
                           ['true', None, '5.5', 'str']]
        assert(actual_result.result_set == expected_result)

    # Verify that the database can be reloaded correctly after creating multiple
    # relations of the same type connecting the same two entities.
    # TODO Cypher allows multiple relations like this to coexist; we currently overwrite the original.
    # This test should be updated as our handling changes.
    def test04_repeated_edges(self):
        graphname = "repeated_edges"
        graph = Graph(graphname, redis_con)
        # Build two nodes
        create_query = """CREATE (:p {name: 'src'}), (:p {name: 'dest'})"""
        actual_result = graph.query(create_query)
        assert(actual_result.nodes_created == 2)

        # Connect nodes
        create_query = """MATCH(a:p {name: 'src'}), (b:p {name: 'dest'}) CREATE (a)-[:e {val: 1}]->(b)"""
        actual_result = graph.query(create_query)
        assert(actual_result.relationships_created == 1)

        # Verify the new edge
        read_query = """MATCH (a)-[e]->(b) RETURN e, a, b"""
        actual_result = graph.query(read_query)
        expected_result = [['e.val', 'a.name', 'b.name'],
                           [1, 'src', 'dest']]
        assert(actual_result.result_set == expected_result)

        # Overwrite the existing edge
        create_query = """MATCH(a:p {name: 'src'}), (b:p {name: 'dest'}) CREATE (a)-[:e {val: 2}]->(b)"""
        actual_result = graph.query(create_query)
        assert(actual_result.relationships_created == 1)

        actual_result = graph.query(read_query)
        # TODO This is the expected current behavior, subject to later change.
        expected_result = [['e.val', 'a.name', 'b.name'],
                           [2, 'src', 'dest']]
        assert(actual_result.result_set == expected_result)

        # Save RDB & Load from RDB
        redis_con.execute_command("DEBUG", "RELOAD")

        # Verify that the latest edge was properly saved and loaded
        actual_result = graph.query(read_query)
        assert(actual_result.result_set == expected_result)

if __name__ == '__main__':
    unittest.main()
